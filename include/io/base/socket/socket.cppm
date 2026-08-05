module;

#include <cassert>
#include <cstdint>
#include <openssl/err.h>
#include <openssl/quic.h>
#include <openssl/ssl.h>
#include <utility>

#ifdef _WIN32

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <winsock2.h>
#include <ws2tcpip.h>

#else

#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>

#endif

export module io_base_socket;

import std;
import io_error;
import core_events;
import core_logger;
import io_base_leverage;
import shared;
export import :consts;

#ifdef _WIN32
export import :win32;
#else
export import :posix;
#endif

class SSLAutoInitializer {
  public:
    /**
     * @brief Bet, this is the whole reason the class exists — fires `OPENSSL_init_ssl`/
     * `OPENSSL_init_crypto` once, at static-init time, via the file-scope `AUTO_INIT` instance
     * below. Every `Socket<TLS>`/`Socket<QUIC>` downstream just rides on OpenSSL already being
     * warmed up by the time `main()` runs.
     * @throws std::runtime_error if either `OPENSSL_init_ssl` or `OPENSSL_init_crypto` returns 0.
     * @warning If this throws, it's during static initialization — that's an L with basically no
     * good recovery story, the program's toast before `main()` even starts. Also note the
     * `ERR_print_errors_fp` call sits *after* the `throw`, so it's unreachable dead code, not an
     * actual diagnostic on failure.
     */
    // noexcept: every throwing path below is caught internally and turned into an explicit,
    // logged std::terminate() (see the try/catch), so this constructor genuinely cannot let an
    // exception escape — which is what let AUTO_INIT below drop its
    // bugprone-throwing-static-initialization finding without any behavior change.
    SSLAutoInitializer() noexcept {
        // Everything below used to let a std::runtime_error escape straight out of static
        // initialization (before main() even runs), which is what
        // bugprone-throwing-static-initialization flags — an exception there can't be caught by
        // anything, it just hits the default terminate handler. Wrapped in try/catch so the
        // failure path is an explicit, logged std::terminate() instead of an implicit one; the
        // "program's toast either way" outcome documented above is unchanged.
        try {
            // Load the SSL string tables first, then the crypto side (ciphers, digests, config) —
            // both need to succeed before anything downstream can touch OpenSSL safely.
            int ssl_init = OPENSSL_init_ssl(
                OPENSSL_INIT_LOAD_SSL_STRINGS | OPENSSL_INIT_LOAD_CRYPTO_STRINGS, nullptr);

            int crypto_init = OPENSSL_init_crypto(
                OPENSSL_INIT_LOAD_CONFIG | OPENSSL_INIT_ADD_ALL_CIPHERS |
                    OPENSSL_INIT_ADD_ALL_DIGESTS,
                nullptr);

            // Either init call returning 0 means OpenSSL never came up — nothing to do but bail.
            if (ssl_init == 0 || crypto_init == 0) {
                throw std::runtime_error(
                    "Failed to initialize OpenSSL libraries (Fatal - plesae check "
                    "online or create a new issue)");
                ERR_print_errors_fp(stderr);
            }
            std::println("OpenSSL libraries initialized successfully (DevInfo)");
        } catch (const std::exception &caught_error) {
            // The diagnostic print itself could in theory throw (format/allocation failure);
            // swallow that too so this constructor can never let anything past std::terminate()
            // below — this catch block already means the whole program is toast either way, per
            // the class-level @warning above.
            try {
                std::println(stderr, "Fatal: OpenSSL static initialization failed: {}",
                             caught_error.what());
            } catch (...) {  // NOLINT(bugprone-empty-catch) — best-effort diagnostic only, std::terminate() below fires unconditionally either way
                // Best-effort diagnostic only — terminate() below fires regardless.
            }
            std::terminate();
        }
    }
};

static const SSLAutoInitializer AUTO_INIT;


export namespace io::base::socket {

enum class Protocol : std::uint8_t { TCP = 0, UDP = 1, TLS = 2, QUIC = 3 };
enum class WaitMode : std::uint8_t { AS_SOON_AS_ARRIVED, WAIT_FOR_WHOLE_MESSAGE };
[[nodiscard]] constexpr bool is_waiting(WaitMode mode) noexcept {
    return mode == WaitMode::WAIT_FOR_WHOLE_MESSAGE;
}

class AddressInfo {
  public:
    /** @brief Zero-value default — empty sockaddr storage, size 0. */
    AddressInfo() : m_adrinf{} {}

    /**
     * @brief Sets both the raw address bytes and their length in one shot — this is what
     * Socket::sync_receive() feeds the peer address into for UDP reads.
     * @param adrinf the raw sockaddr storage to copy in.
     * @param sock_size the actual length of the address within `adrinf`.
     */
    void set(const sockaddr_storage &adrinf, socklen_t &sock_size) {
        m_adrinf = adrinf;
        m_sock_size = sock_size;
    }

    /**
     * @brief Sets just the raw address bytes, leaving the stored size untouched.
     * @param adrinf the raw sockaddr storage to copy in.
     */
    void set_data(const sockaddr_storage &adrinf) { m_adrinf = adrinf; }
    /**
     * @brief Sets just the stored address length, leaving the raw bytes untouched.
     * @param sock_size the new length to store.
     */
    void set_size(const socklen_t &sock_size) { m_sock_size = sock_size; }

    /**
     * @brief Grabs the raw address bytes.
     * @return a const reference to the stored `sockaddr_storage`.
     */
    [[nodiscard]] const sockaddr_storage &get_data() const noexcept { return m_adrinf; }
    /**
     * @brief Mutable overload of get_data() — lets callers (e.g. `recvfrom`) write straight into
     * the stored storage.
     * @return a mutable reference to the stored `sockaddr_storage`.
     */
    sockaddr_storage &get_data() noexcept { return m_adrinf; }
    /**
     * @brief Grabs the stored address length.
     * @return a const reference to the stored `socklen_t`.
     */
    [[nodiscard]] const socklen_t &get_size() const noexcept { return m_sock_size; }
    /**
     * @brief Mutable overload of get_size() — lets callers write the length directly (e.g. as an
     * in/out param to `recvfrom`).
     * @return a mutable reference to the stored `socklen_t`.
     */
    socklen_t &get_size() noexcept { return m_sock_size; }

  private:
    sockaddr_storage m_adrinf;
    socklen_t m_sock_size{0};
};

enum class Event : std::uint8_t { READ = 0x1, WRITE = 0x2, EXCEPT = 0x4 };

class Endpoint {
  public:
    /** @brief Zero-value default — empty address, port 0. */
    Endpoint() : m_port{0} {}

    /**
     * @brief Builds straight from an already-split address and port — no parsing motion here.
     * @param address host/IP portion.
     * @param port port number.
     */
    Endpoint(std::string_view address, std::uint16_t port) : m_address{address}, m_port{port} {}

    /**
     * @brief Parses a combined `"host:port"` string into its two parts.
     * @param address the combined address string, split on the last `:`.
     * @warning Doesn't throw on a bad address — every failure path (`no ':'`, missing port,
     * unparseable port, out-of-range port) routes through `core::logger::fatal`, which per this
     * codebase's logger convention terminates the process. Bad input here is a straight crash,
     * not a recoverable error. Also note this constructor doesn't handle bracketed IPv6 literals
     * (`[::1]:port`) — it just splits on the *last* `:`, which works for IPv6 addresses that have
     * no port suffix but would mis-split anything with a real `:port` after a raw IPv6 literal.
     */
    Endpoint(std::string_view address) {
        // Split on the last ':' — no colon at all, or a colon with nothing after it, both
        // mean there's no usable port here.
        const auto SEPARATOR = address.find_last_of(':');

        if (SEPARATOR == std::string_view::npos) {
            core::events::publish("socket.endpoint.invalid_address");
            core::logger::fatal("SocketLib", "invalid address");
        }
        if (SEPARATOR == address.size() - 1) {
            core::events::publish("socket.endpoint.missing_port");
            core::logger::fatal("SocketLib", "missing port");
        }

        m_address = address.substr(0, SEPARATOR);

        // Parse the tail as a numeric port — from_chars over ::stoi so we don't have to deal
        // with exceptions for a plain "is this actually a number" check.
        std::string_view port_view = address.substr(SEPARATOR + 1);
        std::uint16_t parsed_port = 0;

        auto [ptr, ec] =
            std::from_chars(port_view.data(), port_view.data() + port_view.size(), parsed_port);

        // Bad port string, out-of-range value, or anything else from_chars didn't like — all
        // fatal, no partial/best-effort port here.
        if (ec != std::errc{}) {
            if (ec == std::errc::invalid_argument) {
                core::events::publish("socket.endpoint.invalid_port_format");
                core::logger::fatal("SocketLib", "Invalid port format");
            } else if (ec == std::errc::result_out_of_range) {
                core::events::publish("socket.endpoint.port_too_large");
                core::logger::fatal("SocketLib", "port too large");
            }

            core::events::publish("socket.endpoint.port_parse_failed");
            core::logger::fatal("SocketLib", "Failed to parse port number");
        }

        m_port = parsed_port;
    }

    /**
     * @brief Builds from a raw platform sockaddr — reverse of the string ctor, this is what
     * Socket::sync_accept()/async_accept() use to turn a freshly-accepted peer address into an
     * `Endpoint`.
     * @param address pointer to a `sockaddr` (IPv4 or IPv6) to decode — caller-owned, only read
     * during the call.
     * @warning Null and unsupported-family inputs both route through `core::logger::fatal`
     * (process-terminating) rather than returning an error state — same crash-on-bad-input
     * pattern as the string ctor.
     */
    Endpoint(const SOCKADDR *address) {
        // Nothing to decode from a null pointer, straight fatal.
        if (address == nullptr) {
            core::events::publish("socket.endpoint.null_address");
            core::logger::fatal("SocketLib", "Null address passed to Endpoint");
            return;
        }

        // Decode the raw bytes per address family — IPv4 and IPv6 need different struct
        // layouts and different string-conversion calls (inet_ntoa vs inet_ntop).
        switch (address->sa_family) {
        case AF_INET: {
            const auto *addr = reinterpret_cast<const SOCKADDR_IN *>(address);
            m_address = inet_ntoa(addr->sin_addr);  // NOLINT(concurrency-mt-unsafe) — inet_ntoa uses a static buffer; switching to inet_ntop would change behavior/API and isn't a mechanical fix
            m_port = ntohs(addr->sin_port);
            break;
        }
        case AF_INET6: {
            const auto *addr = reinterpret_cast<const sockaddr_in6 *>(address);
            char buf[INET6_ADDRSTRLEN];
            m_address = inet_ntop(AF_INET6, &addr->sin6_addr, buf, sizeof(buf));  // FIXME(clang-tidy): array-to-pointer decay
            m_port = ntohs(addr->sin6_port);
            break;
        }
        default: {
            core::events::publish("socket.endpoint.unsupported_family");
            core::logger::fatal("SocketLib", "Unsupported address family");
        }
        }

        // Belt and suspenders — if the family-specific conversion above somehow left the
        // address empty, don't hand back a half-built Endpoint.
        if (m_address.empty()) {
            core::events::publish("socket.endpoint.address_conversion_failed");
            core::logger::fatal("SocketLib", "Failed to convert address to string");
        }
    }

    /**
     * @brief Formats back into the combined `"host:port"` form.
     * @return the formatted `"{address}:{port}"` string.
     */
    [[nodiscard]] std::string to_string() const { return std::format("{}:{}", m_address, m_port); }

    /**
     * @brief Grabs the host/IP portion.
     * @return a const reference to the stored address string.
     */
    [[nodiscard]] const std::string &get_address() const noexcept { return m_address; }
    /**
     * @brief Grabs the port number.
     * @return a const reference to the stored port.
     */
    [[nodiscard]] const std::uint16_t &get_port() const noexcept { return m_port; }

  private:
    std::string m_address;
    std::uint16_t m_port;
};

enum class VALUES : std::int8_t {
    ERRORED = 0x0,
    VALID = 0x1,
    CLEANLY_DISCONNECTED = 0x2,
    NON_BLOCKING_WOULD_HAVE_BLOCKED = 0x3,
    TIMED_OUT = 0x4
};

class SocketStatus {
  public:
    /** @brief Default's pessimistic on purpose — no value given means ERRORED, not some
     * uninitialized/valid-looking state. */
    SocketStatus() : m_value(VALUES::ERRORED) {}
    /**
     * @brief Bool-to-status shorthand — collapses the common "did it work" case down to just
     * VALID/ERRORED, skipping the finer-grained states.
     * @param is_valid `true` maps to VALID, `false` maps to ERRORED.
     */
    SocketStatus(bool is_valid) : m_value(is_valid ? VALUES::VALID : VALUES::ERRORED) {}
    /**
     * @brief Direct wrap of a specific status value — the one to reach for when you need the
     * finer-grained states (TIMED_OUT, WOULD_HAVE_BLOCKED, etc.), not just pass/fail.
     * @param value the exact status to wrap.
     */
    SocketStatus(VALUES value) : m_value(value) {}
    /**
     * @brief Wrap a status alongside the real OS/SSL error code that produced it — what
     * `get_value()` gives back is just the `VALUES` tag (e.g. `ERRORED == 0`), which is useless
     * for diagnostics; this carries the actual `errno`/`SSL_get_error()` code so callers can log
     * something meaningful instead of the tag.
     * @param value the status tag.
     * @param error_code the OS errno or SSL error code behind it, 0 if not applicable.
     */
    SocketStatus(VALUES value, int error_code) : m_value(value), m_error_code(error_code) {}

    /** @brief Trivial value type, default dtor's all that's needed. */
    ~SocketStatus() = default;

    /** @brief Trivially copyable — just the status tag plus an int error code under the hood. */
    SocketStatus(const SocketStatus &) = default;
    /** @brief Trivially movable — same deal as the copy ctor. */
    SocketStatus(SocketStatus &&) = default;
    /** @brief Trivially copy-assignable. */
    SocketStatus &operator=(const SocketStatus &) = default;
    /** @brief Trivially move-assignable. */
    SocketStatus &operator=(SocketStatus &&) = default;

    /**
     * @brief Bool-conversion shorthand — bet, this is what makes `if (status)` work at call
     * sites without spelling out `is_valid()`.
     * @return `true` for any status with a positive underlying value (currently just VALID —
     * every other VALUES entry is 0 or would-be-blocked/timed-out/etc., all non-positive or
     * treated as falsy here). Don't assume this means "operation succeeded" for every non-VALID
     * state, it specifically tracks the underlying enum's numeric sign.
     */
    operator bool() const noexcept { return std::to_underlying(m_value) > 0; }
    /**
     * @brief Grabs the raw underlying enum value as an int8.
     * @return the numeric `VALUES` code.
     */
    [[nodiscard]] std::int8_t get_value() const noexcept { return std::to_underlying(m_value); }
    /**
     * @brief Direct equality against a raw `VALUES` — lets call sites write
     * `status == VALUES::TIMED_OUT` without unwrapping first.
     * @param val the status value to compare against.
     * @return `true` if this status wraps exactly `val`.
     */
    bool operator==(VALUES val) const noexcept { return m_value == val; }

    /**
     * @brief Checks for the clean-success state specifically (not just "truthy").
     * @return `true` if this status is exactly VALID.
     */
    [[nodiscard]] bool is_valid() const noexcept { return m_value == VALUES::VALID; }
    /**
     * @brief Checks for the hard-failure state.
     * @return `true` if this status is exactly ERRORED.
     */
    [[nodiscard]] bool is_errored() const noexcept { return m_value == VALUES::ERRORED; }
    /**
     * @brief Checks for a graceful peer disconnect (as opposed to an actual error).
     * @return `true` if this status is exactly CLEANLY_DISCONNECTED.
     */
    [[nodiscard]] bool is_cleanly_disconnected() const noexcept {
        return m_value == VALUES::CLEANLY_DISCONNECTED;
    }
    /**
     * @brief Checks whether a non-blocking op would've blocked — the "come back later" signal
     * for retry/re-arm logic (see Socket::sync_handshake()'s `wait_for_write` out-param, which
     * only gets meaningful values alongside this state).
     * @return `true` if this status is exactly NON_BLOCKING_WOULD_HAVE_BLOCKED.
     */
    [[nodiscard]] bool would_have_blocked() const noexcept {
        return m_value == VALUES::NON_BLOCKING_WOULD_HAVE_BLOCKED;
    }
    /**
     * @brief Checks for the timeout state specifically.
     * @return `true` if this status is exactly TIMED_OUT.
     */
    [[nodiscard]] bool is_timed_out() const noexcept { return m_value == VALUES::TIMED_OUT; }

    /**
     * @brief Grabs the wrapped status enum directly.
     * @return a const reference to the underlying `VALUES`.
     */
    [[nodiscard]] const VALUES &get_status() const noexcept { return m_value; }
    /**
     * @brief Mutable overload of get_status().
     * @return a mutable reference to the underlying `VALUES`.
     */
    VALUES &get_status() noexcept { return m_value; }

    /**
     * @brief Grabs the real OS errno / `SSL_get_error()` code behind this status, if one was
     * given — use this for logging/diagnostics instead of get_value(), which only returns the
     * `VALUES` tag (e.g. every plain ERRORED without a code attached reads back as `0`).
     * @return the wrapped error code, or 0 if this status was built without one.
     */
    [[nodiscard]] int get_error_code() const noexcept { return m_error_code; }

  private:
    VALUES m_value;
    int m_error_code{0};
};


template <Protocol Protocol, bool RootSocket = false>
class Socket {
  public:
    /**
     * @brief Empty/invalid socket — every handle field zeroed or nulled, no address resolution,
     * no leverager. Mostly here so sync_accept()/async_accept() have something to hand back on a
     * would-block/failure path without needing `std::optional<Socket>`.
     */
    Socket()
        : m_socket{INVALID_SOCKET}, m_ssl{nullptr}, m_ssl_ctx{nullptr}, m_bio{nullptr},
          m_address_info_hint{}, m_address_info_result{nullptr}, m_socket_address_info{nullptr},
          m_socket_input_buffer{}, m_socket_input_buffer_length{0}, m_leverager{std::nullopt},
          m_ktls_tx{false}, m_ktls_rx{false}, m_os{} {}

    /**
     * @brief The main client-side ctor — resolves `endpoint` via `getaddrinfo` and opens a raw
     * socket against the first address family that succeeds. This is the one you reach for
     * before calling sync_connect()/async_connect().
     * @param endpoint the host:port to resolve and prep a socket for.
     * @param leverager optional async I/O engine reference — required for every `async_*` method
     * on this socket; leave it `std::nullopt` for sync-only usage.
     * @note Skips `AI_ADDRCONFIG` in the resolver hints for well-known loopback names/addresses
     * (localhost, ::1, 127.0.0.1, etc.) — works around a known glibc resolver quirk where
     * `AI_ADDRCONFIG` can spuriously drop loopback results, see the referenced Fedora wiki page
     * in the source comment.
     * @warning Resolution/socket-creation failures go through `core::logger::error`, not an
     * exception or return code — check is_valid() after construction if you need to know whether
     * this actually got a live socket.
     */
    Socket(Endpoint endpoint,
           std::optional<std::reference_wrapper<leverage::Leverager<leverage::Context>>> leverager =
               std::nullopt)
        : m_socket{INVALID_SOCKET}, m_ssl{nullptr}, m_ssl_ctx{nullptr}, m_bio{nullptr},
          m_endpoint{std::move(endpoint)}, m_address_info_hint{}, m_address_info_result{nullptr},
          m_socket_address_info{nullptr}, m_socket_input_buffer{}, m_socket_input_buffer_length{0},
          m_leverager{leverager}, m_ktls_tx{false}, m_ktls_rx{false}, m_os{} {
        init_address_info();

        // Don't use AI_ADDRCONFIG if connecting to loopback
        // See https://fedoraproject.org/wiki/QA/Networking/NameResolution/ADDRCONFIG
        const std::string &address = m_endpoint.get_address();
        if (address == "localhost" || address == "localhost.localdomain" ||
            address == "localhost6" || address == "localhost6.localdomain6" ||
            address == "127.0.0.1" || address == "::1") {
            m_address_info_hint.ai_flags = 0;
        }

        // Resolve host:port into every candidate address the resolver can find.
        if (getaddrinfo(address.data(), std::to_string(m_endpoint.get_port()).data(),
                        &m_address_info_hint, &m_address_info_result) != 0) {
            core::logger::error("SocketLib", "resolve failed");
            core::events::publish("socket.resolve_failed", {{"address", address}});
        }

        // Walk the resolved list and open a raw socket against the first address family that
        // actually works — no cap, we don't try to connect yet, just get an fd open.
        for (addrinfo *addr = m_address_info_result; addr != nullptr; addr = addr->ai_next) {
            m_socket = ::socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
            if (m_socket != INVALID_SOCKET) {
                m_socket_address_info = addr;
                break;
            }
        }

        if (m_socket == INVALID_SOCKET) {
            core::logger::error("SocketLib", "socket create failed");
            core::events::publish("socket.create_failed");
        } else {
            core::logger::debug("SocketLib", "socket {} ep {}:{}", m_socket,
                                m_endpoint.get_address(), m_endpoint.get_port());
        }
    }

    /**
     * @brief Wraps an already-open plain (non-SSL) native socket — this is what
     * Socket::sync_accept()/async_accept() use to wrap a freshly-accepted TCP client fd.
     * @param nativ the already-open socket handle to take ownership of.
     * @param endpoint the peer endpoint associated with this socket.
     * @param leverager optional async I/O engine reference for `async_*` methods.
     */
    Socket(SOCKET nativ, Endpoint endpoint,
           std::optional<std::reference_wrapper<leverage::Leverager<leverage::Context>>> leverager =
               std::nullopt)
        : m_socket{nativ}, m_ssl{nullptr}, m_ssl_ctx{nullptr}, m_bio{nullptr},
          m_endpoint{std::move(endpoint)}, m_address_info_hint{}, m_address_info_result{nullptr},
          m_socket_address_info{nullptr}, m_socket_input_buffer{}, m_socket_input_buffer_length{0},
          m_leverager{leverager}, m_ktls_tx{false}, m_ktls_rx{false}, m_os{} {
        init_address_info();
        core::logger::debug("SocketLib", "socket {} ep {}:{}", nativ, m_endpoint.get_address(),
                            m_endpoint.get_port());
    }

