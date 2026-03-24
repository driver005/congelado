module;
#include <cassert>
#include <openssl/err.h>
#include <openssl/rand.h>
export module quic:crypto;

import std;
import :types;

// quic:crypto is now minimal — OpenSSL 3.6 QUIC handles all key derivation,
// HKDF, AEAD, and header protection internally via OSSL_QUIC_server_method().
// Only random byte generation and CID construction remain here.

namespace quic::crypto {

export struct CryptoError : std::runtime_error {
    explicit CryptoError(std::string_view ctx) : std::runtime_error(std::string(ctx) + ": " + ssl_error()) {}

  private:
    static std::string ssl_error() {
        char buf[256];
        ERR_error_string_n(ERR_get_error(), buf, sizeof(buf));
        return buf;
    }
};

export void random_bytes(std::span<std::byte> out) {
    if (RAND_bytes(reinterpret_cast<unsigned char *>(out.data()), static_cast<int>(out.size())) != 1)
        throw CryptoError("RAND_bytes");
}

export template <std::size_t Len = CID_DEFAULT_LEN>
ConnectionId generate_cid() {
    static_assert(Len <= CID_MAX_LEN);
    ConnectionId cid;
    cid.len = static_cast<std::uint8_t>(Len);
    random_bytes(std::span{reinterpret_cast<std::byte *>(cid.data), Len});
    return cid;
}

export ConnectionId generate_cid(std::size_t len) {
    assert(len <= CID_MAX_LEN);
    ConnectionId cid;
    cid.len = static_cast<std::uint8_t>(len);
    random_bytes(std::span{reinterpret_cast<std::byte *>(cid.data), len});
    return cid;
}

} // namespace quic::crypto
