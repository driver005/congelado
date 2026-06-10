module io.base.socket;
@nogc nothrow:

import io.error.base   : handle_error;
import io.base.leverage.types : off_t;
import io.base.socket.consts;
import shared_.logger : error, warning, debug_, fatal;
import util.optional  : Optional;

public import io.base.socket.consts;

version (Windows) {
    public import io.base.socket.win32;
} else {
    public import io.base.socket.posix;
}

// OpenSSL bindings (minimal; see modules/openssl.d for full set)
extern(C) {
    struct SSL;
    struct SSL_CTX;
    struct BIO;
    struct BIO_METHOD;
    struct addrinfo;
    struct sockaddr_storage;

    int  SSL_do_handshake(SSL* ssl) @nogc nothrow;
    int  SSL_write_ex(SSL* ssl, const(void)* buf, size_t num, size_t* written) @nogc nothrow;
    int  SSL_read_ex(SSL* ssl, void* buf, size_t num, size_t* readbytes) @nogc nothrow;
    void SSL_free(SSL* ssl) @nogc nothrow;
    void SSL_CTX_free(SSL_CTX* ctx) @nogc nothrow;
    void BIO_free(BIO* bio) @nogc nothrow;
    void SSL_set_shutdown(SSL* ssl, int mode) @nogc nothrow;
    int  SSL_shutdown(SSL* ssl) @nogc nothrow;
    int  SSL_get_error(const(SSL)* ssl, int ret) @nogc nothrow;
    int  SSL_pending(const(SSL)* ssl) @nogc nothrow;
    int  SSL_want(const(SSL)* ssl) @nogc nothrow;
    int  SSL_is_init_finished(const(SSL)* ssl) @nogc nothrow;
    int  SSL_set_fd(SSL* ssl, int fd) @nogc nothrow;
    void SSL_set_accept_state(SSL* ssl) @nogc nothrow;
    void SSL_set_connect_state(SSL* ssl) @nogc nothrow;
    void SSL_set_bio(SSL* ssl, BIO* rbio, BIO* wbio) @nogc nothrow;
    SSL* SSL_new(SSL_CTX* ctx) @nogc nothrow;
    SSL* SSL_accept_connection(SSL* ssl, uint flags) @nogc nothrow;
    int  SSL_set_tlsext_host_name(SSL* ssl, const(char)* name) @nogc nothrow;
    int  SSL_set_mode(SSL* ssl, long mode) @nogc nothrow;
    int  SSL_clear_mode(SSL* ssl, long mode) @nogc nothrow;
    int  BIO_get_ktls_send(BIO* bio) @nogc nothrow;
    int  BIO_get_ktls_recv(BIO* bio) @nogc nothrow;
    int  BIO_set_fd(BIO* bio, int fd, int close_flag) @nogc nothrow;
    int  BIO_ctrl(BIO* bio, int cmd, long larg, void* parg) @nogc nothrow;
    const(BIO_METHOD)* BIO_s_datagram() @nogc nothrow;
    BIO* BIO_new(const(BIO_METHOD)* method) @nogc nothrow;
    SSL_CTX* SSL_CTX_new(const(void)* method) @nogc nothrow;
    const(void)* TLS_server_method() @nogc nothrow;
    const(void)* TLS_client_method() @nogc nothrow;
    const(void)* OSSL_QUIC_server_method() @nogc nothrow;
    const(void)* OSSL_QUIC_client_method() @nogc nothrow;
    void SSL_CTX_set_verify(SSL_CTX* ctx, int mode, void* callback) @nogc nothrow;
    int  SSL_CTX_set_default_verify_paths(SSL_CTX* ctx) @nogc nothrow;
    int  SSL_CTX_use_certificate_chain_file(SSL_CTX* ctx, const(char)* file) @nogc nothrow;
    int  SSL_CTX_use_PrivateKey_file(SSL_CTX* ctx, const(char)* file, int type) @nogc nothrow;
    int  SSL_CTX_check_private_key(const(SSL_CTX)* ctx) @nogc nothrow;
    int  SSL_CTX_set_alpn_protos(SSL_CTX* ctx, const(ubyte)* protos, uint protos_len) @nogc nothrow;
    void SSL_CTX_set_alpn_select_cb(SSL_CTX* ctx, void* cb, void* arg) @nogc nothrow;
    long SSL_CTX_set_options(SSL_CTX* ctx, long options) @nogc nothrow;
    void SSL_CTX_set_info_callback(SSL_CTX* ctx, void* cb) @nogc nothrow;
    ulong ERR_get_error() @nogc nothrow;
    void  ERR_error_string_n(ulong e, char* buf, size_t len) @nogc nothrow;

    // SSL constants
    enum int SSL_RECEIVED_SHUTDOWN = 2;
    enum int SSL_SENT_SHUTDOWN     = 1;
    enum int SSL_ERROR_WANT_READ   = 2;
    enum int SSL_ERROR_WANT_WRITE  = 3;
    enum int SSL_ERROR_ZERO_RETURN = 6;
    enum int SSL_ERROR_SYSCALL     = 5;
    enum int SSL_CB_ALERT          = 0x4000;
    enum int SSL_CB_READ           = 0x01;
    enum int SSL_FILETYPE_PEM      = 1;
    enum int SSL_VERIFY_NONE       = 0;
    enum int SSL_VERIFY_PEER       = 1;
    enum int SSL_VERIFY_FAIL_IF_NO_PEER_CERT = 8;
    enum long SSL_OP_ENABLE_KTLS   = 0x10_0000;
    enum long SSL_MODE_ENABLE_PARTIAL_WRITE  = 0x00000001L;
    enum long SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER = 0x00000002L;
    enum int BIO_NOCLOSE  = 0;
    enum int BIO_CTRL_DGRAM_SET_CONNECTED = 32;
    enum int OPENSSL_NPN_NEGOTIATED = 1;
}

// POSIX addrinfo / socket
import core.sys.posix.sys.socket : sockaddr, sockaddr_in, sockaddr_in6, sockaddr_storage,
                                    socklen_t, AF_INET, AF_INET6, SOCK_STREAM, SOCK_DGRAM,
                                    SOL_SOCKET, SO_ERROR, SO_REUSEADDR, SO_BROADCAST,
                                    IPPROTO_TCP, IPPROTO_UDP, IPPROTO_IP,
                                    listen, bind, accept, connect, shutdown,
                                    send, recv, sendto, recvfrom, setsockopt, getsockopt;
import core.sys.posix.netinet.in_ : INADDR_ANY, ntohs, htonl, inet_addr, htons;
import core.stdc.errno : EINPROGRESS, EWOULDBLOCK, EAGAIN, EINTR, ETIMEDOUT;
import core.sys.posix.netdb : addrinfo, getaddrinfo, freeaddrinfo, AI_ADDRCONFIG, AI_NUMERICHOST,
                               AI_PASSIVE, PF_UNSPEC;
import core.sys.posix.netinet.tcp : TCP_NODELAY;
import core.sys.posix.sys.select : select, fd_set, FD_ZERO, FD_SET, timeval;
import core.sys.posix.sys.ioctl : ioctl, FIONREAD, FIONBIO;
import core.stdc.string : inet_ntoa = inet_ntoa;

// PORT-NOTE: leverage import uses the platform default leverager
import io.base.leverage;

// PORT-NOTE: std::system("openssl ...") for certificate generation is dropped in @nogc build.
// generate_certificate() stub documented with PORT-NOTE.