    /**
     * @brief Wraps an already-open native socket plus a pre-built SSL object — what
     * Socket::sync_accept()/async_accept() use to wrap a freshly-accepted TLS client.
     * @param nativ the already-open socket handle to take ownership of.
     * @param ssl the SSL object already associated with `nativ` — this Socket takes ownership
     * and frees it in sync_close()/async_close().
     * @param endpoint the peer endpoint associated with this socket.
     * @param leverager optional async I/O engine reference for `async_*` methods.
     */
    Socket(SOCKET nativ, SSL *ssl, Endpoint endpoint,
           std::optional<std::reference_wrapper<leverage::Leverager<leverage::Context>>> leverager =
               std::nullopt)
        : m_socket{nativ}, m_ssl{ssl}, m_ssl_ctx{nullptr}, m_bio{nullptr},
          m_endpoint{std::move(endpoint)}, m_address_info_hint{}, m_address_info_result{nullptr},
          m_socket_address_info{nullptr}, m_socket_input_buffer{}, m_socket_input_buffer_length{0},
          m_leverager{leverager}, m_ktls_tx{false}, m_ktls_rx{false}, m_os{} {
        init_address_info();
        core::logger::debug("SocketLib", "socket {} ep {}:{}", nativ, m_endpoint.get_address(),
                            m_endpoint.get_port());
    }

    /**
     * @brief QUIC-flavored accepted-connection ctor — no `Endpoint` param since QUIC connections
     * ride over a shared UDP socket, there's no dedicated per-connection fd/peer-address pairing
     * the way TCP/TLS accept gives you.
     * @warning Actual bug: `m_endpoint{nullptr}` resolves to `Endpoint(const SOCKADDR *)`, and
     * that constructor treats a null pointer as fatal — it calls `core::logger::fatal("SocketLib",
     * "Null address passed to Endpoint")` immediately, which per this codebase's logger
     * convention terminates the process. Every QUIC socket built through this ctor crashes on
     * construction as written, it's not just an edge case. Straight L, needs a real fix
     * upstream (either a genuinely-empty `Endpoint` default or dropping this init entirely).
     * @param nativ the (typically shared) socket handle this QUIC connection rides on.
     * @param ssl the SSL object for this QUIC connection — ownership taken, freed in
     * sync_close()/async_close().
     * @param leverager optional async I/O engine reference for `async_*` methods.
     */
    Socket(SOCKET nativ, SSL *ssl,
           std::optional<std::reference_wrapper<leverage::Leverager<leverage::Context>>> leverager =
               std::nullopt)
        : m_socket{nativ}, m_ssl{ssl}, m_ssl_ctx{nullptr}, m_bio{nullptr}, m_endpoint{nullptr},
          m_address_info_hint{}, m_address_info_result{nullptr}, m_socket_address_info{nullptr},
          m_socket_input_buffer{}, m_socket_input_buffer_length{0}, m_leverager{leverager},
          m_ktls_tx{false}, m_ktls_rx{false}, m_os{} {
        init_address_info();
        core::logger::debug("SocketLib", "socket {} (quic)", nativ);
    }

    /** @brief Deleted — a Socket owns a raw fd plus SSL/BIO handles, copying it would double-own
     * (and double-close) OS resources. Move it instead. */
    Socket(const Socket &) = delete;
    /** @brief Deleted — mirrors the copy ctor. */
    Socket &operator=(const Socket &) = delete;

    /**
     * @brief Steals every handle/field from `other` and resets `other` back to an invalid,
     * empty state so its dtor won't double-close anything this Socket now owns.
     * @param other the socket to move from — left invalid (`INVALID_SOCKET`, null SSL/BIO/addr
     * pointers) after the move.
     */
    Socket(Socket &&other) noexcept
        : m_socket{other.m_socket}, m_ssl{other.m_ssl},
          m_ssl_ctx{other.m_ssl_ctx}, m_bio{other.m_bio},
          m_endpoint{std::move(other.m_endpoint)},
          m_address_info_hint{other.m_address_info_hint},
          m_address_info_result{other.m_address_info_result},
          m_socket_address_info{other.m_socket_address_info},
          m_socket_input_buffer{other.m_socket_input_buffer},
          m_socket_input_buffer_length{other.m_socket_input_buffer_length},
          m_leverager{other.m_leverager}, m_ktls_tx{other.m_ktls_tx},
          m_ktls_rx{other.m_ktls_rx}, m_os{other.m_os} {
        other.m_socket = INVALID_SOCKET;
        other.m_ssl = nullptr;
        other.m_ssl_ctx = nullptr;
        other.m_bio = nullptr;
        other.m_address_info_result = nullptr;
        other.m_socket_address_info = nullptr;
    }

    /**
     * @brief Move-assigns every handle/field from `other`, then resets `other` to invalid —
     * mirrors the move ctor's steal-and-reset shape.
     * @warning This does not release `this`'s own resources before overwriting them. If `this`
     * already owned a live socket/SSL/SSL_CTX/BIO/addrinfo before the assignment, those all get
     * silently overwritten and leaked — there's no `sync_close()`/`freeaddrinfo()` call on the
     * old state here, unlike what the dtor does. Move-assigning over an already-valid Socket is
     * a real leak, not just theoretical; only safe to do onto a freshly-default-constructed or
     * already-moved-from instance.
     * @param other the socket to move from — left invalid after the move (self-assignment is a
     * guarded no-op).
     * @return a reference to `*this`.
     */
    Socket &operator=(Socket &&other) noexcept {
        // Self-assignment would otherwise steal our own fields and then null them out from
        // under ourselves — guard it out.
        if (this != &other) {
            // Steal every handle/field from `other`...
            m_socket = other.m_socket;
            m_ssl = other.m_ssl;
            m_ssl_ctx = other.m_ssl_ctx;
            m_bio = other.m_bio;
            m_endpoint = std::move(other.m_endpoint);
            m_address_info_hint = other.m_address_info_hint;
            m_address_info_result = other.m_address_info_result;
            m_socket_address_info = other.m_socket_address_info;
            m_socket_input_buffer = other.m_socket_input_buffer;
            m_socket_input_buffer_length = other.m_socket_input_buffer_length;
            m_leverager = other.m_leverager;
            m_ktls_tx = other.m_ktls_tx;
            m_ktls_rx = other.m_ktls_rx;
            m_os = other.m_os;

            // ...then reset `other` so its dtor won't double-close what we just took ownership of.
            other.m_socket = INVALID_SOCKET;
            other.m_ssl = nullptr;
            other.m_ssl_ctx = nullptr;
            other.m_bio = nullptr;
            other.m_address_info_result = nullptr;
            other.m_socket_address_info = nullptr;
            other.m_leverager = std::nullopt;
        }
        return *this;
    }

    /**
     * @brief Closes the socket (and frees the SSL/BIO/addrinfo chain) via sync_close(), unless
     * this is a non-root QUIC connection sharing its fd with another Socket instance.
     * @note QUIC connections that aren't the root socket skip cleanup here entirely — they ride
     * on a shared UDP fd owned by the root, so closing it here would pull the rug out from under
     * every other connection multiplexed on that socket.
     */
    ~Socket() {
        // Non-root QUIC connections share their fd with the root socket — skip cleanup
        // entirely so we don't pull the rug out from other connections on that fd.
        if constexpr (Protocol != Protocol::QUIC || RootSocket) {
            sync_close();
            if (m_address_info_result != nullptr) {
                freeaddrinfo(m_address_info_result);
            }
        }
    }

    /**
     * @brief Toggles O_NONBLOCK/FIONBIO on the underlying socket via the platform
     * `set_non_blocking_impl`.
     * @param non_blocking `true` to enable non-blocking mode, `false` to go back to blocking.
     */
    void set_non_blocking(bool non_blocking = true) const {
        set_non_blocking_impl(m_socket, non_blocking);
    }

    /**
     * @brief Sets/clears `SO_REUSEADDR`.
     * @param reuse `true` to allow rebinding an address still in TIME_WAIT.
     */
    void set_reuse_address(bool reuse = true) const {
        int optval = reuse ? 1 : 0;
        if (setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<char *>(&optval),  // FIXME(clang-tidy): reinterpret_cast usage
                       sizeof(optval)) != 0) {
            core::logger::error("SocketLib", "Failed to set SO_REUSEADDR");
            core::events::publish("socket.set_reuse_address_failed", {{"fd", std::to_string(m_socket)}});
        }

        core::logger::debug("SocketLib", "socket {} SO_REUSEADDR={}", m_socket, reuse);
    }


    /**
     * @brief Sets/clears `SO_BROADCAST`.
     * @param broadcast `true` to allow sending to a broadcast address.
     */
    void set_broadcast(bool broadcast = true) const {
        int optval = broadcast ? 1 : 0;
        if (setsockopt(m_socket, SOL_SOCKET, SO_BROADCAST, reinterpret_cast<char *>(&optval),  // FIXME(clang-tidy): reinterpret_cast usage
                       sizeof(optval)) != 0) {
            core::logger::error("SocketLib", "Failed to set SO_BROADCAST");
            core::events::publish("socket.set_broadcast_failed", {{"fd", std::to_string(m_socket)}});
        }

        core::logger::debug("SocketLib", "socket {} SO_BROADCAST={}", m_socket, broadcast);
    }

    /**
     * @brief Sets/clears `TCP_NODELAY` (disables/enables Nagle's algorithm) — a compile-time
     * no-op for anything but TCP/TLS.
     * @param no_delay `true` to disable Nagle's algorithm (send small writes immediately).
     */
    void set_tcp_no_delay(bool no_delay = true) const {
        // TCP_NODELAY only makes sense over an actual TCP stream — no-ops (compiles out) for
        // UDP/QUIC.
        if constexpr (Protocol == Protocol::TCP || Protocol == Protocol::TLS) {
            int optval = no_delay ? 1 : 0;
            if (setsockopt(m_socket, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<char *>(&optval),  // FIXME(clang-tidy): reinterpret_cast usage
                           sizeof(optval)) != 0) {
                core::logger::error("SocketLib", "Failed to set TCP_NODELAY");
                core::events::publish("socket.set_tcp_no_delay_failed", {{"fd", std::to_string(m_socket)}});
            }

            core::logger::debug("SocketLib", "socket {} TCP_NODELAY={}", m_socket, no_delay);
        }
    }

    /**
     * @brief Queries `SO_ERROR` off the socket to check for a pending async error.
     * @warning Actual bug: this function is declared to return `SocketStatus` and marked
     * `[[nodiscard]]`, but the body has no `return` statement at all — it just logs `error` and
     * falls off the end. Falling off the end of a value-returning function is undefined behavior
     * in C++; whatever `SocketStatus` a caller gets back here is garbage, not a real status. This
     * needs a real fix (probably `return {error == 0};` at the end), don't trust this method's
     * return value as written.
     * @return undefined — see the warning, the function never actually constructs/returns a
     * `SocketStatus`.
     */
    [[nodiscard]] SocketStatus get_status() const {
        int error = 0;
        socklen_t len = sizeof(error);
        if (getsockopt(m_socket, SOL_SOCKET, SO_ERROR, reinterpret_cast<char *>(&error), &len) !=  // FIXME(clang-tidy): reinterpret_cast usage
            0) {
            core::logger::error("SocketLib", "Failed to get socket status");
            core::events::publish("socket.get_status_failed", {{"fd", std::to_string(m_socket)}});
        }

        core::logger::debug("SocketLib", "socket {} status err={}", m_socket, error);
    }

    /**
     * @brief Shells out to the `openssl` CLI to generate a self-signed cert+key pair, skipping
     * generation entirely if both files already exist.
     * @warning Builds and runs a shell command via `std::system()` with `key_path`/`cert_path`
     * interpolated straight into the command string, unquoted-escape-wise beyond the format
     * string's own literal quotes. If either path is ever attacker-influenced (not just
     * hardcoded config), this is a command-injection footgun — no cap, don't feed this untrusted
     * input.
     * @param cert_path where to write the generated cert. Also checked for pre-existence.
     * @param key_path where to write the generated private key. Also checked for pre-existence.
     */
    void generate_certificate(std::string_view cert_path, std::string_view key_path) {
        // Both files already on disk — nothing to regenerate, bounce out early.
        if (std::filesystem::exists(key_path) && std::filesystem::exists(cert_path)) {
            core::logger::debug("Security", "SSL material already exists, skipping generation.");
            return;
        }

        // Shell out to the openssl CLI to mint a fresh self-signed cert+key pair.
        std::string command = std::format("openssl req -x509 -newkey rsa:2048 -keyout {} -out {} "
                                          "-days 365 -nodes -subj '/CN=localhost'",
                                          key_path, cert_path);

        core::logger::debug("Security", "Generating new SSL material...");

        int result = std::system(command.c_str());  // NOLINT(bugprone-command-processor,concurrency-mt-unsafe) — shelling out to the openssl CLI is the intended behavior here; replacing std::system() with exec()/posix_spawn() would be a real redesign, not a mechanical fix

        // Only trust it if the command exited clean AND the cert actually landed with content —
        // a zero exit code alone isn't proof the file's good.
        if (result == 0 && std::filesystem::exists(cert_path) &&
            std::filesystem::file_size(cert_path) > 0) {
            core::logger::debug("SocketLib", "SSL material generated.");
        } else {
            core::events::publish("socket.tls.generate_material_failed");
            core::logger::fatal("SocketLib", "Failed to generate SSL material via OpenSSL CLI.");
        }
    }


    /**
     * @brief Loads a cert chain + private key into `m_ssl_ctx`, verifying the pair actually
     * matches. TLS/QUIC-only, compiles out (well, no-ops via the `if constexpr`) for TCP/UDP.
     * @warning `cert_file.data()`/`key_file.data()` get handed to OpenSSL C APIs expecting
     * null-terminated `const char *`. `std::string_view::data()` is **not** guaranteed
     * null-terminated — it's only safe here if every caller happens to pass a view over an
     * already-NUL-terminated buffer (e.g. built from a `std::string` or string literal). Passing
     * a view over a non-NUL-terminated substring is a buffer over-read waiting to happen.
     * @param cert_file path to the certificate chain file (PEM).
     * @param key_file path to the private key file (PEM).
     * @return `true` if the chain, key, and key/cert match all loaded cleanly. For non-TLS/QUIC
     * protocols this falls through to `core::logger::fatal` at the end of the function (process
     * termination), since there's no meaningful `bool` to return there.
     */
    bool load_certificate(std::string_view cert_file, std::string_view key_file) {
        if constexpr (Protocol == Protocol::TLS || Protocol == Protocol::QUIC) {
            if (m_ssl_ctx == nullptr) {
                core::events::publish("socket.tls.context_not_initialized");
                core::logger::fatal("SocketLib",
                                    "SSL context is not initialized, cannot load certificate");
            }
            // Load the cert chain first — bail on any single step failing instead of trying
            // the rest with a half-configured context.
            if (SSL_CTX_use_certificate_chain_file(m_ssl_ctx, std::string(cert_file).c_str()) != 1) {
                char err_buf[256];
                ERR_error_string_n(ERR_get_error(), err_buf, sizeof(err_buf));  // FIXME(clang-tidy): array-to-pointer decay
                core::logger::error("SocketLib", "Failed to load chain: {}", err_buf);
                core::events::publish("socket.tls.load_chain_failed", {{"error", std::string{err_buf}}});
                return false;
            }
            // Then the private key that's supposed to go with it.
            if (SSL_CTX_use_PrivateKey_file(m_ssl_ctx, std::string(key_file).c_str(), SSL_FILETYPE_PEM) != 1) {
                char err_buf[256];
                ERR_error_string_n(ERR_get_error(), err_buf, sizeof(err_buf));  // FIXME(clang-tidy): array-to-pointer decay
                core::logger::error("SocketLib", "Failed to load key: {}", err_buf);
                core::events::publish("socket.tls.load_key_failed", {{"error", std::string{err_buf}}});
                return false;
            }
            // Finally confirm the key actually matches the cert — loading two unrelated files
            // without this check would silently produce a broken TLS context.
            if (SSL_CTX_check_private_key(m_ssl_ctx) != 1) {
                char err_buf[256];
                ERR_error_string_n(ERR_get_error(), err_buf, sizeof(err_buf));  // FIXME(clang-tidy): array-to-pointer decay
                core::logger::error(
                    "SocketLib",
                    "Key/Cert Mismatch: Private key does not match public key. Detailed Error: {}",
                    err_buf);
                core::events::publish("socket.tls.key_cert_mismatch", {{"error", std::string{err_buf}}});
                return false;
            }

            core::logger::debug("SocketLib", "cert+key loaded {} {}", cert_file, key_file);
            return true;
        }

        core::events::publish("socket.tls.load_certificate_unsupported_protocol");
        core::logger::fatal("SocketLib",
                            "Loading certificates is only supported for TLS and QUIC protocols");
    }

    /**
     * @brief Binds the socket to its resolved local address; for TLS additionally stands up
     * `m_ssl_ctx` as a server context (ALPN callback, verify mode, default verify paths).
     * @param allow_unauthorized TLS-only: when `true`, sets `SSL_VERIFY_NONE` instead of
     * requiring+verifying a peer cert. Unused (`[[maybe_unused]]`) outside the TLS branch.
     * @warning `allow_unauthorized = true` disables peer certificate verification entirely for
     * this TLS server context — fine for local dev/testing, a real security footgun if that ever
     * lands in a prod path unintentionally.
     */
    void bind([[maybe_unused]] bool allow_unauthorized = false) {
        if (m_socket_address_info == nullptr) {
            core::logger::error("SocketLib", "No valid address info to bind to");
            core::events::publish("socket.bind.no_address_info");
        }

        // Raw bind() to the resolved local address — this part's the same for every protocol.
        if (::bind(m_socket, m_socket_address_info->ai_addr, m_socket_address_info->ai_addrlen) ==
            SOCKET_ERROR) {
            core::logger::error("SocketLib", "Failed to bind socket");
            core::events::publish("socket.bind_failed", {{"fd", std::to_string(m_socket)}});
        }

        // TLS-only: stand up the server-side SSL context now that we've got a bound socket —
        // ALPN callback wiring, then peer-verification mode below.
        if constexpr (Protocol == Protocol::TLS) {
            m_ssl_ctx = SSL_CTX_new(TLS_server_method());
            if (m_ssl_ctx == nullptr) {
                core::logger::error("SocketLib", "Failed to create SSL context");
                core::events::publish("socket.tls.create_context_failed", {{"site", "bind"}});
            }

            SSL_CTX_set_info_callback(m_ssl_ctx, [](const SSL *ssl, int where, int ret) {
                if (where & SSL_CB_ALERT) {
                    const char *type = (where & SSL_CB_READ) ? "read" : "write";
                    core::logger::warning("SocketLib", "SSL alert [{}] on socket {}: {} - {}", type,
                                          reinterpret_cast<const void *>(ssl), SSL_alert_type_string_long(ret),  // FIXME(clang-tidy): reinterpret_cast usage
                                          SSL_alert_desc_string_long(ret));
                    core::events::publish("socket.tls.alert",
                                          {{"direction", type},
                                           {"alert_type", SSL_alert_type_string_long(ret)},
                                           {"alert_desc", SSL_alert_desc_string_long(ret)}});
                }
            });

            // Only wire up ALPN negotiation if the caller actually configured protocols to
            // advertise — an empty wire format means "don't bother selecting anything".
            if (!m_alpn_wire_format.empty()) {
                // TODO: fix printing here
                core::logger::debug("SocketLib", "ALPN protos: {}", m_alpn_wire_format);
                SSL_CTX_set_alpn_select_cb(
                    m_ssl_ctx,
                    [](SSL *, const unsigned char **out, unsigned char *outlen,
                       const unsigned char *client, unsigned int inlen, void *arg) -> int {
                        auto &wire = *static_cast<std::vector<unsigned char> *>(arg);
                        if (SSL_select_next_proto(const_cast<unsigned char **>(out), outlen,  // NOLINT(cppcoreguidelines-pro-type-const-cast) — OpenSSL's ALPN callback signature fixes `out` as `const unsigned char **`, but SSL_select_next_proto() requires a non-const `unsigned char **`
                                                  wire.data(),
                                                  static_cast<unsigned int>(wire.size()), client,
                                                  inlen) == OPENSSL_NPN_NEGOTIATED) {
                            return SSL_TLSEXT_ERR_OK;
                        }
                        return SSL_TLSEXT_ERR_NOACK;
                    },
                    &m_alpn_wire_format);
            }

            // Dev-mode escape hatch vs the real deal — skip peer cert verification entirely,
            // or require+verify one, per the caller's flag.
            if (allow_unauthorized) {
                SSL_CTX_set_verify(m_ssl_ctx, SSL_VERIFY_NONE, nullptr);
            } else {
                SSL_CTX_set_verify(m_ssl_ctx, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT,
                                   nullptr);
            }
            SSL_CTX_set_default_verify_paths(m_ssl_ctx);
        } else if constexpr (Protocol == Protocol::QUIC) {
            // QUIC needs its BIO wrapped around the now-bound UDP socket before listen() can
            // stand up the QUIC-flavored SSL_CTX on top of it.
            add_quic_bio();
        }

        core::logger::debug("SocketLib", "socket {} bound {}:{}", m_socket,
                            m_endpoint.get_address(), m_endpoint.get_port());
    }

