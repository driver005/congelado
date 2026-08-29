module;
#include <cassert>
#include <openssl/err.h>
#include <openssl/rand.h>
export module io_quic:crypto;

import std;
import :types;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

// quic:crypto is now minimal — OpenSSL 3.6 QUIC handles all key derivation,
// HKDF, AEAD, and header protection internally via OSSL_QUIC_server_method().
// Only random byte generation and CID construction remain here.

namespace quic::crypto {

export struct CryptoError : std::runtime_error
{
    /**
     * @brief Builds a runtime_error whose message is `ctx` plus whatever OpenSSL's error queue
     * says went down — pulls the top error off the queue at construction time, so build this
     * right after the failing call, no motion in between.
     * @param ctx short label for what operation failed (e.g. "RAND_bytes").
     */
    explicit CryptoError(std::string_view ctx) :
        std::runtime_error(std::string(ctx) + ": " + ssl_error())
    {
    }

private:
    /**
     * @brief Pops the most recent OpenSSL error off the thread-local error queue and renders it
     * as a human-readable string.
     * @return the formatted OpenSSL error string.
     */
    static std::string ssl_error()
    {
        // Pop the top error off OpenSSL's queue and render it into a fixed buffer.
        char buf[256];
        ERR_error_string_n(
            ERR_get_error(), buf, sizeof(buf)
        );          // FIXME(clang-tidy): array-to-pointer decay
        return buf; // FIXME(clang-tidy): array-to-pointer decay
    }
};

export void random_bytes(std::span<std::byte> out)
{
    // RAND_bytes returns 1 on success — anything else means the CSPRNG came up short, no cap,
    // straight throw instead of handing back weak/partial randomness.
    // std::byte and unsigned char are both 1-byte character types; casting through void* is
    // well-defined (C++ [expr.static.cast]/13) and avoids reinterpret_cast at the call site.
    if (RAND_bytes(static_cast<unsigned char*>(static_cast<void*>(out.data())),
                   static_cast<int>(out.size())) != 1) {
        throw CryptoError("RAND_bytes");
    }
}

export template<std::size_t Len = CID_DEFAULT_LEN>
ConnectionId generate_cid()
{
    static_assert(Len <= CID_MAX_LEN);
    // Stamp the length up front, then fill the CID's own storage with fresh random bytes.
    ConnectionId cid;
    cid.len = static_cast<std::uint8_t>(Len);
    random_bytes(
        std::as_writable_bytes(std::span{cid.data, Len})
    );
    return cid;
}

export ConnectionId generate_cid(std::size_t len)
{
    assert(len <= CID_MAX_LEN);
    // Runtime-length sibling of the template above — same motion, just `len` isn't known
    // at compile time here.
    ConnectionId cid;
    cid.len = static_cast<std::uint8_t>(len);
    random_bytes(
        std::as_writable_bytes(std::span{cid.data, len})
    );
    return cid;
}

} // namespace quic::crypto

// TlsContext/TlsSession/Connection all need real cert files or a live SSL* from an accepted
// QUIC connection to do anything meaningful — not reproducible here. random_bytes()/
// generate_cid() are pure CSPRNG calls with no live network/handshake involved, so they're
// tested directly. CryptoError needs a genuinely-queued OpenSSL error to construct
// meaningfully; skipped as low-value to fake.
#ifdef CONGELADO_TEST
namespace quic::crypto::tests {
using namespace boost::ut;

suite<"random_bytes"> random_bytes_suite = [] {
    "fills the whole buffer and differs between calls"_test = [] {
        std::array<std::byte, 16> first{};
        std::array<std::byte, 16> second{};
        random_bytes(first);
        random_bytes(second);

        expect(not std::ranges::equal(first, second));
    };
};

suite<"generate_cid"> generate_cid_suite = [] {
    "template overload produces the requested length"_test = [] {
        auto cid = generate_cid<12>();
        expect(cid.len == 12);
    };
    "runtime-length overload produces the requested length"_test = [] {
        auto cid = generate_cid(6);
        expect(cid.len == 6);
    };
    "two generated CIDs differ"_test = [] {
        auto first = generate_cid<CID_DEFAULT_LEN>();
        auto second = generate_cid<CID_DEFAULT_LEN>();
        expect(not(first == second));
    };
};

} // namespace quic::crypto::tests
#endif