// SSL auto-initializer — module constructor
// PORT-NOTE: C++ used a static SSLAutoInitializer with throw on failure.
// D uses a module constructor; failures are logged to stderr (no throw).
extern(C) int OPENSSL_init_ssl(ulong opts, const(void)* settings) @nogc nothrow;
extern(C) int OPENSSL_init_crypto(ulong opts, const(void)* settings) @nogc nothrow;
enum ulong OPENSSL_INIT_LOAD_SSL_STRINGS    = 0x00200000L;
enum ulong OPENSSL_INIT_LOAD_CRYPTO_STRINGS = 0x00000002L;
enum ulong OPENSSL_INIT_LOAD_CONFIG         = 0x00000040L;
enum ulong OPENSSL_INIT_ADD_ALL_CIPHERS     = 0x00000004L;
enum ulong OPENSSL_INIT_ADD_ALL_DIGESTS     = 0x00000008L;

import core.stdc.stdio : fprintf, stderr;
shared static this() {
    int ssl_init    = OPENSSL_init_ssl(OPENSSL_INIT_LOAD_SSL_STRINGS | OPENSSL_INIT_LOAD_CRYPTO_STRINGS, null);
    int crypto_init = OPENSSL_init_crypto(OPENSSL_INIT_LOAD_CONFIG | OPENSSL_INIT_ADD_ALL_CIPHERS | OPENSSL_INIT_ADD_ALL_DIGESTS, null);
    if (ssl_init == 0 || crypto_init == 0) {
        fprintf(stderr, "Failed to initialize OpenSSL libraries (Fatal)\n");
        import core.stdc.stdlib : abort;
        abort();
    }
    fprintf(stderr, "OpenSSL libraries initialized successfully (DevInfo)\n");
}

// from_chars helper (POSIX port of std::from_chars for uint16_t)
import core.stdc.stdlib : strtoul;

bool parse_port(const(char)[] sv, ushort* out_port) {
    if (sv.length == 0) return false;
    // PORT-NOTE: strtoul is safe in @nogc; no exception
    import core.stdc.stdlib : strtoul;
    import core.stdc.errno : errno, ERANGE;
    errno = 0;
    // Need null-terminated string; check if slice is already null-terminated
    // PORT-NOTE: use a stack buffer for small port strings
    char[8] buf;
    if (sv.length >= buf.length) return false;
    buf[0 .. sv.length] = sv[];
    buf[sv.length] = '\0';
    char* end;
    ulong val = strtoul(buf.ptr, &end, 10);
    if (end == buf.ptr || errno == ERANGE || val > ushort.max) return false;
    *out_port = cast(ushort) val;
    return true;
}

enum Protocol : ubyte { TCP = 0, UDP = 1, TLS = 2, QUIC = 3 }
enum WaitMode  : ubyte { AS_SOON_AS_ARRIVED, WAIT_FOR_WHOLE_MESSAGE }

bool is_waiting(WaitMode mode) {
    return mode == WaitMode.WAIT_FOR_WHOLE_MESSAGE;
}

// PORT-NOTE: ABI POD struct wrapping sockaddr_storage + socklen_t
struct AddressInfo {
    sockaddr_storage m_adrinf;
    socklen_t        m_sock_size;

    void set(ref const(sockaddr_storage) adrinf, ref socklen_t sock_size) {
        m_adrinf    = adrinf;
        m_sock_size = sock_size;
    }

    void set_data(ref const(sockaddr_storage) adrinf) { m_adrinf    = adrinf; }
    void set_size(ref const(socklen_t) sock_size)     { m_sock_size = sock_size; }

    ref const(sockaddr_storage) get_data() const { return m_adrinf; }
    ref sockaddr_storage        get_data()       { return m_adrinf; }
    ref const(socklen_t)        get_size() const { return m_sock_size; }
    ref socklen_t               get_size()       { return m_sock_size; }
}

enum Event : ubyte { READ = 0x1, WRITE = 0x2, EXCEPT = 0x4 }

// PORT-NOTE: Endpoint stores address as a fixed-size stack buffer to stay @nogc.
// The C++ used std::string for m_address; D port uses char[256] + length.
struct Endpoint {
    char[256] m_address_buf;
    size_t    m_address_len;
    ushort    m_port;

    this(const(char)[] address, ushort port) {
        set_address(address);
        m_port = port;
    }

    // Parse "address:port"
    this(const(char)[] address_with_port) {
        // Find last ':'
        size_t sep = size_t.max;
        foreach_reverse (i, c; address_with_port) {
            if (c == ':') { sep = i; break; }
        }
        if (sep == size_t.max) {
            fatal("SocketLib", "invalid address");
            return;
        }
        if (sep == address_with_port.length - 1) {
            fatal("SocketLib", "missing port");
            return;
        }

        set_address(address_with_port[0 .. sep]);

        ushort parsed_port;
        if (!parse_port(address_with_port[sep + 1 .. $], &parsed_port)) {
            fatal("SocketLib", "Failed to parse port number");
            return;
        }
        m_port = parsed_port;
    }

    // Construct from sockaddr*
    this(const(sockaddr)* address) {
        if (address is null) {
            fatal("SocketLib", "Null address passed to Endpoint");
            return;
        }

        if (address.sa_family == AF_INET) {
            auto addr4 = cast(const(sockaddr_in)*) address;
            char[INET_ADDRSTRLEN] buf;
            // PORT-NOTE: inet_ntoa used; POSIX only — Win32 path uses inet_ntop_compat
            const(char)* s = inet_ntoa(addr4.sin_addr);
            if (s !is null) {
                import core.stdc.string : strlen;
                set_address(s[0 .. strlen(s)]);
            }
            m_port = ntohs(addr4.sin_port);
        } else if (address.sa_family == AF_INET6) {
            auto addr6 = cast(const(sockaddr_in6)*) address;
            char[INET6_ADDRSTRLEN] buf;
            // PORT-NOTE: inet_ntop extern(C) from arpa/inet.h
            extern(C) const(char)* inet_ntop(int af, const(void)* src, char* dst, uint size) @nogc nothrow;
            const(char)* s = inet_ntop(AF_INET6, &addr6.sin6_addr, buf.ptr, cast(uint) buf.length);
            if (s !is null) {
                import core.stdc.string : strlen;
                set_address(s[0 .. strlen(s)]);
            }
            m_port = ntohs(addr6.sin6_port);
        } else {
            fatal("SocketLib", "Unsupported address family");
        }

        if (m_address_len == 0) {
            fatal("SocketLib", "Failed to convert address to string");
        }
    }

    // PORT-NOTE: to_string requires a caller-supplied buffer
    size_t to_string(char[] out_buf) const {
        // Write "address:port\0" into out_buf
        size_t pos = 0;
        foreach (c; get_address()) {
            if (pos >= out_buf.length - 1) break;
            out_buf[pos++] = c;
        }
        if (pos < out_buf.length - 1) out_buf[pos++] = ':';
        // Write port digits
        ushort p = m_port;
        char[6] digits;
        size_t di = 0;
        if (p == 0) { digits[di++] = '0'; }
        else {
            while (p > 0) { digits[di++] = cast(char)('0' + p % 10); p /= 10; }
            // reverse
            for (size_t lo = 0, hi = di - 1; lo < hi; lo++, hi--) {
                char tmp = digits[lo]; digits[lo] = digits[hi]; digits[hi] = tmp;
            }
        }
        foreach (c; digits[0 .. di]) {
            if (pos >= out_buf.length - 1) break;
            out_buf[pos++] = c;
        }
        if (pos < out_buf.length) out_buf[pos] = '\0';
        return pos;
    }

    const(char)[] get_address() const {
        return m_address_buf[0 .. m_address_len];
    }

    const(ushort) get_port() const { return m_port; }