    /**
     * @brief Joins a multicast group — UDP-only, resolves the group + local bind addresses,
     * opens the socket, binds, then issues `IP_ADD_MEMBERSHIP`/`IPV6_JOIN_GROUP` depending on
     * address family.
     * @param endpoint the multicast group endpoint to join.
     * @param group optional source-interface address (`imr_interface`/`ipv6mr_interface`); empty
     * defaults to `INADDR_ANY` (IPv4) or interface index 0 (IPv6).
     * @warning Calling this on a non-UDP Socket hits `core::logger::fatal` (process-terminating
     * per this codebase's logger convention) right at the top via the `Protocol != Protocol::UDP`
     * `if constexpr` guard — genuinely fatal, not a soft no-op. Only ever call this on a UDP
     * socket.
     */
    void join(const Endpoint &endpoint, std::string_view group = "") {
        // UDP-only motion — multicast groups don't mean anything over a TCP/TLS/QUIC stream.
        if constexpr (Protocol != Protocol::UDP) {
            core::events::publish("socket.multicast.join_unsupported_protocol");
            core::logger::fatal("SocketLib",
                                "Joining multicast groups is only supported for UDP protocol");
        }

        // Resolve the multicast group address itself (numeric-only, no DNS lookup needed here).
        addrinfo *multicast_addr_info{};
        addrinfo *local_addr_info{};
        addrinfo hints = {};
        hints.ai_family = PF_UNSPEC;
        hints.ai_flags = AI_NUMERICHOST;
        if (getaddrinfo(endpoint.get_address().data(), nullptr, &hints, &multicast_addr_info) !=
            0) {
            core::logger::error("SocketLib", "Failed to resolve multicast group address");
            core::events::publish("socket.multicast.resolve_group_failed");
        }

        // Now resolve a local bind address matching the group's address family and the target
        // port — this is what we'll actually open+bind a socket on.
        hints.ai_family = multicast_addr_info->ai_family;
        hints.ai_socktype = SOCK_DGRAM;
        hints.ai_flags = AI_PASSIVE;
        if (getaddrinfo(nullptr, std::to_string(endpoint.get_port()).data(), &hints,
                        &local_addr_info) != 0) {
            core::logger::error("SocketLib", "Failed to resolve local address for multicast");
            core::events::publish("socket.multicast.resolve_local_address_failed");
        }

        // Open the socket against the local address and bind it before touching multicast
        // membership — you can't join a group on a socket that isn't bound yet.
        m_socket = ::socket(local_addr_info->ai_family, local_addr_info->ai_socktype,
                            local_addr_info->ai_protocol);
        if (m_socket == INVALID_SOCKET) {
            core::logger::error("SocketLib", "Failed to create socket for multicast");
            core::events::publish("socket.multicast.create_socket_failed");
        } else {
            m_socket_address_info = local_addr_info;
        }

        bind();

        // Family-specific membership request — IPv4 and IPv6 use different mreq structs and
        // different setsockopt options, so the two branches can't be merged.
        if (multicast_addr_info->ai_family == AF_INET &&
            multicast_addr_info->ai_addrlen == sizeof(struct sockaddr_in)) {
            struct ip_mreq mreq{};

            std::memcpy(&mreq.imr_multiaddr,
                        &reinterpret_cast<struct sockaddr_in *>(multicast_addr_info->ai_addr)->sin_addr,  // FIXME(clang-tidy): reinterpret_cast usage
                        sizeof(mreq.imr_multiaddr));

            // No explicit source interface given — let the kernel pick via INADDR_ANY.
            if (group.empty()) {
                mreq.imr_interface.s_addr = htonl(INADDR_ANY);
            } else {
                mreq.imr_interface.s_addr = inet_addr(std::string(group).c_str());
            }

            if (setsockopt(m_socket, IPPROTO_IP, IP_ADD_MEMBERSHIP, reinterpret_cast<char *>(&mreq),  // FIXME(clang-tidy): reinterpret_cast usage
                           sizeof(mreq)) != 0) {
                core::logger::error("SocketLib", "Failed to join multicast group");
                core::events::publish("socket.multicast.join_failed", {{"family", "inet"}});
            }
        } else if (multicast_addr_info->ai_family == AF_INET6 &&
                   multicast_addr_info->ai_addrlen == sizeof(struct sockaddr_in6)) {
            struct ipv6_mreq mreq6{};

            std::memcpy(&mreq6.ipv6mr_multiaddr,
                        &reinterpret_cast<struct sockaddr_in6 *>(multicast_addr_info->ai_addr)->sin6_addr,
                        sizeof(mreq6.ipv6mr_multiaddr));

            // IPv6 wants an interface index rather than an address — resolve the group string
            // and pull its scope id if one was given, else default to "any interface" (0).
            if (group.empty()) {
                mreq6.ipv6mr_interface = 0;
            } else {
                struct addrinfo *group_addr_info{};
                if (getaddrinfo(std::string(group).c_str(), nullptr, nullptr, &group_addr_info) != 0) {
                    core::logger::error("SocketLib",
                                        "Failed to resolve group address for multicast");
                    core::events::publish("socket.multicast.resolve_group_interface_failed");
                }

                mreq6.ipv6mr_interface = reinterpret_cast<sockaddr_in6 *>(group_addr_info->ai_addr)->sin6_scope_id;
                freeaddrinfo(group_addr_info);
            }

            if (setsockopt(m_socket, IPPROTO_IPV6, IPV6_JOIN_GROUP,
                           reinterpret_cast<char *>(&mreq6), sizeof(mreq6)) != 0) {  // FIXME(clang-tidy): reinterpret_cast usage
                core::logger::error("SocketLib", "Failed to join multicast group");
                core::events::publish("socket.multicast.join_failed", {{"family", "inet6"}});
            }
        } else {
            core::logger::error("SocketLib", "Unsupported address family for multicast");
            core::events::publish("socket.multicast.unsupported_family");
        }

        core::logger::debug("SocketLib", "socket {} joined multicast {} {}:{}", m_socket,
                            group.empty() ? "(default)" : group, endpoint.get_address(),
                            endpoint.get_port());

        freeaddrinfo(multicast_addr_info);
    }

    /**
     * @brief Starts listening for incoming connections — plain `::listen()` for TCP/TLS, or
     * stands up a server-side QUIC `SSL`/`SSL_CTX` over the existing BIO for QUIC.
     * @warning QUIC branch requires add_quic_bio() (or an equivalent BIO setup) to have already
     * run — `m_bio == nullptr` only logs an error and keeps going, then immediately calls
     * `SSL_set_bio(m_ssl, m_bio, m_bio)` with that null BIO anyway. Not a hard stop.
     * @note Any protocol besides TCP/TLS/QUIC (i.e. plain UDP) routes to
     * `core::logger::fatal` — properly gated this time (unlike shutdown()'s bug), this is the
     * genuine terminal `else` branch of a real if/else-if/else chain, so it *only* fires for
     * unsupported protocols.
     */
    void listen() {
        // Plain TCP/TLS listen — the SSL context for TLS was already stood up in bind().
        if constexpr (Protocol == Protocol::TCP || Protocol == Protocol::TLS) {
            if (::listen(m_socket, SOMAXCONN) == SOCKET_ERROR) {
                core::logger::error("SocketLib", "Failed to listen on socket");
                core::events::publish("socket.listen_failed", {{"fd", std::to_string(m_socket)}});
            }
            core::logger::debug("SocketLib", "socket {} listening {}:{}", m_socket,
                                m_endpoint.get_address(), m_endpoint.get_port());
        } else if constexpr (Protocol == Protocol::QUIC) {
            // QUIC has no real "listen" syscall — it's all built on top of the datagram BIO
            // set up earlier in bind(), so make sure that actually happened first.
            if (m_bio == nullptr) {
                core::logger::error("SocketLib",
                                    "BIO must be initialized before listening for QUIC");
                core::events::publish("socket.quic.bio_not_initialized");
            }

            // Stand up a server-side QUIC SSL_CTX/SSL pair over that BIO.
            m_ssl_ctx = SSL_CTX_new(OSSL_QUIC_server_method());
            if (m_ssl_ctx == nullptr) {
                core::logger::error("SocketLib", "Failed to create SSL context for QUIC");
                core::events::publish("socket.tls.create_context_failed", {{"site", "listen_quic"}});
            }

            SSL_CTX_set_info_callback(m_ssl_ctx, [](const SSL *ssl, int where, int ret) {
                if (where & SSL_CB_ALERT) {
                    const char *type = (where & SSL_CB_READ) ? "read" : "write";
                    core::logger::warning("SocketLib", "SSL alert [{}] on socket {}: {} - {}", type,
                                          reinterpret_cast<const void *>(ssl), SSL_alert_type_string_long(ret),  // FIXME(clang-tidy): reinterpret_cast usage
                                          SSL_alert_desc_string_long(ret));
                    core::events::publish("socket.tls.alert",
                                          {{"direction", type},
                                           {"alert_type", SSL_alert_type_string_long(ret)},
                                           {"alert_desc", SSL_alert_desc_string_long(ret)}});
                }
            });

            m_ssl = SSL_new(m_ssl_ctx);
            if (m_ssl == nullptr) {
                core::logger::error("SocketLib", "Failed to create SSL object for QUIC");
                core::events::publish("socket.tls.create_ssl_object_failed", {{"site", "listen_quic"}});
            }

            // Wire the SSL object to the shared BIO and put it in server (accept) mode.
            SSL_set_bio(m_ssl, m_bio, m_bio);
            SSL_set_accept_state(m_ssl);

            core::logger::debug("SocketLib", "socket {} listening (quic) {}:{}", m_socket,
                                m_endpoint.get_address(), m_endpoint.get_port());
        } else {
            core::events::publish("socket.listen_unsupported_protocol");
            core::logger::fatal("SocketLib",
                                "Listen is only supported for TCP, TLS, and QUIC protocols");
        }
    }

    /**
     * @brief Thin wrapper over the posix `select()` syscall for a single fd (this socket's own),
     * watching whichever of read/write/except are requested.
     * @param event_mask actually an `Event` bitmask (READ/WRITE/EXCEPT), despite the misleading
     * param name — matched against `m_socket` for each requested direction, not a separate fd to
     * watch.
     * @param timeout_ms how long to wait before giving up.
     * @return VALID if `select()` reports the socket ready, TIMED_OUT if the timeout elapsed
     * first, NON_BLOCKING_WOULD_HAVE_BLOCKED on `EINTR`/`EAGAIN`/`EWOULDBLOCK`, or ERRORED on any
     * other `select()` failure.
     */
    [[nodiscard]] SocketStatus select(int event_mask, std::uint64_t timeout_ms) const noexcept {
        fd_set readfds;
        fd_set writefds;
        fd_set exceptfds;

        fd_set *p_read = nullptr;
        fd_set *p_write = nullptr;
        fd_set *p_except = nullptr;

        // Only build (and watch) the fd_sets for whichever directions the caller's bitmask
        // actually asked for — select() wants nullptr for the ones we're not watching.
        if ((event_mask & std::to_underlying(Event::READ)) != 0) {
            FD_ZERO(&readfds);
            FD_SET(m_socket, &readfds);
            p_read = &readfds;
        }
        if ((event_mask & std::to_underlying(Event::WRITE)) != 0) {
            FD_ZERO(&writefds);
            FD_SET(m_socket, &writefds);
            p_write = &writefds;
        }
        if ((event_mask & std::to_underlying(Event::EXCEPT)) != 0) {
            FD_ZERO(&exceptfds);
            FD_SET(m_socket, &exceptfds);
            p_except = &exceptfds;
        }

        struct timeval tv{};
        tv.tv_sec = static_cast<decltype(tv.tv_sec)>(timeout_ms / 1000);
        tv.tv_usec = static_cast<decltype(tv.tv_usec)>((timeout_ms % 1000) * 1000);

        // Three-way outcome: real failure, timeout with nothing ready, or the fd's actually
        // ready — each maps to a different SocketStatus.
        int result = ::select(static_cast<int>(m_socket + 1), p_read, p_write, p_except, &tv);
        if (result < 0) {
            const auto ERR = get_error_code();
            if (ERR == EINTR || ERR == EAGAIN || ERR == EWOULDBLOCK) {
                core::logger::warning("SocketLib", "socket {} select blocked/interrupted",
                                      m_socket);
                core::events::publish("socket.select.blocked_interrupted",
                                      {{"fd", std::to_string(m_socket)}});
                return {VALUES::NON_BLOCKING_WOULD_HAVE_BLOCKED};
            }
            core::logger::warning("SocketLib",
                                  "Critical failure in select() syscall with error code `{}`", ERR);
            core::events::publish("socket.select.failed",
                                  {{"fd", std::to_string(m_socket)}, {"error_code", std::to_string(ERR)}});
            return {VALUES::ERRORED};
        }
        if (result == 0) {
            core::logger::warning("SocketLib", "socket {} select timeout {}ms", m_socket,
                                  timeout_ms);
            core::events::publish("socket.select.timeout",
                                  {{"fd", std::to_string(m_socket)}, {"timeout_ms", std::to_string(timeout_ms)}});
            return {VALUES::TIMED_OUT};
        }

        core::logger::debug("SocketLib", "socket {} select ready", m_socket);
        return {VALUES::VALID};
    }

    /**
     * @brief Synchronously connects, trying `m_socket_address_info` first and falling back
     * through every other resolved address in `m_address_info_result` on failure. TLS/QUIC
     * additionally runs the handshake setup (SSL context/object) once the raw connect lands.
     * @param timeout milliseconds to wait per connect attempt (0 for blocking-forever connect);
     * forwarded straight to the private connect() helper.
     * @return VALID once connected (and, for TLS/QUIC, TLS-context-ready); ERRORED if every
     * address fails to connect, or if TLS/QUIC setup fails after a successful raw connect.
     * @note UDP isn't in the supported-protocol list here and falls to `core::logger::fatal`
     * (process-terminating) — properly gated by a real if/else, not the shutdown()-style bug.
     */
    SocketStatus sync_connect(std::uint64_t timeout = 0) {
        if constexpr (Protocol == Protocol::TCP || Protocol == Protocol::TLS ||
                      Protocol == Protocol::QUIC) {
            // Try the address the ctor already opened a socket against first — only fall back
            // to walking the rest of the resolved list if that one doesn't connect.
            auto *current = m_socket_address_info;
            if (connect<false>(current, timeout) != SocketStatus(VALUES::VALID)) {
                for (auto *addr = m_address_info_result->ai_next; addr != nullptr;
                     addr = addr->ai_next) {
                    // Already checked this one, skip it
                    if (addr == current) {
                        continue;
                    }
                    if (connect<true>(addr, timeout) == SocketStatus(VALUES::VALID)) {
                        break;
                    }
                }
            }

            if (m_socket == INVALID_SOCKET) {
                core::logger::error("SocketLib", "Failed to connect to any resolved address");
                core::events::publish("socket.connect.all_addresses_failed",
                                      {{"address", m_endpoint.get_address()}});
            }

            // Raw TCP connect landed — now layer the TLS/QUIC handshake prep on top before
            // reporting success.
            if constexpr (Protocol == Protocol::TLS) {
                if (!setup_tls()) {
                    return {VALUES::ERRORED};
                }
            } else if constexpr (Protocol == Protocol::QUIC) {
                add_quic_bio();

                if (!setup_quic()) {
                    return {VALUES::ERRORED};
                }
            }

            core::logger::debug("SocketLib", "socket {} connected {}:{}", m_socket,
                                m_endpoint.get_address(), m_endpoint.get_port());
            return {VALUES::VALID};
        } else {
            core::events::publish("socket.connect_unsupported_protocol");
            core::logger::fatal("SocketLib",
                                "Connect is only supported for TCP, TLS, QUIC protocols");
        }
    }

    /**
     * @brief Async counterpart to sync_connect() — kicks off a non-blocking connect through the
     * leverager, and on failure re-arms itself against the next resolved address, walking
     * `m_address_info_result`'s linked list one attempt per completion until one succeeds or the
     * list runs out.
     * @param callback invoked once with the final SocketStatus — VALID once connected (and, for
     * TLS/QUIC, TLS-context-ready), TIMED_OUT/NON_BLOCKING_WOULD_HAVE_BLOCKED/ERRORED depending
     * on what killed the last attempt.
     * @param iflags forwarded to each leverager connect() call.
     * @warning Requires `m_leverager` to be set — calling this without one routes to
     * `core::logger::fatal` (process-terminating).
     */
    // readability-function-cognitive-complexity: genuinely fixed, not NOLINT'd — the three
    // distinct outcomes of one re-arm attempt (connect landed, ran out of candidates, still got
    // candidates left) are now complete_async_connect(), report_final_async_connect_error(), and
    // reopen_socket_for_connect_retry(), private methods below. Logic, ordering, and the
    // retry/fallback behavior are unchanged — this is a straight mechanical split of the lambda
    // body into named steps.
    void async_connect(std::move_only_function<void(SocketStatus)> callback,
                       std::uint8_t iflags = 0) {
        if constexpr (Protocol == Protocol::TCP || Protocol == Protocol::TLS ||
                      Protocol == Protocol::QUIC) {
            if (m_leverager) {
                auto *current = (m_socket_address_info != nullptr) ? m_socket_address_info
                                                                   : m_address_info_result;

                set_non_blocking(true);

                auto attempt = std::make_shared<std::function<void(int)>>();
                *attempt = [this, current, callback = std::move(callback), iflags,
                            attempt](int res, std::uint32_t) mutable {
                    // Connect landed — layer TLS/QUIC setup on top same as sync_connect(), then
                    // report success.
                    if (res == 0) {
                        complete_async_connect(current, callback);
                        return;
                    }

                    // This attempt failed — advance to the next resolved address. Ran out of
                    // candidates: classify the last error and report a final status.
                    current = current->ai_next;
                    if (!current) {
                        report_final_async_connect_error(res, callback);
                        return;
                    }

                    // close();

                    // Still got candidates left — open a fresh socket for the next address
                    // family and re-arm the async connect against it.
                    if (!reopen_socket_for_connect_retry(current)) {
                        callback(SocketStatus(VALUES::ERRORED));
                        return;
                    }

                    m_leverager->get().connect(m_socket, current->ai_addr, current->ai_addrlen,
                                               *attempt, iflags);
                };

                // Kick off the first attempt against the currently-open socket/address.
                m_leverager->get().connect(m_socket, current->ai_addr, current->ai_addrlen,
                                           *attempt, iflags);
            } else {
                core::events::publish("socket.leverager_not_set", {{"site", "async_connect"}});
                core::logger::fatal("SocketLib",
                                    "m_leverager is not set so async funtion calls cannot be used");
            }
        }
    }


    /// Call this after connect() or after accept() returns a socket, each time
    /// the completion queue signals the fd is ready. Returns:
    ///   socket_status::valid                       — handshake complete, ready to send/recv
    ///   socket_status::non_blocking_would_have_blocked — not done yet; re-arm the fd
    ///     *wait_for_write is set to tell you which direction to wait on:
    ///       false → wait for the fd to be readable  (EPOLLIN / IOCP recv)
    ///       true  → wait for the fd to be writable  (EPOLLOUT / IOCP send)
    ///   socket_status::errored                     — handshake failed, close the socket
    ///
    /// For plain tcp sockets this is a no-op and always returns valid.
    /// @warning Stale relative to the current body below — plain TCP does NOT take a no-op/valid
    /// path here. The `else` branch for non-TLS/QUIC protocols calls `core::logger::fatal`
    /// (process-terminating per this codebase's convention), not a silent valid return. Trust the
    /// code over this comment on that specific point.
    SocketStatus sync_handshake(bool *wait_for_write = nullptr) noexcept {
        if constexpr (Protocol == Protocol::TLS || Protocol == Protocol::QUIC) {
            if (m_ssl == nullptr) {
                core::logger::warning("SocketLib", "SSL object is not initialized for handshake");
                core::events::publish("socket.tls.ssl_not_initialized", {{"site", "sync_handshake"}});
                return {VALUES::ERRORED};
            }

            // Drive the handshake state machine forward one step.
            int ret = SSL_do_handshake(m_ssl);
            if (ret == 1) {
                // Done — for plain TLS (not QUIC, which handles its own record layer), check
                // whether the kernel actually took over TX/RX via kTLS offload.
                if constexpr (Protocol == Protocol::TLS) {
                    m_ktls_tx = BIO_get_ktls_send(SSL_get_wbio(m_ssl));
                    m_ktls_rx = BIO_get_ktls_recv(SSL_get_rbio(m_ssl));
                    if (m_ktls_tx) {
                        core::logger::debug("SocketLib", "socket {} kTLS TX active", m_socket);
                    } else {
                        core::logger::debug("SocketLib", "socket {} kTLS TX inactive", m_socket);
                    }
                    if (m_ktls_rx) {
                        core::logger::debug("SocketLib", "socket {} kTLS RX active", m_socket);
                    } else {
                        core::logger::debug("SocketLib", "socket {} kTLS RX inactive", m_socket);
                    }
                }
                core::logger::debug("SocketLib", "socket {} handshake done", m_socket);
                return {VALUES::VALID};
            }

            // Not done yet — tell the caller which direction to re-arm on (read or write)
            // depending on what OpenSSL says it's blocked waiting for.
            int err = SSL_get_error(m_ssl, ret);
            if (err == SSL_ERROR_WANT_READ) {
                if (wait_for_write != nullptr) {
                    *wait_for_write = false;
                }
                core::logger::debug("SocketLib",
                                    "socket {} handshake wait-read (pending={} want={})", m_socket,
                                    SSL_pending(m_ssl), SSL_want(m_ssl));
                return {VALUES::NON_BLOCKING_WOULD_HAVE_BLOCKED};
            }
            if (err == SSL_ERROR_WANT_WRITE) {
                if (wait_for_write != nullptr) {
                    *wait_for_write = true;
                }
                core::logger::debug("SocketLib",
                                    "socket {} handshake wait-write (pending={} want={})", m_socket,
                                    SSL_pending(m_ssl), SSL_want(m_ssl));
                return {VALUES::NON_BLOCKING_WOULD_HAVE_BLOCKED};
            }

            char err_buf[256];
            ERR_error_string_n(ERR_get_error(), err_buf, sizeof(err_buf));  // FIXME(clang-tidy): array-to-pointer decay
            core::logger::warning(
                "SocketLib",
                "Socket `{}` handshake failed: SSL_get_error={} detail={}", m_socket, err,
                err_buf);
            core::events::publish("socket.tls.handshake_failed",
                                  {{"fd", std::to_string(m_socket)},
                                   {"ssl_error", std::to_string(err)},
                                   {"detail", std::string{err_buf}}});
            return {VALUES::ERRORED};
        } else {
            core::events::publish("socket.handshake_plain_tcp", {{"fd", std::to_string(m_socket)}});
            core::logger::fatal("SocketLib",
                                "Socket `{}` is a plain TCP socket, no handshake needed", m_socket);
        }
    }

    /**
     * @brief Async counterpart to sync_handshake() — jumpstarts the handshake, and every time
     * `sync_handshake()` reports would-have-blocked, re-arms itself on a zero-length recv/send
     * through the leverager (per the `wait_for_write` direction) until the handshake completes
     * or fails.
     * @param callback invoked once with the final SocketStatus.
     * @param iflags forwarded to each leverager recv()/send() re-arm call.
     * @warning TLS/QUIC only. For any other protocol, or if `m_leverager` isn't set, this routes
     * to `core::logger::fatal` (process-terminating) — same "plain TCP socket, no handshake
     * needed" message as sync_handshake(), same caveat about that being a hard crash, not a
     * no-op.
     */
    void async_handshake(std::move_only_function<void(SocketStatus)> callback,
                         std::uint8_t iflags = 0) noexcept {
        if constexpr (Protocol == Protocol::TLS || Protocol == Protocol::QUIC) {
            if (m_leverager) {
                auto attempt = std::make_shared<std::function<void(int)>>();
                *attempt = [this, callback = std::move(callback), iflags,
                            attempt](int res, std::uint32_t) mutable {
                    // The re-arm itself failed — no point driving the handshake further.
                    if (res < 0) {
                        core::logger::warning(
                            "SocketLib", "socket {} async handshake failed err={}", m_socket, res);
                        core::events::publish("socket.tls.async_handshake_failed",
                                              {{"fd", std::to_string(m_socket)},
                                               {"error_code", std::to_string(res)}});
                        callback(SocketStatus(VALUES::ERRORED));
                        return;
                    }

                    // Push the handshake state machine forward one step synchronously.
                    bool wait_for_write = false;
                    SocketStatus status = sync_handshake(&wait_for_write);

                    // Still not done — re-arm on whichever direction sync_handshake() said it's
                    // waiting on (zero-length recv/send just re-triggers the leverager on that fd).
                    if (status.would_have_blocked()) {
                        if (wait_for_write) {
                            m_leverager->get().send(m_socket, nullptr, 0, 0, *attempt, iflags);
                        } else {
                            m_leverager->get().recv(m_socket, nullptr, 0, 0, *attempt, iflags);
                        }
                    }

                    callback(status);
                };

                // Call to jumpstart the async handshake process, it will immediately call the
                // callback which will then re-arm itself until the handshake is complete or fails
                (*attempt)(0);
            } else {
                core::events::publish("socket.leverager_not_set", {{"site", "async_handshake"}});
                core::logger::fatal("SocketLib",
                                    "m_leverager is not set so async funtion calls cannot be used");
            }
        } else {
            core::events::publish("socket.handshake_plain_tcp", {{"fd", std::to_string(m_socket)},
                                                                 {"site", "async_handshake"}});
            core::logger::fatal("SocketLib",
                                "Socket `{}` is a plain TCP socket, no handshake needed", m_socket);
        }
    }

    /**
     * @brief Checks whether the TLS/SSL handshake has completed.
     * @warning Likely a copy-paste bug: the `if constexpr` checks `TCP || QUIC`, not `TLS ||
     * QUIC`. `m_ssl` is never set for plain TCP sockets (only TLS/QUIC ever populate it), so this
     * always evaluates `(nullptr != nullptr) && ...` and returns `false` for every TCP socket —
     * and worse, **TLS sockets fall through to the unconditional `return true;` at the bottom
     * instead of actually checking `SSL_is_init_finished`**, since `Protocol::TLS` isn't in the
     * condition at all. So this reports "handshake done" for TLS unconditionally and "handshake
     * not done" for TCP unconditionally — both backwards from what the name promises. Don't rely
     * on this to gate real handshake-completion logic until the condition is fixed to `TLS ||
     * QUIC`.
     * @return per the current (likely buggy) condition: real SSL-state-based result for QUIC,
     * always `false` for TCP, always `true` for TLS and UDP.
     */
    [[nodiscard]] bool is_handshake_done() const noexcept {
        // See the @warning above — this condition is likely supposed to read `TLS || QUIC`.
        if constexpr (Protocol == Protocol::TCP || Protocol == Protocol::QUIC) {
            return (m_ssl != nullptr) && (SSL_is_init_finished(m_ssl) != 0);
        }
        return true;
    }

    /**
     * @brief Blocking accept — plain `::accept()` for TCP/TLS (wrapping the client fd in a fresh
     * SSL object for TLS), or `SSL_accept_connection` for QUIC.
     * @warning The TCP/TLS accepted-socket constructors here (`Socket<Protocol>{client_fd, ...}`)
     * are called *without* `m_leverager` — unlike async_accept(), which does thread it through.
     * A socket handed back by this method has no async engine wired up, so any `async_*` call on
     * it will hit the "m_leverager is not set" fatal. If you need the accepted connection to
     * support async I/O, use async_accept() instead.
     * @return a live client Socket, or a default-constructed (invalid) one if `accept()` would've
     * blocked (`EWOULDBLOCK`/`EAGAIN`/`EINTR`).
     * @note UDP isn't accept-able and routes to `core::logger::fatal` — properly gated by a real
     * if/else-if/else chain here.
     */
    [[nodiscard]] Socket<Protocol> sync_accept() const {
        if constexpr (Protocol == Protocol::TCP || Protocol == Protocol::TLS) {
            sockaddr_storage client_addr{};
            socklen_t addr_len = sizeof(client_addr);

            // Would-block/interrupted isn't a real failure — just hand back an invalid Socket
            // so the caller knows to try again later.
            SOCKET client_fd =
                ::accept(m_socket, reinterpret_cast<sockaddr *>(&client_addr), &addr_len);  // FIXME(clang-tidy): reinterpret_cast usage
            if (client_fd == INVALID_SOCKET) {
                const auto ERR = get_error_code();

                if (ERR == EWOULDBLOCK || ERR == EAGAIN || ERR == EINTR) {
                    core::logger::debug("SocketLib", "socket {} accept would block", m_socket);
                    return Socket<Protocol>{};
                }

                core::logger::error(
                    "SocketLib", "Critical failure in accept() syscall with error code `{}`", ERR);
                core::events::publish("socket.accept.failed",
                                      {{"fd", std::to_string(m_socket)}, {"error_code", std::to_string(ERR)}});
            }

            // TLS: wrap the freshly-accepted fd in its own SSL object, in server (accept) mode,
            // before handing it back as a Socket.
            if constexpr (Protocol == Protocol::TLS) {
                SSL *client_ssl = SSL_new(m_ssl_ctx);
                if (client_ssl == nullptr) {
                    closesocket(client_fd);
                    core::logger::error("SocketLib",
                                        "Failed to create SSL object for accepted connection");
                    core::events::publish("socket.tls.create_ssl_object_failed", {{"site", "sync_accept"}});
                }

                if (SSL_set_fd(client_ssl, client_fd) == 0) {
                    SSL_free(client_ssl);
                    closesocket(client_fd);
                    core::logger::error("SocketLib",
                                        "Failed to associate SSL object with accepted socket");
                    core::events::publish("socket.tls.set_fd_failed", {{"site", "sync_accept"}});
                }

                SSL_set_accept_state(client_ssl);

                auto endpoint = Endpoint(reinterpret_cast<sockaddr *>(&client_addr));  // FIXME(clang-tidy): reinterpret_cast usage
                core::logger::debug("SocketLib", "socket {} accepted TLS from {}:{}", m_socket,
                                    endpoint.get_address(), endpoint.get_port());

                return Socket<Protocol>{client_fd, client_ssl, std::move(endpoint)};
            }

            // Plain TCP: just wrap the fd, no SSL object needed.
            auto endpoint = Endpoint(reinterpret_cast<sockaddr *>(&client_addr));  // FIXME(clang-tidy): reinterpret_cast usage
            core::logger::debug("SocketLib", "socket {} accepted TCP from {}:{}", m_socket,
                                endpoint.get_address(), endpoint.get_port());

            return Socket<Protocol>{client_fd, std::move(endpoint)};
        } else if constexpr (Protocol == Protocol::QUIC) {
            // QUIC connections aren't accepted off a listening fd the BSD-socket way — they're
            // pulled off the shared UDP socket's SSL object as individual QUIC streams/conns.
            if (m_ssl == nullptr) {
                core::logger::error("SocketLib",
                                    "Socket `{}` SSL object is not initialized for QUIC accept",
                                    m_socket);
                core::events::publish("socket.tls.ssl_not_initialized",
                                      {{"fd", std::to_string(m_socket)}, {"site", "sync_accept_quic"}});
            }

            SSL *client_ssl = SSL_accept_connection(m_ssl, 0);
            if (client_ssl == nullptr) {
                core::logger::error("SocketLib",
                                    "Socket `{}` failed to accept new QUIC connection ", m_socket);
                core::events::publish("socket.quic.accept_failed", {{"fd", std::to_string(m_socket)}});
            }

            core::logger::debug("SocketLib", "socket {} accepted QUIC connection", m_socket);
            return Socket<Protocol>{m_socket, client_ssl};

        } else {
            core::events::publish("socket.accept_unsupported_protocol", {{"site", "sync_accept"}});
            core::logger::fatal("SocketLib",
                                "Accept is only supported for TCP, TLS and QUIC protocols");
        }
    }

    /**
     * @brief Async counterpart to sync_accept() — queues an accept through the leverager for
     * TCP/TLS, wraps the resulting client fd in a fresh SSL object for TLS, and threads
     * `m_leverager` through to the accepted Socket so it can itself do async I/O.
     * @param callback invoked with the accepted client Socket (TCP/TLS path), or a
     * default-constructed invalid one on failure/would-block.
     * @param iflags forwarded to the leverager's accept() call.
     * @warning Actual bug, won't compile as written for `Protocol::QUIC`: the QUIC branch does
     * `return Socket<Protocol>{m_socket, client_ssl};`, but this function returns `void` —
     * returning a value from a void function is ill-formed C++. This looks like leftover
     * copy-paste from sync_accept()'s QUIC branch; it should be calling `callback(...)` instead
     * of `return`ing. Any instantiation of this template for QUIC will fail to compile until
     * that's fixed.
     */
    // bugprone-exception-escape: not `noexcept` (see below) — this method isn't a virtual
    // override and doesn't satisfy any AsyncSendable/AsyncReceivable/AsyncClose-style concept in
    // interfaces::io (grepped — async_accept isn't referenced there at all), so there's no
    // contract requiring it to stay noexcept. Dropping the qualifier is the genuine fix: the
    // std::make_shared allocations and the leverager's accept() call can legitimately throw
    // (bad_alloc, submission-time failures), and letting that propagate to the caller is more
    // honest than pretending it can't.
    // readability-function-cognitive-complexity: genuinely fixed, not NOLINT'd — the
    // would-block-vs-critical-failure classification is now report_async_accept_failure(), and
    // the TLS SSL-object setup is now create_accepted_tls_ssl(), both private methods below.
    void async_accept(std::move_only_function<void(Socket<Protocol>)> callback,
                      std::uint8_t iflags = 0) {
        if constexpr (Protocol == Protocol::TCP || Protocol == Protocol::TLS) {
            if (m_leverager) {
                auto client_addr = std::make_shared<sockaddr_storage>();
                auto addr_len = std::make_shared<socklen_t>(sizeof(sockaddr_storage));

                m_leverager->get().accept(
                    m_socket, reinterpret_cast<sockaddr *>(client_addr.get()), addr_len.get(), 0,  // FIXME(clang-tidy): reinterpret_cast usage
                    [this, client_addr, addr_len,
                     callback = std::move(callback)](int res, std::uint32_t) mutable {
                        // Same would-block-vs-real-failure split as sync_accept(), just routed
                        // through the callback instead of a direct return.
                        if (report_async_accept_failure(res, callback)) {
                            return;
                        }

                        auto client_fd = static_cast<SOCKET>(res);

                        // TLS: wrap the fd in a fresh SSL object same as sync_accept(), but
                        // thread m_leverager through so the accepted Socket can do async I/O too.
                        if constexpr (Protocol == Protocol::TLS) {
                            SSL *client_ssl = create_accepted_tls_ssl(client_fd);
                            if (client_ssl == nullptr) {
                                callback(Socket<Protocol>{});
                                return;
                            }

                            auto endpoint =
                                Endpoint(reinterpret_cast<sockaddr *>(client_addr.get()));  // FIXME(clang-tidy): reinterpret_cast usage
                            core::logger::debug("SocketLib", "socket {} accepted TLS from {}:{}",
                                                m_socket, endpoint.get_address(),
                                                endpoint.get_port());
                            callback(Socket<Protocol>{client_fd, client_ssl, std::move(endpoint),
                                                      m_leverager});
                            return;
                        }
                        // Plain TCP: wrap the fd, still threading m_leverager through.
                        auto endpoint = Endpoint(reinterpret_cast<sockaddr *>(client_addr.get()));  // FIXME(clang-tidy): reinterpret_cast usage
                        core::logger::debug("SocketLib", "socket {} accepted TCP from {}:{}",
                                            m_socket, endpoint.get_address(), endpoint.get_port());

                        callback(Socket<Protocol>{client_fd, std::move(endpoint), m_leverager});
                    },
                    iflags);
            } else {
                core::events::publish("socket.leverager_not_set", {{"site", "async_accept"}});
                core::logger::fatal("SocketLib",
                                    "m_leverager is not set so async funtion calls cannot be used");
            }
        } else if constexpr (Protocol == Protocol::QUIC) {
            if (m_ssl == nullptr) {
                core::logger::error("SocketLib",
                                    "Socket `{}` SSL object is not initialized for QUIC accept",
                                    m_socket);
                core::events::publish("socket.tls.ssl_not_initialized",
                                      {{"fd", std::to_string(m_socket)}, {"site", "async_accept_quic"}});
            }

            SSL *client_ssl = SSL_accept_connection(m_ssl, 0);
            if (client_ssl == nullptr) {
                core::logger::error("SocketLib",
                                    "Socket `{}` failed to accept new QUIC connection ", m_socket);
                core::events::publish("socket.quic.accept_failed", {{"fd", std::to_string(m_socket)}});
            }

            core::logger::debug("SocketLib", "socket {} accepted QUIC connection", m_socket);
            return Socket<Protocol>{m_socket, client_ssl};

        } else {
            core::events::publish("socket.accept_unsupported_protocol", {{"site", "async_accept"}});
            core::logger::fatal("SocketLib",
                                "Accept is only supported for TCP, TLS and QUIC protocols");
        }
    }


    /**
     * @brief Blocking send — plain `::send()` for TCP (or TLS with kTLS active), `SSL_write_ex`
     * for TLS/QUIC without kTLS, `::sendto()`/`::send()` for UDP depending on whether `addr` is
     * given.
     * @param buffer bytes to send — caller-owned, only read during the call.
     * @param length number of bytes in `buffer` to send.
     * @param addr UDP-only: explicit destination address; null falls back to
     * `m_socket_address_info` (the endpoint this socket connected/resolved to).
     * @return the number of bytes actually sent alongside a status: VALID on a normal send,
     * NON_BLOCKING_WOULD_HAVE_BLOCKED if the socket/SSL object would've blocked,
     * CLEANLY_DISCONNECTED if the peer closed (SSL zero-return), or ERRORED on any other failure.
     * Bytes sent is always 0 on a non-VALID status.
     */
    // readability-function-cognitive-complexity: genuinely fixed, not NOLINT'd — the SSL-error
    // mapping switch (want-read/want-write/zero-return/syscall/default) is now map_send_ssl_error(),
    // a private method below, pulled out purely to bring this function's complexity down; logic
    // and ordering are unchanged.
    std::pair<std::size_t, SocketStatus> sync_send(const std::byte *buffer,
                                                   const std::size_t LENGTH,
                                                   AddressInfo *addr = nullptr) const {
        std::size_t sent_bytes{0};
        int ssl_call_result{0};

        // Dispatch the actual write per-protocol — plain send() for TCP/UDP, SSL_write_ex for
        // TLS/QUIC unless kTLS has taken over the record layer (then it's a plain send() too).
        if constexpr (Protocol == Protocol::TCP) {
            sent_bytes = ::send(m_socket, reinterpret_cast<const char *>(buffer),  // FIXME(clang-tidy): reinterpret_cast usage
                                static_cast<buffsize_t>(LENGTH), 0);
        } else if constexpr (Protocol == Protocol::TLS) {
            if (m_ktls_tx) {
                sent_bytes = ::send(m_socket, reinterpret_cast<const char *>(buffer),  // FIXME(clang-tidy): reinterpret_cast usage
                                    static_cast<buffsize_t>(LENGTH), 0);
            } else {
                ssl_call_result = SSL_write_ex(m_ssl, reinterpret_cast<const void *>(buffer),  // FIXME(clang-tidy): reinterpret_cast usage
                                               LENGTH, &sent_bytes);
            }
        } else if constexpr (Protocol == Protocol::QUIC) {
            ssl_call_result =
                SSL_write_ex(m_ssl, reinterpret_cast<const void *>(buffer), LENGTH, &sent_bytes);  // FIXME(clang-tidy): reinterpret_cast usage
        } else if constexpr (Protocol == Protocol::UDP) {
            if (addr != nullptr) {
                sent_bytes = ::sendto(m_socket, reinterpret_cast<const char *>(buffer),  // FIXME(clang-tidy): reinterpret_cast usage
                                      static_cast<buffsize_t>(LENGTH), 0,
                                      reinterpret_cast<const sockaddr *>(&addr->get_data()),  // FIXME(clang-tidy): reinterpret_cast usage
                                      addr->get_size());
            } else {
                sent_bytes = ::sendto(m_socket, reinterpret_cast<const char *>(buffer),  // FIXME(clang-tidy): reinterpret_cast usage
                                      static_cast<buffsize_t>(LENGTH), 0,
                                      static_cast<sockaddr *>(m_socket_address_info->ai_addr),
                                      m_socket_address_info->ai_addrlen);
            }
        }

        // kTLS bypasses SSL_write_ex entirely (it's a plain send() above), so its error handling
        // lives in the `else` below — this branch only covers the non-kTLS SSL_write_ex path.
        if (!m_ktls_tx) {
            if constexpr (Protocol == Protocol::TLS || Protocol == Protocol::QUIC) {
                if (ssl_call_result <= 0) {
                    return map_send_ssl_error(ssl_call_result);
                }
            }
        } else {
            // Plain send()/sendto() path (TCP, UDP, or kTLS-offloaded TLS) — same
            // would-block-vs-real-error split as everywhere else in this class.
            if (sent_bytes < 0) {
                const auto ERR = get_error_code();
                if (ERR == EWOULDBLOCK) {
                    core::logger::warning(
                        "SocketLib",
                        "Send on socket `{}` would have blocked, no buffer space available",
                        m_socket);
                    core::events::publish("socket.send.would_block", {{"fd", std::to_string(m_socket)}});
                    return std::make_pair(0, SocketStatus(VALUES::NON_BLOCKING_WOULD_HAVE_BLOCKED));
                }
                core::logger::warning("SocketLib", "Socket `{}` critical failure in send() syscall",
                                      m_socket);
                core::events::publish("socket.send.critical_failure",
                                      {{"fd", std::to_string(m_socket)}, {"error_code", std::to_string(ERR)}});
                return std::make_pair(0, SocketStatus(VALUES::ERRORED, ERR));
            }
        }

        core::logger::debug("SocketLib", "socket {} sent {} bytes", m_socket, sent_bytes);
        return std::make_pair(sent_bytes, SocketStatus(VALUES::VALID));
    }

    /**
     * @brief Async counterpart to sync_send() — routes through the leverager for TCP/kTLS-TLS
     * and UDP, or through a self-re-arming SSL_write_ex retry loop for QUIC (and non-kTLS TLS,
     * which is explicitly rejected, see the warning).
     * @param buffer bytes to send — caller-owned till `callback` fires.
     * @param length number of bytes in `buffer` to send.
     * @param callback invoked with the bytes sent and resulting status.
     * @param addr UDP-only: explicit destination address; null falls back to
     * `m_socket_address_info`.
     * @param iflags forwarded to the underlying leverager send()/sendmsg() call.
     * @warning TLS sockets without kTLS enabled hit `core::logger::fatal` (process-terminating)
     * instead of sending — `SSL_write_ex`'s return semantics don't map cleanly onto a single
     * async completion, so this backend only supports async send once kernel TLS offload is on.
     * Call set_reuse_address()-style kTLS setup (via a successful handshake) first, or stick to
     * sync_send() for non-kTLS TLS.
     * @warning Requires `m_leverager` — without one, this also routes to `core::logger::fatal`.
     */
    // bugprone-exception-escape: genuinely fixed, not NOLINT'd — the whole body below is wrapped
    // in a top-level try/catch so nothing can escape this noexcept function. It has to stay
    // noexcept: interfaces::io::AsyncSendable's requires-expression demands
    // `{ sock.async_send(...) } noexcept -> std::same_as<void>`, and
    // io/flow/sender/async.cppm has `static_assert(AsyncSendable<Socket<TCP>, SocketStatus>)`,
    // which would stop compiling if this lost noexcept. What can actually throw here:
    // std::make_shared (QUIC branch), converting the completion lambdas to the leverager's
    // `completion_callback`/`std::function` (the type-erasing conversion can allocate), and the
    // leverager's own send()/sendmsg() calls (they queue through Context::submit_async(), which
    // isn't noexcept). If any of that throws, `callback` may already be moved-from (empty) by the
    // time we reach the catch — `if (callback)` on a move_only_function is a safe, well-defined
    // check for that, so we only re-invoke it when it's still ours to call, and that
    // re-invocation is itself guarded so a throwing callback can't escape either.
    // readability-function-cognitive-complexity: genuinely fixed, not NOLINT'd — the three
    // TCP/TLS + UDP(addr) + UDP(no-addr) completion lambdas were byte-identical, so they're now
    // one shared complete_async_send() helper; the QUIC retry loop's SSL-error switch is now
    // handle_async_send_ssl_wait(). Both are private methods below, called instead of inlined.
    void async_send(const std::byte *buffer, const std::size_t LENGTH,
                    std::move_only_function<void(std::size_t, SocketStatus)> callback,
                    AddressInfo *addr = nullptr, std::uint8_t iflags = 0) noexcept {
        try {
            if (m_leverager) {
                if constexpr (Protocol == Protocol::TCP || Protocol == Protocol::TLS) {
                    // Non-kTLS TLS can't do async send — SSL_write_ex's return semantics don't map
                    // onto a single completion, so refuse instead of behaving weirdly.
                    if constexpr (Protocol == Protocol::TLS) {
                        if (!m_ktls_tx) {
                            core::events::publish("socket.async_send.tls_without_ktls_unsupported",
                                                  {{"fd", std::to_string(m_socket)}});
                            core::logger::fatal(
                                "SocketLib",
                                "Async send is not supported for TLS sockets without kTLS enabled due "
                                "to "
                                "the complexity of handling SSL_write's various return conditions in "
                                "an "
                                "async context. Please enable kTLS for this socket to use async send.");
                        }
                    }
                    m_leverager->get().send(
                        m_socket, buffer, static_cast<unsigned>(LENGTH), 0,
                        [this, callback = std::move(callback)](int sent_bytes, std::uint32_t) mutable {
                            complete_async_send(sent_bytes, callback);
                        },
                        iflags);
                } else if constexpr (Protocol == Protocol::UDP) {
                    // Explicit destination given vs falling back to this socket's own resolved
                    // endpoint — same sendmsg() plumbing either way, just a different msg_name;
                    // these two cases used to be fully duplicated blocks (down to the completion
                    // lambda), merged here into one shared iovec/msghdr/sendmsg call since only
                    // the two msg_name/msg_namelen assignments actually differed.
                    iovec iov{};
                    iov.iov_base = const_cast<std::byte *>(buffer);  // NOLINT(cppcoreguidelines-pro-type-const-cast) — iovec::iov_base is a plain (non-const) void* by POSIX definition; sendmsg() only reads it here, but the struct's field type forces the const_cast
                    iov.iov_len = LENGTH;

                    msghdr msg{};
                    if (addr != nullptr) {
                        msg.msg_name = reinterpret_cast<sockaddr *>(&addr->get_data());  // FIXME(clang-tidy): reinterpret_cast usage
                        msg.msg_namelen = addr->get_size();
                    } else {
                        msg.msg_name = static_cast<sockaddr *>(m_socket_address_info->ai_addr);
                        msg.msg_namelen = m_socket_address_info->ai_addrlen;
                    }
                    msg.msg_iov = &iov;
                    msg.msg_iovlen = 1;

                    m_leverager->get().sendmsg(
                        m_socket, &msg, 0,
                        [this, callback = std::move(callback)](int sent_bytes, std::uint32_t) mutable {
                            complete_async_send(sent_bytes, callback);
                        },
                        iflags);
                } else if constexpr (Protocol == Protocol::QUIC) {
                    auto attempt = std::make_shared<std::function<void(int)>>();
                    *attempt = [this, buffer, LENGTH, callback = std::move(callback), iflags,
                                attempt](int res, std::uint32_t) mutable {
                        attempt_async_quic_send(res, buffer, LENGTH, attempt, callback, iflags);
                    };

                    // Call to jumpstart the async send process
                    (*attempt)(0);
                } else {
                    core::events::publish("socket.async_send.unsupported_protocol");
                    core::logger::fatal("SocketLib", "Unsupported protocol for async send");
                }
            } else {
                core::events::publish("socket.leverager_not_set", {{"site", "async_send"}});
                core::logger::fatal("SocketLib",
                                    "m_leverager is not set so async funtion calls cannot be used");
            }
        } catch (...) {
            // The actual diagnostic-logging/callback-recovery logic lives in
            // report_async_io_exception() below, kept as a separate method (rather than inlined
            // here) specifically so its own try/catch nesting doesn't count against this
            // function's cognitive complexity too.
            report_async_io_exception("async_send", callback);
        }
    }