    void set_address(const(char)[] addr) {
        size_t len = addr.length < m_address_buf.length ? addr.length : m_address_buf.length - 1;
        m_address_buf[0 .. len] = addr[0 .. len];
        m_address_buf[len] = '\0';
        m_address_len = len;
    }

  private:
    enum int INET_ADDRSTRLEN  = 16;
    enum int INET6_ADDRSTRLEN = 46;
}

enum VALUES : byte {
    ERRORED                       = 0x0,
    VALID                         = 0x1,
    CLEANLY_DISCONNECTED          = 0x2,
    NON_BLOCKING_WOULD_HAVE_BLOCKED = 0x3,
    TIMED_OUT                     = 0x4,
}

// PORT-NOTE: value wrapper struct
struct SocketStatus {
    VALUES m_value = VALUES.ERRORED;

    this(bool is_valid) {
        m_value = is_valid ? VALUES.VALID : VALUES.ERRORED;
    }
    this(VALUES value) {
        m_value = value;
    }

    bool opCast(T : bool)() const {
        return cast(byte) m_value > 0;
    }
    byte get_value() const { return cast(byte) m_value; }
    bool opEquals(VALUES val) const { return m_value == val; }

    bool is_valid()              const { return m_value == VALUES.VALID; }
    bool is_errored()            const { return m_value == VALUES.ERRORED; }
    bool is_cleanly_disconnected() const { return m_value == VALUES.CLEANLY_DISCONNECTED; }
    bool would_have_blocked()    const { return m_value == VALUES.NON_BLOCKING_WOULD_HAVE_BLOCKED; }
    bool is_timed_out()          const { return m_value == VALUES.TIMED_OUT; }

    ref const(VALUES) get_status() const { return m_value; }
    ref VALUES        get_status()       { return m_value; }
}

// PORT-NOTE: Socket!Protocol is a D template class.
// C++ template bool params (Protocol, RootSocket) map to template value params.
// Async callbacks that used std::move_only_function are @nogc fn+ctx pairs.
// Fields that used std::string are char[256]+length; std::vector<unsigned char> is ubyte[].
// std::optional<std::reference_wrapper<Leverager>> → raw pointer (null = none).

class Socket(Protocol proto, bool RootSocket = false) {
  public:
    // Default constructor
    this() {
        m_socket = INVALID_SOCKET;
        m_ssl    = null;
        m_ssl_ctx = null;
        m_bio    = null;
        m_address_info_result   = null;
        m_socket_address_info   = null;
        m_ktls_tx = false;
        m_ktls_rx = false;
        m_leverager = null;
    }

    // Endpoint + optional leverager
    this(Endpoint endpoint, DefaultLeverager leverager = null) {
        m_socket = INVALID_SOCKET;
        m_ssl    = null;
        m_ssl_ctx = null;
        m_bio    = null;
        m_endpoint = endpoint;
        m_address_info_result  = null;
        m_socket_address_info  = null;
        m_ktls_tx = false;
        m_ktls_rx = false;
        m_leverager = leverager;
        init_address_info();

        // Don't use AI_ADDRCONFIG if connecting to loopback
        // See https://fedoraproject.org/wiki/QA/Networking/NameResolution/ADDRCONFIG
        const(char)[] ADDRESS = m_endpoint.get_address();
        if (slice_eq(ADDRESS, "localhost") || slice_eq(ADDRESS, "127.0.0.1") || slice_eq(ADDRESS, "::1")) {
            m_address_info_hint.ai_flags = 0;
        }

        // PORT-NOTE: getaddrinfo allocates; stored in m_address_info_result (freed in ~this)
        char[8] port_str;
        format_port(m_endpoint.get_port(), port_str);
        if (getaddrinfo(ADDRESS.ptr, port_str.ptr, &m_address_info_hint, &m_address_info_result) != 0) {
            error("SocketLib", "resolve failed");
        }

        for (auto addr = m_address_info_result; addr !is null; addr = addr.ai_next) {
            m_socket = .socket(addr.ai_family, addr.ai_socktype, addr.ai_protocol);
            if (m_socket != INVALID_SOCKET) {
                m_socket_address_info = addr;
                break;
            }
        }

        if (m_socket == INVALID_SOCKET) {
            error("SocketLib", "socket create failed");
        } else {
            debug_("SocketLib", "socket created");
        }
    }

    // native fd + endpoint
    this(SOCKET nativ, Endpoint endpoint, DefaultLeverager leverager = null) {
        m_socket  = nativ;
        m_ssl     = null;
        m_ssl_ctx = null;
        m_bio     = null;
        m_endpoint = endpoint;
        m_address_info_result  = null;
        m_socket_address_info  = null;
        m_ktls_tx = false;
        m_ktls_rx = false;
        m_leverager = leverager;
        init_address_info();
        debug_("SocketLib", "socket accepted");
    }

    // native fd + SSL + endpoint
    this(SOCKET nativ, SSL* ssl, Endpoint endpoint, DefaultLeverager leverager = null) {
        m_socket  = nativ;
        m_ssl     = ssl;
        m_ssl_ctx = null;
        m_bio     = null;
        m_endpoint = endpoint;
        m_address_info_result  = null;
        m_socket_address_info  = null;
        m_ktls_tx = false;
        m_ktls_rx = false;
        m_leverager = leverager;
        init_address_info();
        debug_("SocketLib", "socket TLS accepted");
    }

    // native fd + SSL (QUIC, no endpoint)
    this(SOCKET nativ, SSL* ssl, DefaultLeverager leverager = null) {
        m_socket  = nativ;
        m_ssl     = ssl;
        m_ssl_ctx = null;
        m_bio     = null;
        m_address_info_result  = null;
        m_socket_address_info  = null;
        m_ktls_tx = false;
        m_ktls_rx = false;
        m_leverager = leverager;
        init_address_info();
        debug_("SocketLib", "socket QUIC");
    }

    ~this() {
        static if (proto != Protocol.QUIC || RootSocket) {
            sync_close();
            if (m_address_info_result !is null) {
                freeaddrinfo(m_address_info_result);
            }
        }
    }

    void set_non_blocking(bool non_blocking = true) const {
        set_non_blocking_impl(m_socket, non_blocking);
    }