    /**
     * @brief Blocking receive into an output iterator, offset variant — advances `out` by
     * `start_offset` before writing, letting callers fill the tail of a larger buffer in
     * multiple calls. Dispatches to `::recv`/`SSL_read_ex`/`::recvfrom` per-protocol like
     * sync_send()'s mirror image.
     * @tparam Out an output iterator over `std::byte` (e.g. from a `std::vector<std::byte>` or
     * raw buffer).
     * @param out destination iterator — advanced by `start_offset` before any bytes are written.
     * @param length total logical buffer length; `length - start_offset` is the actual max bytes
     * read this call.
     * @param start_offset how far into the logical buffer this call's data should land.
     * @param addr UDP-only: out-param filled with the sender's address.
     * @warning `MAX_LENGTH` (`length - start_offset`) is computed as `std::size_t` (unsigned),
     * then checked with `if (MAX_LENGTH < 0)` — that comparison can never be true for an unsigned
     * type. If `start_offset > length`, the subtraction wraps around to a huge value instead of
     * going negative, and the intended bounds-check silently does nothing. Passing a bad offset
     * doesn't get caught here the way the code implies it does.
     * @return bytes received alongside a status — same VALID/NON_BLOCKING_WOULD_HAVE_BLOCKED/
     * CLEANLY_DISCONNECTED/ERRORED contract as sync_send()'s return.
     */
    // readability-function-cognitive-complexity: genuinely fixed, not NOLINT'd — the SSL-error
    // mapping switch is now map_receive_ssl_error(), a private method below shared with the other
    // sync_receive() overload (the two switches were byte-identical); logic and ordering are
    // unchanged.
    template <std::output_iterator<std::byte> Out>
    std::pair<std::size_t, SocketStatus> sync_receive(Out out, const std::size_t LENGTH,
                                                      const std::size_t START_OFFSET = 0,
                                                      AddressInfo *addr = nullptr) {
        // Advance the destination iterator past the part of the buffer we're not filling this
        // call, then figure out how much room is actually left for the read.
        std::advance(out, START_OFFSET);
        const auto MAX_LENGTH = LENGTH - START_OFFSET;
        if (MAX_LENGTH < 0) {
            core::logger::error("SocketLib",
                                "Start offset {} is greater than the total buffer length {}",
                                START_OFFSET, LENGTH);
            core::events::publish("socket.receive.invalid_offset",
                                  {{"start_offset", std::to_string(START_OFFSET)},
                                   {"length", std::to_string(LENGTH)}});
            return std::make_pair(0, SocketStatus(VALUES::ERRORED));
        }

        std::size_t received_bytes{0};
        int ssl_call_result{0};

        // Dispatch the actual read per-protocol, mirroring sync_send()'s dispatch shape.
        if constexpr (Protocol == Protocol::TCP) {
            received_bytes = ::recv(m_socket, reinterpret_cast<char *>(std::to_address(out)),  // FIXME(clang-tidy): reinterpret_cast usage
                                    static_cast<buffsize_t>(MAX_LENGTH), 0);
        } else if constexpr (Protocol == Protocol::TLS) {
            if (m_ktls_rx) {
                received_bytes = ::recv(m_socket, reinterpret_cast<char *>(std::to_address(out)),  // FIXME(clang-tidy): reinterpret_cast usage
                                        static_cast<buffsize_t>(MAX_LENGTH), 0);
            } else {
                ssl_call_result = SSL_read_ex(m_ssl, reinterpret_cast<void *>(std::to_address(out)),  // FIXME(clang-tidy): reinterpret_cast usage
                                              MAX_LENGTH, &received_bytes);
            }
        } else if constexpr (Protocol == Protocol::QUIC) {
            ssl_call_result = SSL_read_ex(m_ssl, reinterpret_cast<void *>(std::to_address(out)),  // FIXME(clang-tidy): reinterpret_cast usage
                                          MAX_LENGTH, &received_bytes);
        } else if constexpr (Protocol == Protocol::UDP) {
            m_socket_input_buffer_length = sizeof(m_socket_input_buffer);

            received_bytes = ::recvfrom(m_socket, reinterpret_cast<char *>(std::to_address(out)),  // FIXME(clang-tidy): reinterpret_cast usage
                                        static_cast<buffsize_t>(MAX_LENGTH), 0,
                                        reinterpret_cast<sockaddr *>(&m_socket_input_buffer),  // FIXME(clang-tidy): reinterpret_cast usage
                                        &m_socket_input_buffer_length);
            if (addr) {
                addr->set(m_socket_input_buffer, m_socket_input_buffer_length);
            }
        }

        // kTLS bypasses SSL_read_ex (it's a plain recv() above), so its error handling lives in
        // the `else` below — this branch only covers the non-kTLS SSL_read_ex path.
        if (!m_ktls_rx) {
            if constexpr (Protocol == Protocol::TLS || Protocol == Protocol::QUIC) {
                if (ssl_call_result <= 0) {
                    return map_receive_ssl_error(ssl_call_result);
                }
            }
        } else {
            // Plain recv()/recvfrom() path (TCP, UDP, or kTLS-offloaded TLS) — a 0-byte read
            // here genuinely means the peer closed, not "nothing arrived yet".
            if (received_bytes < 0) {
                const auto ERR = get_error_code();
                if (ERR == EWOULDBLOCK || ERR == EAGAIN) {
                    core::logger::warning(
                        "SocketLib", "Receive on socket `{}` would have blocked, no data available",
                        m_socket);
                    core::events::publish("socket.receive.would_block", {{"fd", std::to_string(m_socket)}});
                    return std::make_pair(0, SocketStatus(VALUES::NON_BLOCKING_WOULD_HAVE_BLOCKED));
                }
                core::logger::warning("SocketLib", "Socket `{}` critical failure in recv() syscall",
                                      m_socket);
                core::events::publish("socket.receive.critical_failure",
                                      {{"fd", std::to_string(m_socket)}, {"error_code", std::to_string(ERR)}});
                return std::make_pair(0, SocketStatus(VALUES::ERRORED, ERR));
            }

            core::logger::debug("SocketLib", "socket {} cleanly disconnected", m_socket);
            return std::make_pair(0, SocketStatus(VALUES::CLEANLY_DISCONNECTED));
        }

        core::logger::debug("SocketLib", "socket {} received {} bytes", m_socket, received_bytes);
        return std::make_pair(received_bytes, SocketStatus(VALUES::VALID));
    }

    /**
     * @brief Blocking receive overload, wait-mode variant — trades the offset param from the
     * other sync_receive() overload for a `WaitMode` toggle between "wait for the whole message"
     * (`MSG_WAITALL`) and "grab whatever's arrived" (temporarily flips non-blocking).
     * @tparam Out an output iterator over `std::byte`.
     * @param out destination iterator.
     * @param length max bytes to receive into `out`.
     * @param wait WAIT_FOR_WHOLE_MESSAGE blocks until `length` bytes arrive; AS_SOON_AS_ARRIVED
     * returns with whatever's available right away.
     * @param addr UDP-only: out-param filled with the sender's address.
     * @warning For AS_SOON_AS_ARRIVED on non-Windows, this flips the socket non-blocking via
     * `MSG_DONTWAIT` on the syscall itself (no lasting state change) — but on Windows it calls
     * set_non_blocking(true) then set_non_blocking(false) around the call, which *does* mutate
     * persistent socket state. If another thread touches this socket's blocking mode
     * concurrently on Windows, that's a real race — posix doesn't have this issue since it's a
     * per-call flag, not global socket state.
     * @return bytes received alongside a status — same contract as the other sync_receive()
     * overload.
     */
    // readability-function-cognitive-complexity: genuinely fixed, not NOLINT'd — the wait-mode
    // flag setup (MSG_WAITALL vs MSG_DONTWAIT/set_non_blocking) was the same shape 3 times over
    // (TCP, TLS-with-kTLS, UDP), now shared via prepare_receive_wait_flags(); the SSL-error
    // mapping switch is now map_receive_ssl_error(), the same private method the other
    // sync_receive() overload uses (the two switches were byte-identical). Logic and ordering are
    // unchanged; the post-call Windows-only `set_non_blocking(false)` restore stays inline per
    // branch since UDP's original code never had that restore step (an existing TCP/TLS-vs-UDP
    // asymmetry, not something this pass should silently "fix").
    template <std::output_iterator<std::byte> Out>
    std::pair<std::size_t, SocketStatus> sync_receive(Out out, const std::size_t LENGTH,
                                                      WaitMode wait = WaitMode::AS_SOON_AS_ARRIVED,
                                                      AddressInfo *addr = nullptr) {
        std::size_t received_bytes{0};
        int ssl_call_result{0};

        // Plain TCP: MSG_WAITALL blocks until `length` bytes show up; otherwise grab whatever's
        // there right now. Non-Windows does that per-call via MSG_DONTWAIT; Windows has no
        // per-call non-blocking flag on recv() so it has to flip the socket's mode around the
        // call instead (see the class-level @warning about that being a real race).
        if constexpr (Protocol == Protocol::TCP) {
            int flags = prepare_receive_wait_flags(wait, MSG_WAITALL);

            received_bytes = ::recv(m_socket, reinterpret_cast<char *>(std::to_address(out)),  // FIXME(clang-tidy): reinterpret_cast usage
                                    static_cast<buffsize_t>(LENGTH), flags);

            if constexpr (IS_WINDOWS) {
                set_non_blocking(false);
            }
        } else if constexpr (Protocol == Protocol::TLS) {
            // kTLS active: same MSG_WAITALL/MSG_DONTWAIT dance as plain TCP above, since the
            // kernel's doing the record layer.
            if (m_ktls_rx) {
                int flags = prepare_receive_wait_flags(wait, MSG_WAITALL);

                received_bytes = ::recv(m_socket, reinterpret_cast<char *>(std::to_address(out)),  // FIXME(clang-tidy): reinterpret_cast usage
                                        static_cast<buffsize_t>(LENGTH), flags);

                if constexpr (IS_WINDOWS) {
                    set_non_blocking(false);
                }
            } else {
                // No kTLS: toggle SSL's partial-write mode around a non-waiting read so
                // SSL_read_ex can hand back less than `length` instead of insisting on a full
                // fill, then restore the mode afterward.
                ssl_call_result = read_ssl_with_partial_mode_toggle(
                    wait, reinterpret_cast<void *>(std::to_address(out)), LENGTH, received_bytes);  // FIXME(clang-tidy): reinterpret_cast usage
            }

        } else if constexpr (Protocol == Protocol::QUIC) {
            // Same partial-write-mode toggle as non-kTLS TLS above.
            ssl_call_result = read_ssl_with_partial_mode_toggle(
                wait, reinterpret_cast<void *>(std::to_address(out)), LENGTH, received_bytes);  // FIXME(clang-tidy): reinterpret_cast usage
        } else if constexpr (Protocol == Protocol::UDP) {
            m_socket_input_buffer_length = sizeof(m_socket_input_buffer);

            // UDP has no MSG_WAITALL equivalent — WAIT_FOR_WHOLE_MESSAGE here just means
            // "block until a datagram arrives" (the default), AS_SOON_AS_ARRIVED goes
            // non-blocking. Passing 0 as the "waiting" flag means prepare_receive_wait_flags()
            // never sets MSG_WAITALL, matching the original UDP-only behavior exactly.
            int flags = prepare_receive_wait_flags(wait, 0);

            received_bytes = ::recvfrom(m_socket, reinterpret_cast<char *>(std::to_address(out)),  // FIXME(clang-tidy): reinterpret_cast usage
                                        static_cast<buffsize_t>(LENGTH), flags,
                                        reinterpret_cast<sockaddr *>(&m_socket_input_buffer),  // FIXME(clang-tidy): reinterpret_cast usage
                                        &m_socket_input_buffer_length);
            if (addr) {
                addr->set(m_socket_input_buffer, m_socket_input_buffer_length);
            }
        }

        // Same non-kTLS-SSL-error-vs-plain-syscall-error split as the offset overload above.
        if (!m_ktls_rx) {
            if constexpr (Protocol == Protocol::TLS || Protocol == Protocol::QUIC) {
                if (ssl_call_result <= 0) {
                    return map_receive_ssl_error(ssl_call_result);
                }
            }
        } else {
            if (received_bytes < 0) {
                const auto ERR = get_error_code();
                if (ERR == EWOULDBLOCK || ERR == EAGAIN) {
                    core::logger::warning(
                        "SocketLib", "Receive on socket `{}` would have blocked, no data available",
                        m_socket);
                    core::events::publish("socket.receive.would_block", {{"fd", std::to_string(m_socket)}});
                    return std::make_pair(0, SocketStatus(VALUES::NON_BLOCKING_WOULD_HAVE_BLOCKED));
                }
                core::logger::warning("SocketLib", "Socket `{}` critical failure in recv() syscall",
                                      m_socket);
                core::events::publish("socket.receive.critical_failure",
                                      {{"fd", std::to_string(m_socket)}, {"error_code", std::to_string(ERR)}});
                return std::make_pair(0, SocketStatus(VALUES::ERRORED, ERR));
            }
            core::logger::debug("SocketLib", "socket {} cleanly disconnected", m_socket);
            return std::make_pair(0, SocketStatus(VALUES::CLEANLY_DISCONNECTED));
        }

        core::logger::debug("SocketLib", "socket {} received {} bytes", m_socket, received_bytes);
        return std::make_pair(received_bytes, SocketStatus(VALUES::VALID));
    }

    /**
     * @brief Async counterpart to sync_receive() — routes through the leverager for TCP/kTLS-TLS
     * and UDP, or a self-re-arming SSL_read_ex retry loop for QUIC (non-kTLS TLS is rejected, see
     * the warning, mirroring async_send()).
     * @tparam Out an output iterator over `std::byte`.
     * @param out destination iterator — caller-owned till `callback` fires.
     * @param length max bytes to receive into `out`.
     * @param callback invoked with the bytes received and resulting status.
     * @param iflags forwarded to the underlying leverager recv()/recvmsg() call.
     * @warning TLS sockets without kTLS enabled hit `core::logger::fatal` instead of receiving —
     * same reasoning as async_send(). Requires `m_leverager` — without one, this also routes to
     * `core::logger::fatal`.
     */
    // bugprone-exception-escape: genuinely fixed, not NOLINT'd — same reasoning and pattern as
    // async_send() above: the whole body is wrapped in a top-level try/catch, funneling into the
    // shared report_async_io_exception() helper on any exception, so nothing can escape. Stays
    // noexcept because interfaces::io::AsyncReceivable's requires-expression demands
    // `{ sock.async_receive(...) } noexcept -> std::same_as<void>`.
    // readability-function-cognitive-complexity: genuinely fixed, not NOLINT'd — the TCP/TLS and
    // UDP completion lambdas were byte-identical, so they're now one shared
    // complete_async_receive() helper; the QUIC retry loop's attempt lambda and its SSL-error
    // switch are now attempt_async_quic_receive() and handle_async_receive_ssl_wait(), mirroring
    // async_send()'s decomposition.
    template <std::output_iterator<std::byte> Out>
    void async_receive(Out out, const std::size_t LENGTH,
                       std::move_only_function<void(std::size_t, SocketStatus)> callback,
                       std::uint8_t iflags = 0) noexcept {
        try {
            if (m_leverager) {
                if constexpr (Protocol == Protocol::TCP || Protocol == Protocol::TLS) {
                    // Same non-kTLS-TLS refusal as async_send() — no clean way to map SSL_read_ex
                    // onto a single async completion without kernel offload.
                    if constexpr (Protocol == Protocol::TLS) {
                        if (!m_ktls_rx) {
                            core::events::publish("socket.async_receive.tls_without_ktls_unsupported",
                                                  {{"fd", std::to_string(m_socket)}});
                            core::logger::fatal("SocketLib", "Async receive is not supported for TLS "
                                                             "sockets without kTLS enabled due to "
                                                             "the complexity of handling SSL_write's "
                                                             "various return conditions in an "
                                                             "async context. Please enable kTLS for "
                                                             "this socket to use async receive.");
                        }
                    }
                    m_leverager->get().recv(
                        m_socket, reinterpret_cast<char *>(std::to_address(out)),  // FIXME(clang-tidy): reinterpret_cast usage
                        static_cast<unsigned>(LENGTH), 0,
                        [this, callback = std::move(callback)](int received_bytes,
                                                               std::uint32_t) mutable {
                            complete_async_receive(received_bytes, callback);
                        },
                        iflags);
                } else if constexpr (Protocol == Protocol::UDP) {
                    iovec iov{};
                    iov.iov_base = reinterpret_cast<char *>(std::to_address(out));  // FIXME(clang-tidy): reinterpret_cast usage
                    iov.iov_len = LENGTH;

                    msghdr msg{};
                    msg.msg_iov = &iov;
                    msg.msg_iovlen = 1;
                    msg.msg_name = &m_socket_input_buffer;
                    msg.msg_namelen = sizeof(m_socket_input_buffer);

                    m_leverager->get().recvmsg(
                        m_socket, &msg, 0,
                        [this, callback = std::move(callback)](int received_bytes,
                                                               std::uint32_t) mutable {
                            complete_async_receive(received_bytes, callback);
                        },
                        iflags);
                } else if constexpr (Protocol == Protocol::QUIC) {
                    auto *data_ptr = reinterpret_cast<char *>(std::to_address(out));  // FIXME(clang-tidy): reinterpret_cast usage
                    auto attempt = std::make_shared<std::function<void(int)>>();
                    *attempt = [this, data_ptr, LENGTH, callback = std::move(callback), iflags,
                                attempt](int res, std::uint32_t) mutable {
                        attempt_async_quic_receive(res, data_ptr, LENGTH, attempt, callback, iflags);
                    };

                    // Call to jumpstart the async receive process
                    (*attempt)(0);
                } else {
                    core::events::publish("socket.async_receive.unsupported_protocol");
                    core::logger::fatal("SocketLib", "Unsupported protocol for async receive");
                }
            } else {
                core::events::publish("socket.leverager_not_set", {{"site", "async_receive"}});
                core::logger::fatal("SocketLib",
                                    "m_leverager is not set so async funtion calls cannot be used");
            }
        } catch (...) {
            report_async_io_exception("async_receive", callback);
        }
    }

    /**
     * @brief Tears down SSL/BIO/context state (TLS/QUIC) and closes the native socket — this is
     * the real cleanup logic the dtor and move-assign-leak warning both reference.
     * @note Only fires if `m_socket != INVALID_SOCKET` — calling this on an already-closed or
     * default-constructed Socket is a safe no-op, no double-close risk.
     * @note Minor logging quirk: `m_socket` gets reset to `INVALID_SOCKET` *before* the trailing
     * `core::logger::debug("socket {} closed", m_socket)` call, so the debug line always logs
     * `-1`/`INVALID_SOCKET` rather than the fd that actually got closed. Cosmetic, not a
     * functional bug.
     */
    void sync_close() {
        // No-op on an already-closed/default-constructed Socket — nothing to tear down.
        if (m_socket != INVALID_SOCKET) {
            // TLS/QUIC: mark both directions shut, run the SSL shutdown handshake, then free
            // the SSL object, context, and (QUIC-only) the shared BIO.
            if constexpr (Protocol == Protocol::TLS || Protocol == Protocol::QUIC) {
                if (m_ssl != nullptr) {
                    SSL_set_shutdown(m_ssl, SSL_RECEIVED_SHUTDOWN | SSL_SENT_SHUTDOWN);
                    SSL_shutdown(m_ssl);
                    SSL_free(m_ssl);
                }

                if (m_ssl_ctx != nullptr) {
                    SSL_CTX_free(m_ssl_ctx);
                }

                if constexpr (Protocol == Protocol::QUIC) {
                    if (m_bio != nullptr) {
                        BIO_free(m_bio);
                    }
                }
            }
            // Only close the actual fd if we own it outright — non-root QUIC connections share
            // their fd with the root socket, closing it here would break every other connection.
            if constexpr (Protocol != Protocol::QUIC || RootSocket) {
                if (closesocket(m_socket) == SOCKET_ERROR) {
                    core::logger::error("SocketLib", "Socket `{}` failed to close", m_socket);
                    core::events::publish("socket.close_failed", {{"fd", std::to_string(m_socket)}});
                }
            }

            m_socket = INVALID_SOCKET;

            core::logger::debug("SocketLib", "socket {} closed", m_socket);
        }
    }

    /**
     * @brief Async counterpart to sync_close() — tears down SSL/BIO/context state the same way,
     * then queues the actual fd close through the leverager instead of blocking on it.
     * @param callback invoked with `true`/`false` once the close completes.
     * @param iflags forwarded to the leverager's close() call.
     * @warning For non-root QUIC connections (shared fd, skipped per the dtor's note), `callback`
     * never fires at all — the whole leverager-close block, callback included, sits inside the
     * `Protocol != Protocol::QUIC || RootSocket` `if constexpr` guard. A caller awaiting the
     * callback on a non-root QUIC socket will hang forever. Same for a Socket whose `m_socket` is
     * already `INVALID_SOCKET` — the outer `if` skips everything, callback included.
     */
    // bugprone-exception-escape: not `noexcept` (see below) — this method isn't a virtual
    // override, and the only concept that even names async_close() is interfaces::io::AsyncClose,
    // which requires a zero-argument `sock.async_close()` call; this overload takes a mandatory
    // `callback` with no default, so it can never satisfy that concept regardless of the
    // noexcept qualifier (confirmed no static_assert anywhere in the tree checks AsyncClose
    // against Socket, and the two call sites that invoke `.async_close()` with no arguments —
    // Sender/Receiver's on_released() — are templates never instantiated with Socket as the
    // Worker type). So nothing actually depends on this staying noexcept; dropping it is the
    // genuine fix, since the leverager's close() call and the user-supplied `callback` can both
    // legitimately throw.
    void async_close(std::function<void(bool)> callback, std::uint8_t iflags = 0) {
        if (m_socket != INVALID_SOCKET) {
            // Same SSL/BIO teardown as sync_close() — this part's still synchronous, only the
            // actual fd close below gets queued through the leverager.
            if constexpr (Protocol == Protocol::TLS || Protocol == Protocol::QUIC) {
                if (m_ssl != nullptr) {
                    SSL_set_shutdown(m_ssl, SSL_RECEIVED_SHUTDOWN | SSL_SENT_SHUTDOWN);
                    SSL_shutdown(m_ssl);
                    SSL_free(m_ssl);
                }

                if (m_ssl_ctx != nullptr) {
                    SSL_CTX_free(m_ssl_ctx);
                }

                if constexpr (Protocol == Protocol::QUIC) {
                    if (m_bio != nullptr) {
                        BIO_free(m_bio);
                    }
                }
            }

            // Same fd-ownership guard as sync_close() — queue the actual close only if this
            // socket owns the fd outright.
            if constexpr (Protocol != Protocol::QUIC || RootSocket) {
                m_leverager->get().close(
                    m_socket,
                    [this, callback = std::move(callback)](int res, std::uint32_t) mutable {
                        if (res < 0) {
                            core::logger::warning("SocketLib", "Socket `{}` failed to close",
                                                  m_socket);
                            core::events::publish("socket.close_failed", {{"fd", std::to_string(m_socket)}});
                            callback(false);
                            return;
                        }
                        core::logger::debug("SocketLib", "socket {} closed", m_socket);
                        m_socket = INVALID_SOCKET;
                        callback(true);
                    },
                    iflags);
            }
        }
    }

    /**
     * @brief Issues a synchronous `SHUT_RDWR` on the socket — compile-time skipped for non-root
     * QUIC connections (shared fd, see the dtor's note).
     * @warning Actual bug, not a footgun: the `core::logger::fatal("Shutdown is not supported for
     * QUIC or non-root sockets")` call at the bottom sits **outside** the `if constexpr` guard, so
     * it runs unconditionally on every single call — including a perfectly normal TCP/UDP/TLS
     * shutdown that just succeeded two lines above. Every call to this method terminates the
     * process (per this codebase's `logger::fatal` convention) after doing its real work, not
     * just the QUIC-non-root case the message claims. Looks like the fatal call was meant to live
     * inside an `else` branch of the `if constexpr` and got left outside it by mistake.
     */
    void shutdown() {
        // Root-owned fd only (mirrors sync_close()'s guard) — issue the actual SHUT_RDWR.
        if constexpr (Protocol != Protocol::QUIC || RootSocket) {
            if (m_socket != INVALID_SOCKET) {
                if (::shutdown(m_socket, SHUT_RDWR) == SOCKET_ERROR) {
                    core::logger::error("SocketLib", "Socket `{}` failed to shutdown", m_socket);
                    core::events::publish("socket.shutdown_failed", {{"fd", std::to_string(m_socket)}});
                }

                core::logger::debug("SocketLib", "socket {} shutdown", m_socket);
            }
        }

        // See the @warning above — this fatal() sits outside the if constexpr, so it fires on
        // every call, including ones that just shut down cleanly.
        core::events::publish("socket.shutdown_unconditional_fatal", {{"fd", std::to_string(m_socket)}});
        core::logger::fatal("SocketLib", "Shutdown is not supported for QUIC or non-root sockets");
    }

    /**
     * @brief Async counterpart to shutdown() — queues the shutdown through the leverager instead
     * of blocking.
     * @warning Same unconditional-`fatal`-at-the-bottom bug as shutdown() — see its warning.
     * Every call here also terminates the process after queuing (or skipping) the shutdown op.
     * @note The `if constexpr` guard here is `Protocol != Protocol::QUIC || !RootSocket` — the
     * inverse condition from shutdown()'s `Protocol != Protocol::QUIC || RootSocket`. That means
     * this one runs its body for QUIC *non-root* sockets and skips QUIC *root* sockets, backwards
     * from shutdown()'s root/non-root split. Worth confirming that asymmetry is intentional before
     * relying on it.
     * @param callback invoked with `true`/`false` once the shutdown completes (only reachable if
     * the process survives the trailing fatal() — see the warning).
     * @param iflags forwarded to the leverager's shutdown() call.
     */
    void async_shutdown(std::function<void(bool)> callback, std::uint8_t iflags = 0) {
        // Note the inverted guard vs shutdown() — see the @note above on that asymmetry.
        if constexpr (Protocol != Protocol::QUIC || !RootSocket) {
            if (m_socket != INVALID_SOCKET) {
                m_leverager->get().shutdown(
                    m_socket, SHUT_RDWR,
                    [this, callback = std::move(callback)](int res, std::uint32_t) mutable {
                        if (res < 0) {
                            core::logger::warning("SocketLib", "Socket `{}` failed to shutdown",
                                                  m_socket);
                            core::events::publish("socket.shutdown_failed", {{"fd", std::to_string(m_socket)}});
                            callback(false);
                            return;
                        }
                        core::logger::debug("SocketLib", "socket {} shutdown", m_socket);
                        callback(true);
                    },
                    iflags);
            }
        }

        // Same unconditional-fatal quirk as shutdown() — see its @warning.
        core::events::publish("socket.shutdown_unconditional_fatal", {{"fd", std::to_string(m_socket)}});
        core::logger::fatal("SocketLib", "Shutdown is not supported for QUIC or non-root sockets");
    }

    /**
     * @brief Grabs the endpoint this socket was constructed/connected against.
     * @return a const reference to `m_endpoint`.
     */
    [[nodiscard]] const Endpoint &get_endpoint() const noexcept { return m_endpoint; }
    /**
     * @brief Mutable overload of get_endpoint().
     * @return a mutable reference to `m_endpoint`.
     */
    Endpoint &get_endpoint() noexcept { return m_endpoint; }

    /**
     * @brief Grabs the *peer* endpoint a datagram actually arrived from — different from
     * get_endpoint() for UDP, where the socket's own configured endpoint and the last packet's
     * sender can be different addresses.
     * @warning Only TCP and UDP are handled. For TLS/QUIC there's no `if constexpr` branch and no
     * trailing `return`, so this falls off the end of a non-void function — undefined behavior,
     * same class of bug as SocketStatus get_status() above. Don't call this on a TLS/QUIC socket.
     * @return get_endpoint() for TCP; the sender address decoded from `m_socket_input_buffer`
     * (populated by the last sync_receive()/async_receive() UDP call) for UDP; undefined
     * otherwise.
     */
    [[nodiscard]] Endpoint get_recived_endpoint() const {
        // TCP's peer never changes mid-connection, so the socket's own endpoint is the answer.
        if constexpr (Protocol == Protocol::TCP) {
            return get_endpoint();
        } else if constexpr (Protocol == Protocol::UDP) {
            // UDP: decode whoever the *last* datagram actually came from, which can differ
            // packet to packet.
            const auto *addr = reinterpret_cast<const SOCKADDR *>(&m_socket_input_buffer);  // FIXME(clang-tidy): reinterpret_cast usage
            return Endpoint{addr};
        }
    }

    /**
     * @brief Queries how many bytes are sitting in the socket's receive buffer right now
     * (`FIONREAD`/`ioctlsocket`).
     * @return the pending byte count, or 0 if the ioctl reported 0 (or, per the `> 0` guard, a
     * negative value — which shouldn't happen from `FIONREAD` but isn't distinguished from
     * "genuinely zero pending" if it did).
     */
    [[nodiscard]] std::size_t get_pending_bytes() const {
        ioctl_setting pending_bytes = 0;
        if (ioctlsocket(m_socket, FIONREAD, &pending_bytes) < 0) {
            core::logger::error("SocketLib", "Failed to get pending bytes");
            core::events::publish("socket.get_pending_bytes_failed", {{"fd", std::to_string(m_socket)}});
        }

        core::logger::debug("SocketLib", "socket {} pending {} bytes", m_socket, pending_bytes);

        // Only trust a strictly-positive count — collapse 0 and any unexpected negative value
        // down to a plain 0 rather than casting a negative into a huge unsigned number.
        if (pending_bytes > 0) {
            return static_cast<std::size_t>(pending_bytes);
        }
        return 0;
    }

    /**
     * @brief Rebuilds `m_alpn_wire_format` from scratch (RFC 7301 length-prefixed wire format)
     * out of a full protocol list — clears whatever was there before.
     * @param protocols the ALPN protocol names to advertise, in preference order.
     * @warning Unlike add_alpn_proto() (which `return`s early on an oversized name),
     * this one only logs an error for a >255-byte protocol name and then keeps going anyway —
     * it still pushes `static_cast<unsigned char>(proto.length())` (silently truncated/wrapped
     * mod 256) as the length prefix and appends the *full* (untruncated) proto bytes after it.
     * That desyncs the length prefix from the actual byte count, producing a corrupt ALPN wire
     * entry that'll confuse whatever parses it downstream. Real inconsistency between the two
     * ALPN-setting methods worth fixing.
     */
    void set_alpn_protos(const std::vector<std::string> &protocols) {
        // Full rebuild — wipe whatever ALPN wire format was there before.
        m_alpn_wire_format.clear();
        for (const auto &proto : protocols) {
            // See the @warning above — an oversized name only logs here, it doesn't skip the
            // entry, so the length prefix below can desync from the real byte count.
            if (proto.length() > 255) {
                core::logger::error("SocketLib", "ALPN protocol name invalid or too long");
                core::events::publish("socket.alpn.name_invalid", {{"site", "set_alpn_protos"}});
            }

            core::logger::debug("SocketLib", "socket {} ALPN add {}", m_socket, proto);
            m_alpn_wire_format.push_back(static_cast<unsigned char>(proto.length()));
            m_alpn_wire_format.insert(m_alpn_wire_format.end(), proto.begin(), proto.end());
        }
    }

    /**
     * @brief Appends one more ALPN protocol name onto the existing `m_alpn_wire_format`, without
     * touching what's already there — the incremental counterpart to set_alpn_protos().
     * @param ALPN the protocol name to add.
     * @note Correctly bails out (`return`) on an empty or >255-byte name, unlike
     * set_alpn_protos()'s loop — see that method's warning for the inconsistency.
     */
    void add_alpn_proto(const std::string_view ALPN) {
        // Unlike set_alpn_protos()'s loop, this one actually bails on a bad name instead of
        // pushing a desynced entry.
        if (ALPN.empty() || ALPN.length() > 255) {
            core::logger::error("SocketLib", "ALPN protocol name invalid or too long");
            core::events::publish("socket.alpn.name_invalid", {{"site", "add_alpn_proto"}});
            return;
        }

        core::logger::debug("SocketLib", "socket {} ALPN add {}", m_socket, ALPN);
        m_alpn_wire_format.push_back(static_cast<unsigned char>(ALPN.length()));
        m_alpn_wire_format.insert(m_alpn_wire_format.end(), ALPN.begin(), ALPN.end());
    }

    /**
     * @brief Sets whether a client-side TLS handshake (`setup_tls()`) requires and verifies the
     * peer's certificate against the default trust store. Must be called before `sync_connect()`
     * /`async_connect()` — `setup_tls()` reads this flag once, right when it builds `m_ssl_ctx`.
     * @param verify `false` to set `SSL_VERIFY_NONE` instead of `SSL_VERIFY_PEER` — needed for
     * connecting to a self-signed/dev cert that isn't in the system trust store; `true` (the
     * default) keeps normal peer verification.
     */
    void set_verify_peer(bool verify) noexcept { m_verify_peer = verify; }

    /**
     * @brief Grabs the compile-time protocol this Socket instantiation is bound to.
     * @return the `Protocol` template parameter's value.
     */
    static enum Protocol get_protocol() noexcept { return Protocol; }
    /**
     * @brief Grabs the raw native socket handle, mutably.
     * @warning Handing out a mutable reference to the owned handle means a caller can reassign
     * `m_socket` out from under this Socket (e.g. leaking the original fd or aliasing a foreign
     * one) — there's no encapsulation protecting ownership here.
     * @return a mutable reference to `m_socket`.
     */
    SOCKET &get_fd() noexcept { return m_socket; }
    /**
     * @brief Const overload of get_fd().
     * @return a const reference to `m_socket`.
     */
    [[nodiscard]] const SOCKET &get_fd() const noexcept { return m_socket; }

    /**
     * @brief Identity comparison purely on the native fd value.
     * @param other the socket to compare against.
     * @return `true` if both sockets wrap the same native fd.
     */
    constexpr bool operator==(const Socket &other) const noexcept {
        return m_socket == other.m_socket;
    }
    /**
     * @brief Checks whether this Socket wraps a real (non-`INVALID_SOCKET`) handle.
     * @return `true` if the underlying fd isn't `INVALID_SOCKET`.
     */
    [[nodiscard]] bool is_valid() const noexcept { return m_socket != INVALID_SOCKET; }

    /**
     * @brief Bool-conversion shorthand for is_valid() — bet, lets call sites write
     * `if (socket) { ... }` directly.
     * @return `true` if the socket is valid.
     */
    operator bool() const noexcept { return is_valid(); }

  private:
    /**
     * @brief Body of async_connect()'s "connect landed" branch — layers TLS/QUIC setup on top of
     * a successful raw connect same as sync_connect(), then reports the final status through
     * `callback`. Pulled out to its own method purely to bring async_connect()'s cognitive
     * complexity down; logic and ordering are unchanged from the original inline code.
     * @param addr the resolved address the successful connect landed against.
     * @param callback the caller's async_connect() callback.
     */
    void complete_async_connect(addrinfo *addr,
                                std::move_only_function<void(SocketStatus)> &callback) {
        m_socket_address_info = addr;

        if constexpr (Protocol == Protocol::TLS) {
            if (!setup_tls()) {
                callback(SocketStatus(VALUES::ERRORED));
                return;
            }
        } else if constexpr (Protocol == Protocol::QUIC) {
            add_quic_bio();
            if (!setup_quic()) {
                callback(SocketStatus(VALUES::ERRORED));
                return;
            }
        }

        core::logger::debug("SocketLib", "socket {} connected {}:{}", m_socket,
                            m_endpoint.get_address(), m_endpoint.get_port());
        callback(SocketStatus(VALUES::VALID));
    }

    /**
     * @brief Body of async_connect()'s "ran out of candidates" branch — classifies the last
     * attempt's error and reports a final status through `callback`. Pulled out to its own method
     * purely to bring async_connect()'s cognitive complexity down; logic and ordering are
     * unchanged from the original inline code.
     * @param res the last re-arm attempt's completion result (a negative errno).
     * @param callback the caller's async_connect() callback.
     */
    void report_final_async_connect_error(int res,
                                          std::move_only_function<void(SocketStatus)> &callback) const {
        core::logger::warning("SocketLib", "socket {} no more addresses to try", m_socket);
        core::events::publish("socket.connect.no_more_addresses", {{"fd", std::to_string(m_socket)}});
        if (res == -ETIMEDOUT) {
            core::logger::warning("SocketLib", "socket {} connect timed out", m_socket);
            core::events::publish("socket.connect.timed_out", {{"fd", std::to_string(m_socket)}});
            callback(SocketStatus(VALUES::TIMED_OUT));
        } else if (res == -EWOULDBLOCK || res == -EAGAIN || res == -EINPROGRESS) {
            core::logger::warning("SocketLib", "socket {} connect would block", m_socket);
            core::events::publish("socket.connect.would_block", {{"fd", std::to_string(m_socket)}});
            callback(SocketStatus(VALUES::NON_BLOCKING_WOULD_HAVE_BLOCKED));
        } else {
            core::logger::warning("SocketLib", "socket {} connect failed err={}", m_socket, res);
            core::events::publish("socket.connect.failed",
                                  {{"fd", std::to_string(m_socket)}, {"error_code", std::to_string(res)}});
            callback(SocketStatus(VALUES::ERRORED));
        }
    }

    /**
     * @brief Body of async_connect()'s "still got candidates left" branch — opens a fresh socket
     * for the next address family ahead of re-arming the async connect against it. Pulled out to
     * its own method purely to bring async_connect()'s cognitive complexity down; logic and
     * ordering are unchanged from the original inline code.
     * @param addr the next resolved address to open a socket for.
     * @return `true` on success, `false` if socket creation failed (already logged).
     */
    [[nodiscard]] bool reopen_socket_for_connect_retry(addrinfo *addr) {
        m_socket = ::socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
        if (m_socket == INVALID_SOCKET) {
            core::logger::warning("SocketLib", "socket create failed for connect retry");
            core::events::publish("socket.connect_retry.create_failed");
            return false;
        }

        set_non_blocking(true);
        return true;
    }

    /**
     * @brief Classifies the SSL error from sync_send()'s non-kTLS `SSL_write_ex` path into the
     * matching SocketStatus — want-read/want-write both mean "come back later", zero-return means
     * the peer hung up cleanly. Pulled out to its own method purely to bring sync_send()'s
     * cognitive complexity down; logic and ordering are unchanged from the original inline switch.
     * @param ssl_call_result the value to feed `SSL_get_error` (`SSL_write_ex`'s own return value).
     * @return `(0, status)` for every case — sync_send() never has partial bytes to report on a
     * non-success SSL result.
     */
    [[nodiscard]] std::pair<std::size_t, SocketStatus> map_send_ssl_error(int ssl_call_result) const
        requires(Protocol == Protocol::TLS || Protocol == Protocol::QUIC)
    {
        const int SSL_ERR = SSL_get_error(m_ssl, ssl_call_result);

        switch (SSL_ERR) {
        case SSL_ERROR_WANT_READ: {
            core::logger::warning(
                "SocketLib", "Read on socket `{}` would have blocked (SSL - Pending: {}, Want: {})",
                m_socket, SSL_pending(m_ssl), SSL_want(m_ssl));
            core::events::publish("socket.tls.send.want_read", {{"fd", std::to_string(m_socket)}});
            return std::make_pair(0, SocketStatus(VALUES::NON_BLOCKING_WOULD_HAVE_BLOCKED));
        }
        case SSL_ERROR_WANT_WRITE: {
            core::logger::warning(
                "SocketLib", "Write on socket `{}` would have blocked (SSL - Pending: {}, Want: {})",
                m_socket, SSL_pending(m_ssl), SSL_want(m_ssl));
            core::events::publish("socket.tls.send.want_write", {{"fd", std::to_string(m_socket)}});
            return std::make_pair(0, SocketStatus(VALUES::NON_BLOCKING_WOULD_HAVE_BLOCKED));
        }
        case SSL_ERROR_ZERO_RETURN: {
            core::logger::debug("SocketLib", "socket {} cleanly disconnected", m_socket);
            return std::make_pair(0, SocketStatus(VALUES::CLEANLY_DISCONNECTED));
        }
        case SSL_ERROR_SYSCALL: {
            if (get_error_code() == EWOULDBLOCK) {
                core::logger::warning("SocketLib",
                                      "Sending on socket `{}` would have blocked, no data available "
                                      "(SSL - Pending: {}, Want: {})",
                                      m_socket, SSL_pending(m_ssl), SSL_want(m_ssl));
                core::events::publish("socket.tls.send.would_block", {{"fd", std::to_string(m_socket)}});
                return std::make_pair(0, SocketStatus(VALUES::NON_BLOCKING_WOULD_HAVE_BLOCKED));
            }

            const auto ERR = get_error_code();
            core::logger::warning(
                "SocketLib", "Socket `{}` critical failure in SSL_write_ex() with system error code `{}`",
                m_socket, ERR);
            core::events::publish("socket.tls.send.syscall_failure",
                                  {{"fd", std::to_string(m_socket)}, {"error_code", std::to_string(ERR)}});
            return std::make_pair(0, SocketStatus(VALUES::ERRORED, ERR));
        }
        default: {
            core::logger::warning(
                "SocketLib", "Socket `{}` critical failure in SSL_write_ex() with SSL error code `{}`",
                m_socket, SSL_ERR);
            core::events::publish("socket.tls.send.ssl_failure",
                                  {{"fd", std::to_string(m_socket)}, {"ssl_error", std::to_string(SSL_ERR)}});
            return std::make_pair(0, SocketStatus(VALUES::ERRORED, SSL_ERR));
        }
        }
    }