    void set_reuse_address(bool reuse = true) const {
        int optval = reuse ? 1 : 0;
        if (setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, cast(char*) &optval, optval.sizeof) != 0) {
            error("SocketLib", "Failed to set SO_REUSEADDR");
        }
        debug_("SocketLib", "SO_REUSEADDR set");
    }

    void set_broadcast(bool broadcast = true) const {
        int optval = broadcast ? 1 : 0;
        if (setsockopt(m_socket, SOL_SOCKET, SO_BROADCAST, cast(char*) &optval, optval.sizeof) != 0) {
            error("SocketLib", "Failed to set SO_BROADCAST");
        }
        debug_("SocketLib", "SO_BROADCAST set");
    }

    void set_tcp_no_delay(bool no_delay = true) const {
        static if (proto == Protocol.TCP || proto == Protocol.TLS) {
            int optval = no_delay ? 1 : 0;
            if (setsockopt(m_socket, IPPROTO_TCP, TCP_NODELAY, cast(char*) &optval, optval.sizeof) != 0) {
                error("SocketLib", "Failed to set TCP_NODELAY");
            }
            debug_("SocketLib", "TCP_NODELAY set");
        }
    }

    SocketStatus get_status() const {
        int err = 0;
        socklen_t len = err.sizeof;
        if (getsockopt(m_socket, SOL_SOCKET, SO_ERROR, cast(char*) &err, &len) != 0) {
            error("SocketLib", "Failed to get socket status");
        }
        debug_("SocketLib", "socket status checked");
        return SocketStatus(err == 0);
    }

    // PORT-NOTE: generate_certificate used std::filesystem and std::system (not @nogc).
    // Dropped entirely in @nogc D port. Caller must generate certs externally.
    void generate_certificate(const(char)[] cert_path, const(char)[] key_path) {
        // PORT-NOTE: std::system("openssl ...") dropped — @nogc incompatible.
        // Use external tooling (openssl CLI) to generate certificates.
        fatal("SocketLib", "generate_certificate: not supported in @nogc build — use external openssl CLI");
    }

    bool load_certificate(const(char)[] cert_file, const(char)[] key_file) {
        static if (proto == Protocol.TLS || proto == Protocol.QUIC) {
            if (m_ssl_ctx is null) {
                fatal("SocketLib", "SSL context is not initialized, cannot load certificate");
            }
            if (SSL_CTX_use_certificate_chain_file(m_ssl_ctx, cert_file.ptr) != 1) {
                char[256] err_buf;
                ERR_error_string_n(ERR_get_error(), err_buf.ptr, err_buf.length);
                error("SocketLib", "Failed to load chain");
                return false;
            }
            if (SSL_CTX_use_PrivateKey_file(m_ssl_ctx, key_file.ptr, SSL_FILETYPE_PEM) != 1) {
                char[256] err_buf;
                ERR_error_string_n(ERR_get_error(), err_buf.ptr, err_buf.length);
                error("SocketLib", "Failed to load key");
                return false;
            }
            if (SSL_CTX_check_private_key(m_ssl_ctx) != 1) {
                char[256] err_buf;
                ERR_error_string_n(ERR_get_error(), err_buf.ptr, err_buf.length);
                error("SocketLib", "Key/Cert Mismatch");
                return false;
            }
            debug_("SocketLib", "cert+key loaded");
            return true;
        } else {
            fatal("SocketLib", "Loading certificates is only supported for TLS and QUIC protocols");
            return false;
        }
    }

    void bind_(bool allow_unauthorized = false) {  // PORT-NOTE: renamed from bind (D keyword)
        if (m_socket_address_info is null) {
            error("SocketLib", "No valid address info to bind to");
        }

        if (.bind(m_socket, m_socket_address_info.ai_addr,
                  cast(socklen_t) m_socket_address_info.ai_addrlen) == SOCKET_ERROR) {
            error("SocketLib", "Failed to bind socket");
        }

        static if (proto == Protocol.TLS) {
            m_ssl_ctx = SSL_CTX_new(TLS_server_method());
            if (m_ssl_ctx is null) {
                error("SocketLib", "Failed to create SSL context");
            }

            // SSL_CTX_set_info_callback: lambda not @nogc — omitted
            // PORT-NOTE: info callback dropped; use debug logging if needed

            if (m_alpn_wire_format_len > 0) {
                debug_("SocketLib", "ALPN protos set");
                // PORT-NOTE: ALPN select callback uses a lambda capture in C++;
                // dropped here — see improvement idea in IMPROVEMENTS.md
                // SSL_CTX_set_alpn_select_cb(m_ssl_ctx, null, null);
            }

            if (allow_unauthorized) {
                SSL_CTX_set_verify(m_ssl_ctx, SSL_VERIFY_NONE, null);
            } else {
                SSL_CTX_set_verify(m_ssl_ctx, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT, null);
            }
            SSL_CTX_set_default_verify_paths(m_ssl_ctx);

        } else static if (proto == Protocol.QUIC) {
            add_quic_bio();
        }

        debug_("SocketLib", "socket bound");
    }

    // PORT-NOTE: join() (multicast) omitted; requires dynamic alloc (std::to_string port).
    // Stub left for Run 2.
    void join(ref const(Endpoint) endpoint, const(char)[] group = "") {
        static if (proto != Protocol.UDP) {
            fatal("SocketLib", "Joining multicast groups is only supported for UDP protocol");
        }
        // PORT-NOTE: multicast join stub — full impl in Run 2
        fatal("SocketLib", "join() not yet implemented in @nogc port");
    }

    void listen() {
        static if (proto == Protocol.TCP || proto == Protocol.TLS) {
            if (.listen(m_socket, 128 /*SOMAXCONN*/) == SOCKET_ERROR) {
                error("SocketLib", "Failed to listen on socket");
            }
            debug_("SocketLib", "socket listening");
        } else static if (proto == Protocol.QUIC) {
            if (m_bio is null) {
                error("SocketLib", "BIO must be initialized before listening for QUIC");
            }

            m_ssl_ctx = SSL_CTX_new(OSSL_QUIC_server_method());
            if (m_ssl_ctx is null) {
                error("SocketLib", "Failed to create SSL context for QUIC");
            }

            m_ssl = SSL_new(m_ssl_ctx);
            if (m_ssl is null) {
                error("SocketLib", "Failed to create SSL object for QUIC");
            }

            SSL_set_bio(m_ssl, m_bio, m_bio);
            SSL_set_accept_state(m_ssl);

            debug_("SocketLib", "socket listening (quic)");
        } else {
            fatal("SocketLib", "Listen is only supported for TCP, TLS, and QUIC protocols");
        }
    }

    SocketStatus select_(int event_mask, ulong timeout_ms) const {
        fd_set readfds, writefds, exceptfds;
        fd_set* p_read   = null;
        fd_set* p_write  = null;
        fd_set* p_except = null;

        if ((event_mask & cast(int) Event.READ) != 0) {
            FD_ZERO(&readfds);
            FD_SET(m_socket, &readfds);
            p_read = &readfds;
        }
        if ((event_mask & cast(int) Event.WRITE) != 0) {
            FD_ZERO(&writefds);
            FD_SET(m_socket, &writefds);
            p_write = &writefds;
        }
        if ((event_mask & cast(int) Event.EXCEPT) != 0) {
            FD_ZERO(&exceptfds);
            FD_SET(m_socket, &exceptfds);
            p_except = &exceptfds;
        }

        timeval tv;
        tv.tv_sec  = cast(typeof(tv.tv_sec))  (timeout_ms / 1000);
        tv.tv_usec = cast(typeof(tv.tv_usec)) ((timeout_ms % 1000) * 1000);

        int result = .select(cast(int)(m_socket + 1), p_read, p_write, p_except, &tv);
        if (result < 0) {
            int ERR = get_error_code();
            if (ERR == EINTR || ERR == EAGAIN || ERR == EWOULDBLOCK) {
                warning("SocketLib", "socket select blocked/interrupted");
                return SocketStatus(VALUES.NON_BLOCKING_WOULD_HAVE_BLOCKED);
            }
            warning("SocketLib", "Critical failure in select() syscall");
            return SocketStatus(VALUES.ERRORED);
        } else if (result == 0) {
            warning("SocketLib", "socket select timeout");
            return SocketStatus(VALUES.TIMED_OUT);
        }

        debug_("SocketLib", "socket select ready");
        return SocketStatus(VALUES.VALID);
    }

    SocketStatus sync_connect(ulong timeout = 0) {
        static if (proto == Protocol.TCP || proto == Protocol.TLS || proto == Protocol.QUIC) {
            auto current = m_socket_address_info;
            if (connect_addr!(false)(current, timeout) != SocketStatus(VALUES.VALID)) {
                for (auto addr = m_address_info_result.ai_next; addr !is null; addr = addr.ai_next) {
                    if (addr == current) continue;
                    if (connect_addr!(true)(addr, timeout) == SocketStatus(VALUES.VALID)) break;
                }
            }

            if (m_socket == INVALID_SOCKET) {
                error("SocketLib", "Failed to connect to any resolved address");
            }

            static if (proto == Protocol.TLS) {
                if (!setup_tls()) return SocketStatus(VALUES.ERRORED);
            } else static if (proto == Protocol.QUIC) {
                add_quic_bio();
                if (!setup_quic()) return SocketStatus(VALUES.ERRORED);
            }

            debug_("SocketLib", "socket connected");
            return SocketStatus(VALUES.VALID);
        } else {
            fatal("SocketLib", "Connect is only supported for TCP, TLS, QUIC protocols");
            return SocketStatus(VALUES.ERRORED);
        }
    }

    // PORT-NOTE: async_connect uses fn+ctx pair instead of std::move_only_function.
    alias ConnectCallback = void function(SocketStatus, void*) @nogc nothrow;

    void async_connect(ConnectCallback callback, void* ctx, ubyte iflags = 0) {
        static if (proto == Protocol.TCP || proto == Protocol.TLS || proto == Protocol.QUIC) {
            if (m_leverager !is null) {
                auto current = (m_socket_address_info !is null) ? m_socket_address_info : m_address_info_result;
                set_non_blocking(true);

                // PORT-NOTE: The C++ used a shared_ptr<function<void(int)>> for recursive
                // async retry. D @nogc cannot do heap lambda captures; this is a stub.
                // Full retry loop deferred to Run 2.
                m_leverager.connect(m_socket, current.ai_addr,
                    cast(socklen_t) current.ai_addrlen,
                    completion_callback(null, null), iflags);  // PORT-NOTE: stub
                callback(SocketStatus(VALUES.ERRORED), ctx);
            } else {
                fatal("SocketLib", "m_leverager is not set so async function calls cannot be used");
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
    SocketStatus sync_handshake(bool* wait_for_write = null) {
        static if (proto == Protocol.TLS || proto == Protocol.QUIC) {
            if (m_ssl is null) {
                warning("SocketLib", "SSL object is not initialized for handshake");
                return SocketStatus(VALUES.ERRORED);
            }

            int ret = SSL_do_handshake(m_ssl);
            if (ret == 1) {
                static if (proto == Protocol.TLS) {
                    m_ktls_tx = BIO_get_ktls_send(SSL_get_wbio(m_ssl)) != 0;
                    m_ktls_rx = BIO_get_ktls_recv(SSL_get_rbio(m_ssl)) != 0;
                }
                debug_("SocketLib", "socket handshake done");
                return SocketStatus(VALUES.VALID);
            }

            int err = SSL_get_error(m_ssl, ret);
            if (err == SSL_ERROR_WANT_READ) {
                if (wait_for_write !is null) *wait_for_write = false;
                debug_("SocketLib", "socket handshake wait-read");
                return SocketStatus(VALUES.NON_BLOCKING_WOULD_HAVE_BLOCKED);
            }
            if (err == SSL_ERROR_WANT_WRITE) {
                if (wait_for_write !is null) *wait_for_write = true;
                debug_("SocketLib", "socket handshake wait-write");
                return SocketStatus(VALUES.NON_BLOCKING_WOULD_HAVE_BLOCKED);
            }

            return SocketStatus(VALUES.ERRORED);
        } else {
            fatal("SocketLib", "Socket is a plain TCP socket, no handshake needed");
            return SocketStatus(VALUES.ERRORED);
        }
    }

    // PORT-NOTE: async_handshake stub — recursive fn+ctx retry deferred to Run 2
    alias HandshakeCallback = void function(SocketStatus, void*) @nogc nothrow;

    void async_handshake(HandshakeCallback callback, void* ctx, ubyte iflags = 0) {
        static if (proto == Protocol.TLS || proto == Protocol.QUIC) {
            // PORT-NOTE: recursive retry via completion_callback deferred to Run 2
            callback(SocketStatus(VALUES.ERRORED), ctx);
        } else {
            fatal("SocketLib", "Socket is a plain TCP socket, no handshake needed");
        }
    }

    bool is_handshake_done() const {
        static if (proto == Protocol.TCP || proto == Protocol.QUIC) {
            return (m_ssl !is null) && (SSL_is_init_finished(m_ssl) != 0);
        } else {
            return true;
        }
    }

    Socket!(proto) sync_accept() const {
        static if (proto == Protocol.TCP || proto == Protocol.TLS) {
            sockaddr_storage client_addr;
            socklen_t addr_len = sockaddr_storage.sizeof;

            SOCKET client_fd = cast(SOCKET) .accept(m_socket,
                cast(sockaddr*) &client_addr, &addr_len);
            if (client_fd == INVALID_SOCKET) {
                int ERR = get_error_code();
                if (ERR == EWOULDBLOCK || ERR == EAGAIN || ERR == EINTR) {
                    debug_("SocketLib", "socket accept would block");
                    return new Socket!proto();
                }
                error("SocketLib", "Critical failure in accept() syscall");
            }

            static if (proto == Protocol.TLS) {
                SSL* client_ssl = SSL_new(m_ssl_ctx);
                if (client_ssl is null) {
                    closesocket(client_fd);
                    error("SocketLib", "Failed to create SSL object for accepted connection");
                }
                if (SSL_set_fd(client_ssl, client_fd) == 0) {
                    SSL_free(client_ssl);
                    closesocket(client_fd);
                    error("SocketLib", "Failed to associate SSL object with accepted socket");
                }
                SSL_set_accept_state(client_ssl);

                auto endpoint = Endpoint(cast(const(sockaddr)*) &client_addr);
                debug_("SocketLib", "socket accepted TLS");
                return new Socket!proto(client_fd, client_ssl, endpoint);
            } else {
                auto endpoint = Endpoint(cast(const(sockaddr)*) &client_addr);
                debug_("SocketLib", "socket accepted TCP");
                return new Socket!proto(client_fd, endpoint);
            }
        } else static if (proto == Protocol.QUIC) {
            if (m_ssl is null) {
                error("SocketLib", "SSL object is not initialized for QUIC accept");
            }
            SSL* client_ssl = SSL_accept_connection(m_ssl, 0);
            if (client_ssl is null) {
                error("SocketLib", "failed to accept new QUIC connection");
            }
            debug_("SocketLib", "socket accepted QUIC connection");
            return new Socket!proto(m_socket, client_ssl);
        } else {
            fatal("SocketLib", "Accept is only supported for TCP, TLS and QUIC protocols");
            return new Socket!proto();
        }
    }

    // PORT-NOTE: async_accept stub — GC-free shared-state retry deferred to Run 2
    alias AcceptCallback = void function(Socket!proto, void*) @nogc nothrow;

    void async_accept(AcceptCallback callback, void* ctx, ubyte iflags = 0) {
        static if (proto == Protocol.TCP || proto == Protocol.TLS) {
            if (m_leverager !is null) {
                // PORT-NOTE: full async accept deferred to Run 2
                callback(new Socket!proto(), ctx);
            } else {
                fatal("SocketLib", "m_leverager is not set so async function calls cannot be used");
            }
        } else {
            fatal("SocketLib", "Accept is only supported for TCP, TLS and QUIC protocols");
        }
    }

    // sync_send: returns (bytes_sent, SocketStatus)
    struct SendResult { size_t bytes; SocketStatus status; }

    SendResult sync_send(const(ubyte)* buffer, size_t length, AddressInfo* addr = null) const {
        size_t sent_bytes;
        int ssl_call_result;

        static if (proto == Protocol.TCP) {
            import core.sys.posix.sys.socket : send;
            sent_bytes = .send(m_socket, cast(const(char)*) buffer, cast(buffsize_t) length, 0);
        } else static if (proto == Protocol.TLS) {
            if (m_ktls_tx) {
                sent_bytes = .send(m_socket, cast(const(char)*) buffer, cast(buffsize_t) length, 0);
            } else {
                ssl_call_result = SSL_write_ex(m_ssl, buffer, length, &sent_bytes);
            }
        } else static if (proto == Protocol.QUIC) {
            ssl_call_result = SSL_write_ex(m_ssl, buffer, length, &sent_bytes);
        } else static if (proto == Protocol.UDP) {
            if (addr !is null) {
                sent_bytes = .sendto(m_socket, cast(const(char)*) buffer, cast(buffsize_t) length, 0,
                    cast(const(sockaddr)*) &addr.get_data(), addr.get_size());
            } else {
                sent_bytes = .sendto(m_socket, cast(const(char)*) buffer, cast(buffsize_t) length, 0,
                    m_socket_address_info.ai_addr,
                    cast(socklen_t) m_socket_address_info.ai_addrlen);
            }
        }

        if (!m_ktls_tx) {
            static if (proto == Protocol.TLS || proto == Protocol.QUIC) {
                if (ssl_call_result <= 0) {
                    return handle_ssl_send_error(ssl_call_result);
                }
            }
        } else {
            if (cast(long) sent_bytes < 0) {
                int ERR = get_error_code();
                if (ERR == EWOULDBLOCK) {
                    warning("SocketLib", "Send would have blocked");
                    return SendResult(0, SocketStatus(VALUES.NON_BLOCKING_WOULD_HAVE_BLOCKED));
                }
                warning("SocketLib", "Critical failure in send()");
                return SendResult(0, SocketStatus(VALUES.ERRORED));
            }
        }

        debug_("SocketLib", "socket sent bytes");
        return SendResult(sent_bytes, SocketStatus(VALUES.VALID));
    }

    // PORT-NOTE: async_send stub — full impl in Run 2
    alias SendCallback = void function(size_t, SocketStatus, void*) @nogc nothrow;

    void async_send(const(ubyte)* buffer, size_t length, SendCallback callback, void* ctx,
                    AddressInfo* addr = null, ubyte iflags = 0) {
        if (m_leverager !is null) {
            // PORT-NOTE: full async send deferred to Run 2
            callback(0, SocketStatus(VALUES.ERRORED), ctx);
        } else {
            fatal("SocketLib", "m_leverager is not set so async function calls cannot be used");
        }
    }

    // sync_receive: returns (bytes_received, SocketStatus)
    struct RecvResult { size_t bytes; SocketStatus status; }

    RecvResult sync_receive(ubyte* out_buf, size_t length, size_t start_offset = 0, AddressInfo* addr = null) {
        if (start_offset > length) {
            error("SocketLib", "Start offset is greater than the total buffer length");
            return RecvResult(0, SocketStatus(VALUES.ERRORED));
        }
        ubyte* out_ = out_buf + start_offset;
        size_t max_length = length - start_offset;

        size_t received_bytes;
        int ssl_call_result;

        static if (proto == Protocol.TCP) {
            received_bytes = .recv(m_socket, cast(char*) out_, cast(buffsize_t) max_length, 0);
        } else static if (proto == Protocol.TLS) {
            if (m_ktls_rx) {
                received_bytes = .recv(m_socket, cast(char*) out_, cast(buffsize_t) max_length, 0);
            } else {
                ssl_call_result = SSL_read_ex(m_ssl, out_, max_length, &received_bytes);
            }
        } else static if (proto == Protocol.QUIC) {
            ssl_call_result = SSL_read_ex(m_ssl, out_, max_length, &received_bytes);
        } else static if (proto == Protocol.UDP) {
            m_socket_input_buffer_length = m_socket_input_buffer.sizeof;
            received_bytes = .recvfrom(m_socket, cast(char*) out_, cast(buffsize_t) max_length, 0,
                cast(sockaddr*) &m_socket_input_buffer, &m_socket_input_buffer_length);
            if (addr !is null) {
                addr.set(m_socket_input_buffer, m_socket_input_buffer_length);
            }
        }

        if (!m_ktls_rx) {
            static if (proto == Protocol.TLS || proto == Protocol.QUIC) {
                if (ssl_call_result <= 0) {
                    return handle_ssl_recv_error(ssl_call_result);
                }
            }
        } else {
            if (cast(long) received_bytes < 0) {
                int ERR = get_error_code();
                if (ERR == EWOULDBLOCK || ERR == EAGAIN) {
                    warning("SocketLib", "Receive would have blocked");
                    return RecvResult(0, SocketStatus(VALUES.NON_BLOCKING_WOULD_HAVE_BLOCKED));
                }
                warning("SocketLib", "Critical failure in recv()");
                return RecvResult(0, SocketStatus(VALUES.ERRORED));
            }
            debug_("SocketLib", "socket cleanly disconnected");
            return RecvResult(0, SocketStatus(VALUES.CLEANLY_DISCONNECTED));
        }

        debug_("SocketLib", "socket received bytes");
        return RecvResult(received_bytes, SocketStatus(VALUES.VALID));
    }

    // PORT-NOTE: async_receive stub — full impl in Run 2
    alias RecvCallback = void function(size_t, SocketStatus, void*) @nogc nothrow;

    void async_receive(ubyte* out_buf, size_t length, RecvCallback callback, void* ctx, ubyte iflags = 0) {
        if (m_leverager !is null) {
            // PORT-NOTE: full async receive deferred to Run 2
            callback(0, SocketStatus(VALUES.ERRORED), ctx);
        } else {
            fatal("SocketLib", "m_leverager is not set so async function calls cannot be used");
        }
    }

    void sync_close() {
        if (m_socket != INVALID_SOCKET) {
            static if (proto == Protocol.TLS || proto == Protocol.QUIC) {
                if (m_ssl !is null) {
                    SSL_set_shutdown(m_ssl, SSL_RECEIVED_SHUTDOWN | SSL_SENT_SHUTDOWN);
                    SSL_shutdown(m_ssl);
                    SSL_free(m_ssl);
                    m_ssl = null;
                }
                if (m_ssl_ctx !is null) {
                    SSL_CTX_free(m_ssl_ctx);
                    m_ssl_ctx = null;
                }
                static if (proto == Protocol.QUIC) {
                    if (m_bio !is null) {
                        BIO_free(m_bio);
                        m_bio = null;
                    }
                }
            }
            static if (proto != Protocol.QUIC || RootSocket) {
                if (closesocket(m_socket) == SOCKET_ERROR) {
                    error("SocketLib", "Socket failed to close");
                }
            }

            m_socket = INVALID_SOCKET;
            debug_("SocketLib", "socket closed");
        }
    }

    // PORT-NOTE: async_close stub — full impl in Run 2
    alias CloseCallback = void function(bool, void*) @nogc nothrow;

    void async_close(CloseCallback callback, void* ctx, ubyte iflags = 0) {
        if (m_socket != INVALID_SOCKET) {
            static if (proto == Protocol.TLS || proto == Protocol.QUIC) {
                if (m_ssl !is null) {
                    SSL_set_shutdown(m_ssl, SSL_RECEIVED_SHUTDOWN | SSL_SENT_SHUTDOWN);
                    SSL_shutdown(m_ssl);
                    SSL_free(m_ssl);
                    m_ssl = null;
                }
                if (m_ssl_ctx !is null) {
                    SSL_CTX_free(m_ssl_ctx);
                    m_ssl_ctx = null;
                }
                static if (proto == Protocol.QUIC) {
                    if (m_bio !is null) {
                        BIO_free(m_bio);
                        m_bio = null;
                    }
                }
            }
            static if (proto != Protocol.QUIC || RootSocket) {
                if (m_leverager !is null) {
                    // PORT-NOTE: async close via leverager deferred to Run 2
                    callback(false, ctx);
                }
            }
        }
    }

    void shutdown_() {  // PORT-NOTE: renamed from shutdown (D keyword)
        static if (proto != Protocol.QUIC || RootSocket) {
            if (m_socket != INVALID_SOCKET) {
                if (.shutdown(m_socket, SHUT_RDWR) == SOCKET_ERROR) {
                    error("SocketLib", "Socket failed to shutdown");
                }
                debug_("SocketLib", "socket shutdown");
            }
        } else {
            fatal("SocketLib", "Shutdown is not supported for QUIC or non-root sockets");
        }
    }

    void async_shutdown(CloseCallback callback, void* ctx, ubyte iflags = 0) {
        static if (proto != Protocol.QUIC || !RootSocket) {
            if (m_socket != INVALID_SOCKET && m_leverager !is null) {
                // PORT-NOTE: async shutdown via leverager deferred to Run 2
                callback(false, ctx);
            }
        } else {
            fatal("SocketLib", "Shutdown is not supported for QUIC or non-root sockets");
        }
    }

    ref const(Endpoint) get_endpoint() const { return m_endpoint; }
    ref Endpoint        get_endpoint()       { return m_endpoint; }

    Endpoint get_recived_endpoint() const {
        static if (proto == Protocol.TCP) {
            return get_endpoint();
        } else static if (proto == Protocol.UDP) {
            return Endpoint(cast(const(sockaddr)*) &m_socket_input_buffer);
        } else {
            return m_endpoint;
        }
    }

    size_t get_pending_bytes() const {
        ioctl_setting pending_bytes = 0;
        if (ioctl(m_socket, FIONREAD, &pending_bytes) < 0) {
            error("SocketLib", "Failed to get pending bytes");
        }
        debug_("SocketLib", "socket pending bytes checked");
        if (pending_bytes > 0) return cast(size_t) pending_bytes;
        return 0;
    }

    // PORT-NOTE: set_alpn_protos/add_alpn_proto use a fixed 512-byte wire-format buffer
    // instead of std::vector<unsigned char>.
    void set_alpn_protos(const(char[])[] protocols) {
        m_alpn_wire_format_len = 0;
        foreach (ref proto_name; protocols) {
            add_alpn_proto(proto_name);
        }
    }

    void add_alpn_proto(const(char)[] alpn) {
        if (alpn.length == 0 || alpn.length > 255) {
            error("SocketLib", "ALPN protocol name invalid or too long");
            return;
        }
        debug_("SocketLib", "ALPN proto added");
        if (m_alpn_wire_format_len + 1 + alpn.length > m_alpn_wire_format.length) {
            error("SocketLib", "ALPN wire format buffer overflow");
            return;
        }
        m_alpn_wire_format[m_alpn_wire_format_len++] = cast(ubyte) alpn.length;
        m_alpn_wire_format[m_alpn_wire_format_len .. m_alpn_wire_format_len + alpn.length] =
            cast(const(ubyte)[]) alpn;
        m_alpn_wire_format_len += alpn.length;
    }

    static Protocol get_protocol() { return proto; }

    ref SOCKET       get_fd()       { return m_socket; }
    ref const(SOCKET) get_fd() const { return m_socket; }

    bool opEquals(ref const(Socket!proto) other) const {
        return m_socket == other.m_socket;
    }

    bool is_valid() const { return m_socket != INVALID_SOCKET; }
    bool opCast(T : bool)() const { return is_valid(); }

  private:
    // SSL write error handler
    SendResult handle_ssl_send_error(int ssl_call_result) const {
        int SSL_ERR = SSL_get_error(m_ssl, ssl_call_result);
        switch (SSL_ERR) {
        case SSL_ERROR_WANT_READ:
            warning("SocketLib", "SSL write want-read would block");
            return SendResult(0, SocketStatus(VALUES.NON_BLOCKING_WOULD_HAVE_BLOCKED));
        case SSL_ERROR_WANT_WRITE:
            warning("SocketLib", "SSL write want-write would block");
            return SendResult(0, SocketStatus(VALUES.NON_BLOCKING_WOULD_HAVE_BLOCKED));
        case SSL_ERROR_ZERO_RETURN:
            debug_("SocketLib", "socket cleanly disconnected");
            return SendResult(0, SocketStatus(VALUES.CLEANLY_DISCONNECTED));
        case SSL_ERROR_SYSCALL:
            if (get_error_code() == EWOULDBLOCK) {
                warning("SocketLib", "SSL write syscall would block");
                return SendResult(0, SocketStatus(VALUES.NON_BLOCKING_WOULD_HAVE_BLOCKED));
            }
            warning("SocketLib", "SSL write syscall failure");
            return SendResult(0, SocketStatus(VALUES.ERRORED));
        default:
            warning("SocketLib", "SSL write failure");
            return SendResult(0, SocketStatus(VALUES.ERRORED));
        }
    }

    // SSL read error handler
    RecvResult handle_ssl_recv_error(int ssl_call_result) const {
        int SSL_ERR = SSL_get_error(m_ssl, ssl_call_result);
        switch (SSL_ERR) {
        case SSL_ERROR_WANT_READ:
            warning("SocketLib", "SSL read want-read would block");
            return RecvResult(0, SocketStatus(VALUES.NON_BLOCKING_WOULD_HAVE_BLOCKED));
        case SSL_ERROR_WANT_WRITE:
            warning("SocketLib", "SSL read want-write would block");
            return RecvResult(0, SocketStatus(VALUES.NON_BLOCKING_WOULD_HAVE_BLOCKED));
        case SSL_ERROR_ZERO_RETURN:
            debug_("SocketLib", "socket cleanly disconnected");
            return RecvResult(0, SocketStatus(VALUES.CLEANLY_DISCONNECTED));
        case SSL_ERROR_SYSCALL:
            int ERR = get_error_code();
            if (ERR == EWOULDBLOCK || ERR == EAGAIN) {
                warning("SocketLib", "SSL read syscall would block");
                return RecvResult(0, SocketStatus(VALUES.NON_BLOCKING_WOULD_HAVE_BLOCKED));
            }
            warning("SocketLib", "SSL syscall failure");
            return RecvResult(0, SocketStatus(VALUES.ERRORED));
        default:
            warning("SocketLib", "SSL read failure");
            return RecvResult(0, SocketStatus(VALUES.ERRORED));
        }
    }

    template!(bool CreateSocket = true)
    SocketStatus connect_addr(addrinfo* addr, ulong timeout) {
        static if (proto == Protocol.TCP || proto == Protocol.TLS) {
            static if (CreateSocket) {
                debug_("SocketLib", "socket closing to retry connect");
                sync_close();
                m_socket_address_info = null;
                m_socket = .socket(addr.ai_family, addr.ai_socktype, addr.ai_protocol);
            }

            if (m_socket == INVALID_SOCKET) {
                warning("SocketLib", "Failed to create socket for connection attempt");
                return SocketStatus(VALUES.ERRORED);
            }

            m_socket_address_info = addr;

            if (timeout > 0) set_non_blocking(true);

            int err_code = .connect(m_socket, addr.ai_addr, cast(socklen_t) addr.ai_addrlen);
            if (err_code == SOCKET_ERROR) {
                err_code = get_error_code();

                if (err_code == EINPROGRESS || err_code == EWOULDBLOCK || err_code == EAGAIN) {
                    debug_("SocketLib", "socket connect in progress");

                    timeval tv;
                    tv.tv_sec  = cast(typeof(tv.tv_sec))  (timeout / 1000);
                    tv.tv_usec = cast(typeof(tv.tv_usec)) ((timeout % 1000) * 1000);

                    fd_set writefds, exceptfds;
                    FD_ZERO(&writefds);
                    FD_SET(m_socket, &writefds);
                    FD_ZERO(&exceptfds);
                    FD_SET(m_socket, &exceptfds);

                    int sel = .select(cast(int)(m_socket + 1), null, &writefds, &exceptfds, &tv);
                    if (sel > 0) {
                        socklen_t len = err_code.sizeof;
                        if (getsockopt(m_socket, SOL_SOCKET, SO_ERROR,
                                       cast(char*) &err_code, &len) < 0) {
                            error("SocketLib", "getsockopt failed after select");
                        }
                    } else if (sel == 0) {
                        err_code = ETIMEDOUT;
                    } else {
                        err_code = get_error_code();
                    }
                }
            }

            if (timeout > 0) set_non_blocking(false);

            if (err_code != 0) {
                sync_close();
                m_socket_address_info = null;
                warning("SocketLib", "Connect attempt failed");
                return SocketStatus(VALUES.ERRORED);
            }

            debug_("SocketLib", "socket connected");
            return SocketStatus(VALUES.VALID);
        } else {
            fatal("SocketLib", "Connect is only supported for TCP and TLS protocols");
            return SocketStatus(VALUES.ERRORED);
        }
    }

    void init_address_info() {
        int sock_type;
        int iprotocol;

        static if (proto == Protocol.TCP || proto == Protocol.TLS) {
            sock_type = SOCK_STREAM;
            iprotocol = IPPROTO_TCP;
        } else static if (proto == Protocol.UDP || proto == Protocol.QUIC) {
            sock_type = SOCK_DGRAM;
            iprotocol = IPPROTO_UDP;
        }

        m_address_info_hint = addrinfo.init;
        m_address_info_hint.ai_family   = AF_UNSPEC;
        m_address_info_hint.ai_socktype = sock_type;
        m_address_info_hint.ai_protocol = iprotocol;
        m_address_info_hint.ai_flags    = AI_ADDRCONFIG;
    }

    void add_quic_bio() {
        m_bio = BIO_new(BIO_s_datagram());
        if (m_bio is null) {
            error("SocketLib", "Failed to create BIO for QUIC socket");
        }
        BIO_set_fd(m_bio, cast(int) m_socket, BIO_NOCLOSE);
        BIO_ctrl(m_bio, BIO_CTRL_DGRAM_SET_CONNECTED, 0, null);
        debug_("SocketLib", "socket QUIC BIO ready");
    }

    bool setup_tls() {
        static if (proto == Protocol.TLS) {
            m_ssl_ctx = SSL_CTX_new(TLS_client_method());
            if (m_ssl_ctx is null) {
                warning("SocketLib", "Failed to create SSL context");
                return false;
            }

            if (m_alpn_wire_format_len > 0) {
                debug_("SocketLib", "ALPN protos set for TLS socket");
                SSL_CTX_set_alpn_protos(m_ssl_ctx, m_alpn_wire_format.ptr,
                    cast(uint) m_alpn_wire_format_len);
            }

            SSL_CTX_set_options(m_ssl_ctx, SSL_OP_ENABLE_KTLS);
            SSL_CTX_set_verify(m_ssl_ctx, SSL_VERIFY_PEER, null);
            SSL_CTX_set_default_verify_paths(m_ssl_ctx);

            m_ssl = SSL_new(m_ssl_ctx);
            if (m_ssl is null) {
                warning("SocketLib", "Failed to create SSL object");
                return false;
            }

            if (SSL_set_fd(m_ssl, cast(int) m_socket) == 0) {
                warning("SocketLib", "Failed to associate SSL object with socket");
                return false;
            }

            if (SSL_set_tlsext_host_name(m_ssl, m_endpoint.m_address_buf.ptr) != 1) {
                warning("SocketLib", "Failed to set SNI hostname");
                return false;
            }

            debug_("SocketLib", "socket TLS SSL ready");
            SSL_set_connect_state(m_ssl);
            return true;
        } else {
            return false;
        }
    }

    bool setup_quic() {
        static if (proto == Protocol.QUIC) {
            add_quic_bio();

            m_ssl_ctx = SSL_CTX_new(OSSL_QUIC_client_method());
            if (m_ssl_ctx is null) {
                warning("SocketLib", "Failed to create SSL context for QUIC");
                return false;
            }

            if (m_alpn_wire_format_len > 0) {
                debug_("SocketLib", "ALPN protos set for QUIC socket");
                SSL_CTX_set_alpn_protos(m_ssl_ctx, m_alpn_wire_format.ptr,
                    cast(uint) m_alpn_wire_format_len);
            }

            SSL_CTX_set_verify(m_ssl_ctx, SSL_VERIFY_PEER, null);
            SSL_CTX_set_default_verify_paths(m_ssl_ctx);

            m_ssl = SSL_new(m_ssl_ctx);
            if (m_ssl is null) {
                warning("SocketLib", "Failed to create SSL object for QUIC");
                return false;
            }

            SSL_set_bio(m_ssl, m_bio, m_bio);

            if (SSL_set_tlsext_host_name(m_ssl, m_endpoint.m_address_buf.ptr) != 1) {
                warning("SocketLib", "Failed to set SNI hostname for QUIC");
                return false;
            }

            debug_("SocketLib", "socket QUIC SSL ready");
            SSL_set_connect_state(m_ssl);
            return true;
        } else {
            return false;
        }
    }

    // Helper: port to null-terminated string
    static void format_port(ushort port, ref char[8] buf) {
        size_t i = 0;
        if (port == 0) { buf[0] = '0'; buf[1] = '\0'; return; }
        char[6] tmp;
        size_t di = 0;
        while (port > 0) { tmp[di++] = cast(char)('0' + port % 10); port /= 10; }
        for (size_t lo = 0, hi = di - 1; lo < hi; lo++, hi--) {
            char t = tmp[lo]; tmp[lo] = tmp[hi]; tmp[hi] = t;
        }
        buf[0 .. di] = tmp[0 .. di];
        buf[di] = '\0';
    }

    // Helper: compare slice to string literal
    static bool slice_eq(const(char)[] s, string lit) {
        if (s.length != lit.length) return false;
        foreach (i, c; s) {
            if (c != lit[i]) return false;
        }
        return true;
    }

    // SSL wbio/rbio accessors (needed for kTLS detection)
    extern(C) BIO* SSL_get_wbio(const(SSL)* ssl) @nogc nothrow;
    extern(C) BIO* SSL_get_rbio(const(SSL)* ssl) @nogc nothrow;

    SOCKET           m_socket;
    SSL*             m_ssl;
    SSL_CTX*         m_ssl_ctx;
    BIO*             m_bio;
    Endpoint         m_endpoint;
    addrinfo         m_address_info_hint;
    addrinfo*        m_address_info_result;
    addrinfo*        m_socket_address_info;
    sockaddr_storage m_socket_input_buffer;
    socklen_t        m_socket_input_buffer_length;
    ubyte[512]       m_alpn_wire_format;      // PORT-NOTE: fixed-size replaces std::vector<unsigned char>
    size_t           m_alpn_wire_format_len;
    DefaultLeverager m_leverager;             // PORT-NOTE: raw pointer; null = no leverager
    bool             m_ktls_tx;
    bool             m_ktls_rx;
    OsPayload        m_os;                    // PORT-NOTE: [[no_unique_address]] → plain field
}

// Convenient aliases for the most common socket types
alias TcpSocket  = Socket!Protocol.TCP;
alias TlsSocket  = Socket!Protocol.TLS;
alias UdpSocket  = Socket!Protocol.UDP;
alias QuicSocket = Socket!Protocol.QUIC;