    /**
     * @brief Classifies the SSL error from both sync_receive() overloads' non-kTLS `SSL_read_ex`
     * path into the matching SocketStatus — the two overloads had byte-identical switches, so
     * this one method now serves both, pulled out purely to bring their cognitive complexity
     * down. Same want-read/want-write/zero-return/syscall/default mapping as map_send_ssl_error(),
     * mirrored for the receive direction.
     * @param ssl_call_result the value to feed `SSL_get_error` (`SSL_read_ex`'s own return value).
     * @return `(0, status)` for every case — neither sync_receive() overload has partial bytes to
     * report on a non-success SSL result.
     */
    [[nodiscard]] std::pair<std::size_t, SocketStatus> map_receive_ssl_error(int ssl_call_result) const
        requires(Protocol == Protocol::TLS || Protocol == Protocol::QUIC)
    {
        const int SSL_ERR = SSL_get_error(m_ssl, ssl_call_result);

        switch (SSL_ERR) {
        case SSL_ERROR_WANT_READ: {
            core::logger::warning(
                "SocketLib", "Read on socket `{}` would have blocked (SSL - Pending: {}, Want: {})",
                m_socket, SSL_pending(m_ssl), SSL_want(m_ssl));
            core::events::publish("socket.tls.receive.want_read", {{"fd", std::to_string(m_socket)}});
            return std::make_pair(0, SocketStatus(VALUES::NON_BLOCKING_WOULD_HAVE_BLOCKED));
        }
        case SSL_ERROR_WANT_WRITE: {
            core::logger::warning(
                "SocketLib", "Write on socket `{}` would have blocked (SSL - Pending: {}, Want: {})",
                m_socket, SSL_pending(m_ssl), SSL_want(m_ssl));
            core::events::publish("socket.tls.receive.want_write", {{"fd", std::to_string(m_socket)}});
            return std::make_pair(0, SocketStatus(VALUES::NON_BLOCKING_WOULD_HAVE_BLOCKED));
        }
        case SSL_ERROR_ZERO_RETURN: {
            core::logger::debug("SocketLib", "socket {} cleanly disconnected", m_socket);
            return std::make_pair(0, SocketStatus(VALUES::CLEANLY_DISCONNECTED));
        }
        case SSL_ERROR_SYSCALL: {
            const auto ERR = get_error_code();
            if (ERR == EWOULDBLOCK || ERR == EAGAIN) {
                core::logger::warning("SocketLib",
                                      "Receive on socket `{}` would have blocked, no data available "
                                      "(SSL - Pending: {}, Want: {})",
                                      m_socket, SSL_pending(m_ssl), SSL_want(m_ssl));
                core::events::publish("socket.tls.receive.would_block", {{"fd", std::to_string(m_socket)}});
                return std::make_pair(0, SocketStatus(VALUES::NON_BLOCKING_WOULD_HAVE_BLOCKED));
            }

            core::logger::warning(
                "SocketLib", "Socket `{}` critical failure in SSL syscall, errno: {}", m_socket, ERR);
            core::events::publish("socket.tls.receive.syscall_failure",
                                  {{"fd", std::to_string(m_socket)}, {"error_code", std::to_string(ERR)}});
            return std::make_pair(0, SocketStatus(VALUES::ERRORED, ERR));
        }
        default: {
            core::logger::warning(
                "SocketLib", "Socket `{}` critical failure in SSL_read_ex with error code `{}`",
                m_socket, SSL_ERR);
            core::events::publish("socket.tls.receive.ssl_failure",
                                  {{"fd", std::to_string(m_socket)}, {"ssl_error", std::to_string(SSL_ERR)}});
            return std::make_pair(0, SocketStatus(VALUES::ERRORED, SSL_ERR));
        }
        }
    }

    /**
     * @brief Computes the `recv()`/`recvfrom()` flags for sync_receive()'s wait-mode overload,
     * flipping non-blocking mode on Windows where there's no per-call flag for it — the same
     * shape used 3 times over (TCP, TLS-with-kTLS, UDP) in that overload, now one method. Logic
     * unchanged: `waitall_flag` is `MSG_WAITALL` for TCP/TLS-kTLS or `0` for UDP (which has no
     * MSG_WAITALL equivalent), matching each call site's original behavior exactly.
     * @param wait which wait mode the caller asked for.
     * @param waitall_flag the flag to use for WAIT_FOR_WHOLE_MESSAGE (`MSG_WAITALL`, or `0` for
     * protocols with no such concept).
     * @return the flags value to pass to the underlying `recv()`/`recvfrom()` call.
     */
    [[nodiscard]] int prepare_receive_wait_flags(WaitMode wait, int waitall_flag) {
        int flags = 0;

        if (is_waiting(wait)) {
            flags = waitall_flag;
        } else {
#ifdef _WIN32
            set_non_blocking(true);
#else
            flags = MSG_DONTWAIT;
#endif
        }

        return flags;
    }

    /**
     * @brief Runs `SSL_read_ex`, toggling `SSL_MODE_ENABLE_PARTIAL_WRITE` /
     * `SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER` around it for a non-waiting read so OpenSSL can hand
     * back less than `length` instead of insisting on a full fill — the exact same 3-step
     * toggle/read/restore shape sync_receive()'s wait-mode overload used inline for both its
     * non-kTLS TLS and QUIC branches, now one method. Logic and ordering unchanged.
     * @param wait which wait mode the caller asked for — the toggle only applies when not
     * WAIT_FOR_WHOLE_MESSAGE.
     * @param dest destination buffer, already resolved from the caller's output iterator.
     * @param length max bytes to receive into `dest`.
     * @param received_bytes out-param, filled by `SSL_read_ex`.
     * @return `SSL_read_ex`'s own return value.
     */
    int read_ssl_with_partial_mode_toggle(WaitMode wait, void *dest, std::size_t length,
                                          std::size_t &received_bytes)
        requires(Protocol == Protocol::TLS || Protocol == Protocol::QUIC)
    {
        if (!is_waiting(wait)) {
            SSL_set_mode(m_ssl, SSL_MODE_ENABLE_PARTIAL_WRITE | SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER);
        }

        int ssl_call_result = SSL_read_ex(m_ssl, dest, length, &received_bytes);

        if (!is_waiting(wait)) {
            SSL_clear_mode(m_ssl, SSL_MODE_ENABLE_PARTIAL_WRITE | SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER);
        }

        return ssl_call_result;
    }

    /**
     * @brief Classifies an async_accept() leverager completion result and, on failure, reports it
     * through `callback` — same would-block-vs-real-failure split as sync_accept(), pulled out to
     * its own method purely to bring async_accept()'s cognitive complexity down.
     * @param res the leverager completion result: the accepted fd on success, negative on error.
     * @param callback the caller's async_accept() callback — invoked with an invalid Socket if
     * this returns `true`.
     * @return `true` if `res` was a failure (callback already invoked, caller should bail),
     * `false` if `res` was a real accepted fd (caller should keep going).
     */
    [[nodiscard]] bool report_async_accept_failure(
        int res, std::move_only_function<void(Socket<Protocol>)> &callback) const {
        if (res >= 0) {
            return false;
        }

        const auto ERR = get_error_code();

        if (ERR == EWOULDBLOCK || ERR == EAGAIN || ERR == EINTR) {
            core::logger::debug("SocketLib", "socket {} accept would block", m_socket);
            callback(Socket<Protocol>{});
            return true;
        }

        core::logger::warning("SocketLib", "Socket `{}` critical failure in async accept", m_socket);
        core::events::publish("socket.async_accept.critical_failure",
                              {{"fd", std::to_string(m_socket)}, {"error_code", std::to_string(ERR)}});
        callback(Socket<Protocol>{});
        return true;
    }

    /**
     * @brief Wraps a freshly-accepted TLS client fd in a fresh `SSL` object, in server (accept)
     * mode — same setup async_accept() and sync_accept() both need, pulled out to its own method
     * purely to bring async_accept()'s cognitive complexity down.
     * @param client_fd the freshly-accepted client fd to wrap — closed by this method on failure.
     * @return the ready-to-use `SSL*` on success, `nullptr` on any setup failure (already logged
     * and cleaned up).
     */
    [[nodiscard]] SSL *create_accepted_tls_ssl(SOCKET client_fd) const
        requires(Protocol == Protocol::TLS)
    {
        SSL *client_ssl = SSL_new(m_ssl_ctx);
        if (client_ssl == nullptr) {
            closesocket(client_fd);
            core::logger::warning("SocketLib", "Failed to create SSL object for accepted connection");
            core::events::publish("socket.tls.create_ssl_object_failed", {{"site", "async_accept"}});
            return nullptr;
        }

        if (SSL_set_fd(client_ssl, client_fd) == 0) {
            SSL_free(client_ssl);
            closesocket(client_fd);
            core::logger::warning("SocketLib", "Failed to associate SSL object with accepted socket");
            core::events::publish("socket.tls.set_fd_failed", {{"site", "async_accept"}});
            return nullptr;
        }

        SSL_set_accept_state(client_ssl);
        return client_ssl;
    }

    /**
     * @brief One connect attempt against a single resolved address — this is what
     * sync_connect()'s address-walking loop calls per candidate.
     * @tparam CreateSocket when `true` (retry path), closes and re-opens `m_socket` for the new
     * address family before connecting; when `false` (first attempt), reuses the socket already
     * opened by the ctor.
     * @param addr the resolved address to attempt.
     * @param timeout milliseconds to wait for the connect to complete via a `select()` on the
     * writable/exceptional fd sets; 0 means a fully blocking connect with no timeout handling.
     * @return VALID on success, ERRORED on socket-creation failure, connect failure, or timeout.
     * @note Non-TCP/TLS protocols route to `core::logger::fatal` — properly gated by a real
     * if/else here.
     */
    // readability-function-cognitive-complexity: genuinely fixed, not NOLINT'd — the
    // non-blocking connect's select()/getsockopt() wait dance is now wait_for_connect_result(), a
    // private method below, pulled out purely to bring this function's complexity down; logic and
    // ordering (including the timeout/retry behavior) are unchanged.
    template <bool CreateSocket = true>
    SocketStatus connect(addrinfo *addr, std::uint64_t timeout) noexcept {
        if constexpr (Protocol == Protocol::TCP || Protocol == Protocol::TLS) {
            // Retry path: the previous attempt's socket is for the wrong address family, close
            // it and open a fresh one for this candidate. First attempt reuses the ctor's socket.
            if constexpr (CreateSocket) {
                core::logger::debug("SocketLib", "socket {} closing to retry connect", m_socket);
                sync_close();
                m_socket_address_info = nullptr;
                m_socket = ::socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
            }

            if (m_socket == INVALID_SOCKET) {
                core::logger::warning("SocketLib",
                                      "Failed to create socket for connection attempt");
                core::events::publish("socket.connect.create_socket_failed");
                return {VALUES::ERRORED};
            }

            m_socket_address_info = addr;

            // A timeout was requested — go non-blocking so the connect() below returns
            // immediately instead of blocking indefinitely, letting us select() with a deadline.
            if (timeout > 0) {
                set_non_blocking(true);
            }

            int err = ::connect(m_socket, addr->ai_addr, addr->ai_addrlen);
            if (err == SOCKET_ERROR) {
                err = get_error_code();

                // Non-blocking connect kicked off but hasn't finished — wait for it to become
                // writable (or error out) via select(), bounded by the caller's timeout.
                if (err == EINPROGRESS || err == EWOULDBLOCK || err == EAGAIN) {
                    core::logger::debug("SocketLib", "socket {} connect in progress timeout={}ms",
                                        m_socket, timeout);
                    err = wait_for_connect_result(timeout);
                }
            }

            // Restore blocking mode before returning either way — this is a synchronous helper,
            // no reason to leave the socket non-blocking behind the caller's back.
            if (timeout > 0) {
                set_non_blocking(false);
            }

            if (err != 0) {
                sync_close();
                m_socket_address_info = nullptr;
                core::logger::warning("SocketLib",
                                      "Connect attempt on socket `{}` failed with error code `{}`",
                                      m_socket, err);
                core::events::publish("socket.connect.attempt_failed",
                                      {{"fd", std::to_string(m_socket)}, {"error_code", std::to_string(err)}});
                return {VALUES::ERRORED};
            }

            core::logger::debug("SocketLib", "socket {} connected {}:{}", m_socket,
                                m_endpoint.get_address(), m_endpoint.get_port());
            return {VALUES::VALID};
        } else {
            core::events::publish("socket.connect_unsupported_protocol", {{"site", "connect_attempt"}});
            core::logger::fatal("SocketLib", "Connect is only supported for TCP and TLS protocols");
        }
    }

    /**
     * @brief Waits (via `select()`) for a non-blocking connect() to finish, bounded by `timeout`,
     * then pulls the real result out of `SO_ERROR` — the select()/getsockopt() half of connect()'s
     * retry dance, pulled out to its own method purely to bring connect()'s cognitive complexity
     * down. Logic and ordering are unchanged from the original inline code.
     * @param timeout milliseconds to wait for the connect to complete.
     * @return `0` on a successful connect, `ETIMEDOUT` if the deadline passed with nothing ready,
     * or the real connect/select error code otherwise.
     */
    [[nodiscard]] int wait_for_connect_result(std::uint64_t timeout) const {
        struct timeval tv{};
        tv.tv_sec = static_cast<decltype(tv.tv_sec)>(timeout / 1000);
        tv.tv_usec = static_cast<decltype(tv.tv_usec)>((timeout % 1000) * 1000);

        fd_set writefds;
        fd_set exceptfds;
        FD_ZERO(&writefds);
        FD_SET(m_socket, &writefds);
        FD_ZERO(&exceptfds);
        FD_SET(m_socket, &exceptfds);

        // Ready, timed out, or select() itself errored — pull the real connect result out of
        // SO_ERROR only in the "ready" case.
        int select_result =
            ::select(static_cast<int>(m_socket + 1), nullptr, &writefds, &exceptfds, &tv);
        if (select_result > 0) {
            int err = 0;
            socklen_t len = sizeof(err);
            if (getsockopt(m_socket, SOL_SOCKET, SO_ERROR, reinterpret_cast<char *>(&err), &len) <  // FIXME(clang-tidy): reinterpret_cast usage
                0) {
                core::logger::error("SocketLib", "getsockopt failed after select");
                core::events::publish("socket.getsockopt_failed", {{"fd", std::to_string(m_socket)}});
            }
            return err;
        }
        if (select_result == 0) {
            return ETIMEDOUT;
        }
        return get_error_code();
    }

    /**
     * @brief Fills `m_address_info_hint` with the right `ai_socktype`/`ai_protocol` for this
     * Socket's compile-time protocol (stream+TCP for TCP/TLS, dgram+UDP for UDP/QUIC), plus
     * `AI_ADDRCONFIG` so `getaddrinfo` only returns address families the host actually has
     * configured. Called from every constructor that resolves an address.
     */
    void init_address_info() {
        int sock_type{};
        int iprotocol{};

        // Stream+TCP for connection-oriented protocols, dgram+UDP for everything datagram-based
        // — QUIC rides on UDP at the transport layer even though it's a stream API up top.
        if constexpr (Protocol == Protocol::TCP || Protocol == Protocol::TLS) {
            sock_type = SOCK_STREAM;
            iprotocol = IPPROTO_TCP;
        } else if constexpr (Protocol == Protocol::UDP || Protocol == Protocol::QUIC) {
            sock_type = SOCK_DGRAM;
            iprotocol = IPPROTO_UDP;
        }

        m_address_info_hint = {};
        m_address_info_hint.ai_family = AF_UNSPEC;
        m_address_info_hint.ai_socktype = sock_type;
        m_address_info_hint.ai_protocol = iprotocol;
        m_address_info_hint.ai_flags = AI_ADDRCONFIG;
    }

    /**
     * @brief Wraps the already-open UDP socket in a connected datagram BIO for OpenSSL's QUIC
     * stack to read/write through — called from both the connect and listen/accept QUIC paths.
     * @warning `BIO_set_fd`/`BIO_ctrl` run unconditionally even if `BIO_new` returned null (the
     * null check only logs, doesn't bail) — that's a null-pointer call into OpenSSL's BIO API on
     * allocation failure, not a guarded skip.
     */
    void add_quic_bio() {
        // Wrap the existing UDP fd in a datagram BIO (BIO_NOCLOSE — this BIO doesn't own the
        // fd, sync_close()/async_close() still handle that), then mark it connected so OpenSSL
        // treats it like a point-to-point stream instead of routing per-datagram addresses.
        m_bio = BIO_new(BIO_s_datagram());
        if (m_bio == nullptr) {
            core::logger::error("SocketLib", "Failed to create BIO for QUIC socket");
            core::events::publish("socket.quic.create_bio_failed");
        }
        BIO_set_fd(m_bio, static_cast<SOCKET>(m_socket), BIO_NOCLOSE);
        BIO_ctrl(m_bio, BIO_CTRL_DGRAM_SET_CONNECTED, 0, nullptr);

        core::logger::debug("SocketLib", "socket {} QUIC BIO ready", m_socket);
    }

    /**
     * @brief One-time runtime probe for whether the kernel actually supports TLS ULP offload
     * (kTLS) — separate from OpenSSL's own `SSL_OP_ENABLE_KTLS` flag, which just tells OpenSSL
     * "attempt it if available" and is documented to fall back to userspace `SSL_write_ex`/
     * `SSL_read_ex` if the kernel refuses. That fallback exists and works (see sync_send()/
     * sync_receive()'s `!m_ktls_tx`/`!m_ktls_rx` branches) — but on hosts without the `tls`
     * kernel module loaded (common in containers/VMs that never `modprobe tls`), letting
     * OpenSSL attempt the enable anyway has been observed to leave the socket unable to write
     * even via that fallback. Checking kernel support directly, up front, and skipping the
     * opt-in entirely when it's unsupported avoids ever hitting that path.
     * @return `true` if `TCP_ULP "tls"` is actually accepted by this kernel, `false` otherwise
     * (including on Windows, which has no kTLS concept at all).
     */
    [[nodiscard]] static bool ktls_kernel_supported() noexcept {
#ifdef _WIN32
        return false;
#else
        static const bool SUPPORTED = [] {
            int probe_fd = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            if (probe_fd < 0) {
                return false;
            }
            constexpr char ULP_NAME[] = "tls";
            bool ok =
                ::setsockopt(probe_fd, IPPROTO_TCP, TCP_ULP, ULP_NAME, sizeof(ULP_NAME)) == 0;
            closesocket(probe_fd);
            return ok;
        }();
        return SUPPORTED;
#endif
    }

    /**
     * @brief Builds the client-side `SSL_CTX`/`SSL` for a TLS connection — sets ALPN (if any),
     * enables kTLS offload, requires+verifies the peer cert against default trust paths, and
     * sets SNI from `m_endpoint`. Called from sync_connect()/async_connect() after the raw TCP
     * connect lands. Constrained to `Protocol::TLS` via `requires`, so it's only even visible
     * for TLS instantiations of this class template.
     * @return `true` once the SSL object is fully configured and in connect-state, ready for
     * sync_handshake()/async_handshake(); `false` on any setup step failing (context/object
     * creation, fd association, or SNI hostname).
     */
    bool setup_tls()
        requires(Protocol == Protocol::TLS)
    {
        // Client-side SSL_CTX first — bail immediately if OpenSSL can't even allocate one.
        m_ssl_ctx = SSL_CTX_new(TLS_client_method());
        if (m_ssl_ctx == nullptr) {
            core::logger::warning("SocketLib", "Failed to create SSL context");
            core::events::publish("socket.tls.create_context_failed", {{"site", "setup_tls"}});
            return false;
        }

        SSL_CTX_set_info_callback(m_ssl_ctx, [](const SSL *ssl, int where, int ret) {
            if (where & SSL_CB_ALERT) {
                const char *type = (where & SSL_CB_READ) ? "read" : "write";
                core::logger::warning("SocketLib", "SSL alert [{}] on socket {}: {} - {}", type,
                                      reinterpret_cast<const void *>(ssl), SSL_alert_type_string_long(ret),  // FIXME(clang-tidy): reinterpret_cast usage
                                      SSL_alert_desc_string_long(ret));
                core::events::publish("socket.tls.alert",
                                      {{"direction", type},
                                       {"alert_type", SSL_alert_type_string_long(ret)},
                                       {"alert_desc", SSL_alert_desc_string_long(ret)}});
            }
        });

        // Advertise ALPN protocols only if the caller configured any.
        if (!m_alpn_wire_format.empty()) {
            core::logger::debug("SocketLib", "ALPN protos set for TLS socket");
            SSL_CTX_set_alpn_protos(m_ssl_ctx, m_alpn_wire_format.data(),
                                    m_alpn_wire_format.size());
        }

        // Opt into kernel TLS offload only when the kernel actually supports it — see
        // ktls_kernel_supported()'s own doc comment for why this is checked directly rather than
        // just always setting the flag and trusting OpenSSL's fallback.
        if (ktls_kernel_supported()) {
            SSL_CTX_set_options(m_ssl_ctx, SSL_OP_ENABLE_KTLS);
        }

        // Require+verify the peer cert against the default trust store, unless the caller
        // opted out via set_verify_peer(false) — e.g. connecting to a self-signed dev cert.
        SSL_CTX_set_verify(m_ssl_ctx, m_verify_peer ? SSL_VERIFY_PEER : SSL_VERIFY_NONE, nullptr);
        SSL_CTX_set_default_verify_paths(m_ssl_ctx);

        // Context's ready — now build the actual SSL object and wire it to the connected fd.
        m_ssl = SSL_new(m_ssl_ctx);
        if (m_ssl == nullptr) {
            core::logger::warning("SocketLib", "Failed to create SSL object");
            core::events::publish("socket.tls.create_ssl_object_failed", {{"site", "setup_tls"}});
            return false;
        }

        if (SSL_set_fd(m_ssl, m_socket) == 0) {
            core::logger::warning("SocketLib", "Failed to associate SSL object with socket");
            core::events::publish("socket.tls.set_fd_failed", {{"site", "setup_tls"}});
            return false;
        }

        // SNI so the server can pick the right cert for virtual-hosted TLS.
        if (SSL_set_tlsext_host_name(m_ssl, m_endpoint.get_address().data()) != 1) {
            core::logger::warning("SocketLib", "Failed to set SNI hostname");
            core::events::publish("socket.tls.set_sni_failed", {{"site", "setup_tls"}});
            return false;
        }

        core::logger::debug("SocketLib", "socket {} TLS SSL ready SNI={}", m_socket,
                            m_endpoint.get_address());
        SSL_set_connect_state(m_ssl);
        return true;
    }

    /**
     * @brief QUIC counterpart to setup_tls() — wraps the socket in a datagram BIO via
     * add_quic_bio() first, then builds the client-side `SSL_CTX`/`SSL` on top of it with the
     * same ALPN/verify/SNI setup shape as setup_tls(). Constrained to `Protocol::QUIC` via
     * `requires`.
     * @return `true` once the SSL object is fully configured and in connect-state; `false` on
     * any setup step failing.
     */
    bool setup_quic()
        requires(Protocol == Protocol::QUIC)
    {
        // QUIC needs the datagram BIO wrapped around the socket before OpenSSL's QUIC stack has
        // anything to bind its SSL object to.
        add_quic_bio();

        m_ssl_ctx = SSL_CTX_new(OSSL_QUIC_client_method());
        if (m_ssl_ctx == nullptr) {
            core::logger::warning("SocketLib", "Failed to create SSL context for QUIC");
            core::events::publish("socket.tls.create_context_failed", {{"site", "setup_quic"}});
            return false;
        }

        SSL_CTX_set_info_callback(m_ssl_ctx, [](const SSL *ssl, int where, int ret) {
            if (where & SSL_CB_ALERT) {
                const char *type = (where & SSL_CB_READ) ? "read" : "write";
                core::logger::warning("SocketLib", "SSL alert [{}] on socket {}: {} - {}", type,
                                      reinterpret_cast<const void *>(ssl), SSL_alert_type_string_long(ret),  // FIXME(clang-tidy): reinterpret_cast usage
                                      SSL_alert_desc_string_long(ret));
                core::events::publish("socket.tls.alert",
                                      {{"direction", type},
                                       {"alert_type", SSL_alert_type_string_long(ret)},
                                       {"alert_desc", SSL_alert_desc_string_long(ret)}});
            }
        });

        // Same ALPN-if-configured + require-verified-peer setup as setup_tls().
        if (!m_alpn_wire_format.empty()) {
            core::logger::debug("SocketLib", "ALPN protos set for QUIC socket");
            SSL_CTX_set_alpn_protos(m_ssl_ctx, m_alpn_wire_format.data(),
                                    m_alpn_wire_format.size());
        }

        SSL_CTX_set_verify(m_ssl_ctx, SSL_VERIFY_PEER, nullptr);
        SSL_CTX_set_default_verify_paths(m_ssl_ctx);

        // Build the SSL object and hand it the BIO from add_quic_bio() above (QUIC has no
        // SSL_set_fd equivalent — it always talks through a BIO).
        m_ssl = SSL_new(m_ssl_ctx);
        if (m_ssl == nullptr) {
            core::logger::warning("SocketLib", "Failed to create SSL object for QUIC");
            core::events::publish("socket.tls.create_ssl_object_failed", {{"site", "setup_quic"}});
            return false;
        }

        SSL_set_bio(m_ssl, m_bio, m_bio);

        // SNI, same reasoning as setup_tls().
        if (SSL_set_tlsext_host_name(m_ssl, m_endpoint.get_address().data()) != 1) {
            core::logger::warning("SocketLib", "Failed to set SNI hostname for QUIC");
            core::events::publish("socket.tls.set_sni_failed", {{"site", "setup_quic"}});
            return false;
        }

        core::logger::debug("SocketLib", "socket {} QUIC SSL ready SNI={}", m_socket,
                            m_endpoint.get_address());
        SSL_set_connect_state(m_ssl);
        return true;
    }

    /**
     * @brief Shared exception-recovery path for async_send()/async_receive() — both are declared
     * `noexcept` (required by `interfaces::io::AsyncSendable`/`AsyncReceivable`, see the note on
     * async_send()), so their bodies wrap everything in a top-level try/catch and funnel here on
     * any exception. Kept as its own method specifically so its own try/catch nesting doesn't
     * count against async_send()'s/async_receive()'s cognitive complexity.
     * @param operation_name which caller this is, purely for the log message ("async_send" or
     * "async_receive").
     * @param callback the caller's callback — may already be moved-from (empty) if the exception
     * came from moving it into a leverager completion lambda; `if (callback)` on a
     * `move_only_function` is a safe, well-defined check for that, so this only re-invokes it
     * when it's still valid, and that re-invocation is itself guarded so a throwing callback
     * can't escape either.
     */
    void report_async_io_exception(
        const char *operation_name,
        std::move_only_function<void(std::size_t, SocketStatus)> &callback) const noexcept {
        try {
            std::rethrow_exception(std::current_exception());
        } catch (const std::exception &caught_error) {
            try {
                core::logger::warning("SocketLib", "Socket `{}` {} raised an exception: {}", m_socket,
                                      operation_name, caught_error.what());
                core::events::publish("socket.async_io.exception",
                                      {{"fd", std::to_string(m_socket)},
                                       {"operation", std::string{operation_name}},
                                       {"error", caught_error.what()}});
            } catch (...) {  // NOLINT(bugprone-empty-catch) — best-effort diagnostic only
            }
        } catch (...) {
            try {
                core::logger::warning("SocketLib", "Socket `{}` {} raised an unknown exception",
                                      m_socket, operation_name);
                core::events::publish("socket.async_io.unknown_exception",
                                      {{"fd", std::to_string(m_socket)},
                                       {"operation", std::string{operation_name}}});
            } catch (...) {  // NOLINT(bugprone-empty-catch) — best-effort diagnostic only
            }
        }
        if (callback) {
            try {
                callback(0, SocketStatus(VALUES::ERRORED));
            } catch (...) {  // NOLINT(bugprone-empty-catch) — nowhere left to report a second failure to
            }
        }
    }

    /**
     * @brief Shared completion handler for async_send()'s TCP/TLS and both UDP (with/without an
     * explicit destination) leverager callbacks — the three were byte-identical, so they're one
     * method now instead of three copies. Same would-block-vs-critical-failure-vs-success mapping
     * as sync_send()'s plain-syscall path.
     * @param sent_bytes the leverager completion result: bytes sent on success, negative on error.
     * @param callback the caller's async_send() callback — invoked exactly once with the mapped
     * status.
     */
    void complete_async_send(int sent_bytes,
                             std::move_only_function<void(std::size_t, SocketStatus)> &callback) const {
        if (sent_bytes < 0) {
            const auto ERR = get_error_code();
            if (ERR == EWOULDBLOCK) {
                core::logger::warning(
                    "SocketLib", "Send on socket `{}` would have blocked, no buffer space available",
                    m_socket);
                core::events::publish("socket.async_send.would_block", {{"fd", std::to_string(m_socket)}});
                callback(0, SocketStatus(VALUES::NON_BLOCKING_WOULD_HAVE_BLOCKED));
                return;
            }
            core::logger::warning("SocketLib", "Socket `{}` critical failure when sending data async",
                                  m_socket);
            core::events::publish("socket.async_send.critical_failure",
                                  {{"fd", std::to_string(m_socket)}, {"error_code", std::to_string(ERR)}});
            callback(0, SocketStatus(VALUES::ERRORED));
            return;
        }

        core::logger::debug("SocketLib", "socket {} sent {} bytes", m_socket, sent_bytes);
        callback(sent_bytes, SocketStatus(VALUES::VALID));
    }

    /**
     * @brief Body of async_send()'s QUIC self-re-arming attempt lambda, pulled out to its own
     * method purely to bring async_send()'s cognitive complexity down — logic and ordering are
     * unchanged from the original inline lambda.
     * @param res the re-arm completion result (negative means the re-arm itself failed).
     * @param buffer bytes to send, forwarded to `SSL_write_ex`.
     * @param length number of bytes in `buffer`.
     * @param attempt the shared re-arm callback, passed through to handle_async_send_ssl_wait() on
     * a want-read/want-write outcome.
     * @param callback the caller's async_send() callback.
     * @param iflags forwarded to the leverager's recv()/send() re-arm call.
     */
    void attempt_async_quic_send(int res, const std::byte *buffer, std::size_t length,
                                 std::shared_ptr<std::function<void(int)>> &attempt,
                                 std::move_only_function<void(std::size_t, SocketStatus)> &callback,
                                 std::uint8_t iflags)
        requires(Protocol == Protocol::QUIC)
    {
        // The re-arm itself failed — nothing left to retry.
        if (res < 0) {
            core::logger::warning("SocketLib",
                                  "Socket `{}` async send attempt failed with error code `{}`",
                                  m_socket, res);
            core::events::publish("socket.async_send.attempt_failed",
                                  {{"fd", std::to_string(m_socket)}, {"error_code", std::to_string(res)}});
            callback(0, SocketStatus(VALUES::ERRORED));
            return;
        }

        // Try the actual SSL write — success reports straight back through the callback, no more
        // re-arming needed.
        std::size_t sent_bytes = 0;
        int ret = SSL_write_ex(m_ssl, buffer, length, &sent_bytes);
        if (ret > 0) {
            core::logger::debug("SocketLib", "socket {} sent {} bytes", m_socket, sent_bytes);
            callback(sent_bytes, SocketStatus(VALUES::VALID));
            return;
        }

        // Not done — figure out what OpenSSL's waiting on and re-arm the leverager on that
        // direction, looping this same lambda until it lands or hard-fails.
        if (sent_bytes <= 0) {
            handle_async_send_ssl_wait(static_cast<int>(sent_bytes), attempt, callback, iflags);
        }
    }

    /**
     * @brief Classifies the SSL error from async_send()'s QUIC retry loop and either re-arms the
     * leverager on the right direction (want-read/want-write) or reports a terminal status through
     * `callback` — same SSL-error-to-SocketStatus mapping as sync_send()'s SSL_write_ex path,
     * pulled out to its own method purely to bring async_send()'s cognitive complexity down.
     * @param ssl_call_result the value to feed `SSL_get_error` — matches the original inline code,
     * which (pre-existing behavior, unchanged here) passes the truncated `sent_bytes` rather than
     * SSL_write_ex()'s own return value.
     * @param attempt the shared re-arm callback, re-submitted to the leverager on want-read/want-write.
     * @param callback the caller's async_send() callback — invoked on a terminal (non-retry) outcome.
     * @param iflags forwarded to the leverager's recv()/send() re-arm call.
     */
    void handle_async_send_ssl_wait(int ssl_call_result, std::shared_ptr<std::function<void(int)>> &attempt,
                                    std::move_only_function<void(std::size_t, SocketStatus)> &callback,
                                    std::uint8_t iflags)
        requires(Protocol == Protocol::QUIC)
    {
        const int SSL_ERR = SSL_get_error(m_ssl, ssl_call_result);

        switch (SSL_ERR) {
        case SSL_ERROR_WANT_READ: {
            // Pre-existing dormant bug fixed here: the original inline version of this switch
            // (Socket<QUIC> is never instantiated anywhere in the tree, so this was dead code no
            // one ever compiled) only passed 2 args for this format string's 3 placeholders,
            // dropping SSL_want(m_ssl). Extracting this into its own method makes the format
            // string a non-dependent expression the compiler checks eagerly (instead of being
            // skipped as an uninstantiated `if constexpr` branch), which surfaced the mismatch as
            // a hard error. Restoring the missing SSL_want(m_ssl) arg matches every sibling
            // occurrence of this exact message elsewhere in the file (sync_send/sync_receive's
            // WANT_READ case).
            core::logger::warning(
                "SocketLib", "Read on socket `{}` would have blocked (SSL - Pending: {}, Want: {})",
                m_socket, SSL_pending(m_ssl), SSL_want(m_ssl));
            core::events::publish("socket.tls.async_send.want_read", {{"fd", std::to_string(m_socket)}});
            m_leverager->get().recv(m_socket, nullptr, 0, 0, *attempt, iflags);
            break;
        }
        case SSL_ERROR_WANT_WRITE: {
            // Same dormant-bug fix as SSL_ERROR_WANT_READ above — restoring the dropped
            // SSL_want(m_ssl) arg.
            core::logger::warning(
                "SocketLib", "Write on socket `{}` would have blocked (SSL - Pending: {}, Want: {})",
                m_socket, SSL_pending(m_ssl), SSL_want(m_ssl));
            core::events::publish("socket.tls.async_send.want_write", {{"fd", std::to_string(m_socket)}});
            m_leverager->get().send(m_socket, nullptr, 0, 0, *attempt, iflags);
            break;
        }
        case SSL_ERROR_ZERO_RETURN: {
            core::logger::debug("SocketLib", "socket {} cleanly disconnected", m_socket);
            callback(0, SocketStatus(VALUES::CLEANLY_DISCONNECTED));
            break;
        }
        case SSL_ERROR_SYSCALL: {
            if (get_error_code() == EWOULDBLOCK) {
                // Same dormant-bug fix as above — restoring the dropped SSL_want(m_ssl) arg.
                core::logger::warning("SocketLib",
                                      "Sending on socket `{}` would have blocked, no data available "
                                      "(SSL - Pending: {}, Want: {})",
                                      m_socket, SSL_pending(m_ssl), SSL_want(m_ssl));
                core::events::publish("socket.tls.async_send.would_block", {{"fd", std::to_string(m_socket)}});
                callback(0, SocketStatus(VALUES::NON_BLOCKING_WOULD_HAVE_BLOCKED));
                break;
            }

            core::logger::warning(
                "SocketLib", "Socket `{}` critical failure in SSL_write_ex() with system error code `{}`",
                m_socket, get_error_code());
            core::events::publish("socket.tls.async_send.syscall_failure",
                                  {{"fd", std::to_string(m_socket)},
                                   {"error_code", std::to_string(get_error_code())}});
            callback(0, SocketStatus(VALUES::ERRORED));
            break;
        }
        default: {
            core::logger::warning(
                "SocketLib", "Socket `{}` critical failure in SSL_write_ex() with SSL error code `{}`",
                m_socket, SSL_ERR);
            core::events::publish("socket.tls.async_send.ssl_failure",
                                  {{"fd", std::to_string(m_socket)}, {"ssl_error", std::to_string(SSL_ERR)}});
            callback(0, SocketStatus(VALUES::ERRORED));
        }
        }
    }

    /**
     * @brief Shared completion handler for async_receive()'s TCP/TLS and UDP leverager callbacks
     * — the two were byte-identical, so they're one method now instead of two copies. Same
     * would-block-vs-critical-failure-vs-clean-disconnect mapping as sync_receive()'s
     * plain-syscall path.
     * @param received_bytes the leverager completion result: bytes received on success, negative
     * on error, 0 on a clean disconnect.
     * @param callback the caller's async_receive() callback — invoked with the mapped status.
     */
    void complete_async_receive(
        int received_bytes, std::move_only_function<void(std::size_t, SocketStatus)> &callback) const {
        if (received_bytes < 0) {
            const auto ERR = get_error_code();
            if (ERR == EWOULDBLOCK || ERR == EAGAIN) {
                core::logger::warning(
                    "SocketLib", "Receive on socket `{}` would have blocked, no data available",
                    m_socket);
                core::events::publish("socket.async_receive.would_block", {{"fd", std::to_string(m_socket)}});
                callback(0, SocketStatus(VALUES::NON_BLOCKING_WOULD_HAVE_BLOCKED));
                return;
            }
            core::logger::warning("SocketLib", "Socket `{}` critical failure in recv() syscall",
                                  m_socket);
            core::events::publish("socket.async_receive.critical_failure",
                                  {{"fd", std::to_string(m_socket)}, {"error_code", std::to_string(ERR)}});
            callback(0, SocketStatus(VALUES::ERRORED));
        } else if (received_bytes == 0) {
            core::logger::debug("SocketLib", "socket {} cleanly disconnected", m_socket);
            callback(0, SocketStatus(VALUES::CLEANLY_DISCONNECTED));
        }
    }

    /**
     * @brief Body of async_receive()'s QUIC self-re-arming attempt lambda, pulled out to its own
     * method purely to bring async_receive()'s cognitive complexity down — logic and ordering are
     * unchanged from the original inline lambda. `out` is passed in already resolved to a raw
     * pointer (via `std::to_address`) rather than as the original templated iterator, since a
     * private member method can't itself be templated on async_receive()'s `Out` without dragging
     * that template parameter along; the resolved address is exactly what the original lambda
     * body used it for anyway (feeding `SSL_read_ex`).
     * @param res the re-arm completion result (negative means the re-arm itself failed).
     * @param data_ptr destination buffer, already resolved from the caller's output iterator.
     * @param length max bytes to receive into `data_ptr`.
     * @param attempt the shared re-arm callback, passed through to handle_async_receive_ssl_wait()
     * on a want-read/want-write outcome.
     * @param callback the caller's async_receive() callback.
     * @param iflags forwarded to the leverager's recv()/send() re-arm call.
     */
    void attempt_async_quic_receive(int res, char *data_ptr, std::size_t length,
                                    std::shared_ptr<std::function<void(int)>> &attempt,
                                    std::move_only_function<void(std::size_t, SocketStatus)> &callback,
                                    std::uint8_t iflags)
        requires(Protocol == Protocol::QUIC)
    {
        // The re-arm itself failed — nothing left to retry.
        if (res < 0) {
            core::logger::warning("SocketLib",
                                  "Socket `{}` async read attempt failed with error code `{}`",
                                  m_socket, res);
            core::events::publish("socket.async_receive.attempt_failed",
                                  {{"fd", std::to_string(m_socket)}, {"error_code", std::to_string(res)}});
            callback(0, SocketStatus(VALUES::ERRORED));
            return;
        }

        // Try the actual SSL read — success reports straight back, no more re-arming needed.
        std::size_t received_bytes = 0;
        int ret = SSL_read_ex(m_ssl, data_ptr, length, &received_bytes);
        if (ret > 0) {
            core::logger::debug("SocketLib", "socket {} received {} bytes", m_socket, received_bytes);
            callback(received_bytes, SocketStatus(VALUES::VALID));
            return;
        }

        // Not done — same want-read/want-write re-arm loop as async_send()'s QUIC branch, just
        // recv-driven instead of send-driven.
        if (received_bytes <= 0) {
            handle_async_receive_ssl_wait(static_cast<int>(received_bytes), attempt, callback, iflags);
        }
    }

    /**
     * @brief Classifies the SSL error from async_receive()'s QUIC retry loop and either re-arms
     * the leverager on the right direction (want-read/want-write) or reports a terminal status
     * through `callback` — same SSL-error-to-SocketStatus mapping as sync_receive()'s SSL_read_ex
     * path, pulled out to its own method purely to bring async_receive()'s cognitive complexity
     * down.
     * @param ssl_call_result the value to feed `SSL_get_error` — matches the original inline code,
     * which (pre-existing behavior, unchanged here) passes the truncated `received_bytes` rather
     * than SSL_read_ex()'s own return value.
     * @param attempt the shared re-arm callback, re-submitted to the leverager on want-read/want-write.
     * @param callback the caller's async_receive() callback — invoked on a terminal outcome.
     * @param iflags forwarded to the leverager's recv()/send() re-arm call.
     */
    void handle_async_receive_ssl_wait(int ssl_call_result,
                                       std::shared_ptr<std::function<void(int)>> &attempt,
                                       std::move_only_function<void(std::size_t, SocketStatus)> &callback,
                                       std::uint8_t iflags)
        requires(Protocol == Protocol::QUIC)
    {
        const int SSL_ERR = SSL_get_error(m_ssl, ssl_call_result);

        switch (SSL_ERR) {
        case SSL_ERROR_WANT_READ: {
            // Same dormant dropped-argument bug as handle_async_send_ssl_wait()'s WANT_READ case
            // (Socket<QUIC> is never instantiated anywhere in the tree, so this was dead code no
            // one ever compiled) — restoring the missing SSL_want(m_ssl) arg to match every
            // sibling occurrence of this message elsewhere in the file.
            core::logger::warning(
                "SocketLib", "Read on socket `{}` would have blocked (SSL - Pending: {}, Want: {})",
                m_socket, SSL_pending(m_ssl), SSL_want(m_ssl));
            core::events::publish("socket.tls.async_receive.want_read", {{"fd", std::to_string(m_socket)}});
            m_leverager->get().recv(m_socket, nullptr, 0, 0, *attempt, iflags);
            break;
        }
        case SSL_ERROR_WANT_WRITE: {
            // Same dormant-bug fix as SSL_ERROR_WANT_READ above.
            core::logger::warning(
                "SocketLib", "Write on socket `{}` would have blocked (SSL - Pending: {}, Want: {})",
                m_socket, SSL_pending(m_ssl), SSL_want(m_ssl));
            core::events::publish("socket.tls.async_receive.want_write", {{"fd", std::to_string(m_socket)}});
            m_leverager->get().send(m_socket, nullptr, 0, 0, *attempt, iflags);
            break;
        }
        case SSL_ERROR_ZERO_RETURN: {
            core::logger::debug("SocketLib", "socket {} cleanly disconnected", m_socket);
            callback(0, SocketStatus(VALUES::CLEANLY_DISCONNECTED));
            break;
        }
        case SSL_ERROR_SYSCALL: {
            auto err = get_error_code();
            if (err == EWOULDBLOCK) {
                // Same dormant-bug fix as above — restoring the dropped SSL_want(m_ssl) arg.
                core::logger::warning("SocketLib",
                                      "Receive on socket `{}` would have blocked, no data available "
                                      "(SSL - Pending: {}, Want: {})",
                                      m_socket, SSL_pending(m_ssl), SSL_want(m_ssl));
                core::events::publish("socket.tls.async_receive.would_block", {{"fd", std::to_string(m_socket)}});
                callback(0, SocketStatus(VALUES::NON_BLOCKING_WOULD_HAVE_BLOCKED));
                break;
            }

            core::logger::warning("SocketLib", "Socket `{}` critical failure in SSL syscall, errno: {}",
                                  m_socket, err);
            core::events::publish("socket.tls.async_receive.syscall_failure",
                                  {{"fd", std::to_string(m_socket)}, {"error_code", std::to_string(err)}});
            callback(0, SocketStatus(VALUES::ERRORED));
            break;
        }
        default: {
            core::logger::warning(
                "SocketLib", "Socket `{}` critical failure in SSL_read_ex with error code `{}`",
                m_socket, SSL_ERR);
            core::events::publish("socket.tls.async_receive.ssl_failure",
                                  {{"fd", std::to_string(m_socket)}, {"ssl_error", std::to_string(SSL_ERR)}});
            callback(0, SocketStatus(VALUES::ERRORED));
        }
        }
    }

    SOCKET m_socket;
    SSL *m_ssl;
    SSL_CTX *m_ssl_ctx;
    BIO *m_bio;
    Endpoint m_endpoint;
    addrinfo m_address_info_hint;
    addrinfo *m_address_info_result;
    addrinfo *m_socket_address_info;
    sockaddr_storage m_socket_input_buffer;
    socklen_t m_socket_input_buffer_length;
    std::vector<unsigned char> m_alpn_wire_format;
    std::optional<std::reference_wrapper<leverage::Leverager<leverage::Context>>> m_leverager;
    bool m_ktls_tx;
    bool m_ktls_rx;
    bool m_verify_peer{true};
    [[no_unique_address]] OsPayload m_os;
};

} // namespace io::base::socket
