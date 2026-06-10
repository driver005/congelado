module io.codec.quic.crypto;
@nogc nothrow:

import io.codec.quic.types;
import modules.openssl;

// quic:crypto is now minimal — OpenSSL 3.6 QUIC handles all key derivation,
// HKDF, AEAD, and header protection internally via OSSL_QUIC_server_method().
// Only random byte generation and CID construction remain here.

// PORT-NOTE: C++ struct CryptoError : std::runtime_error → D: cannot use exceptions
// in @nogc. Errors are signalled by return-false convention; CryptoError retained
// as a tag struct for documentation only.
struct CryptoError {
    // PORT-NOTE: value wrapper (struct), exempt from class-only rule
    const(char)[] message;
}

// random_bytes — fills out with cryptographically random data.
// PORT-NOTE: C++ throws CryptoError → D returns false on failure.
bool random_bytes(ubyte[] out_bytes) {
    return RAND_bytes(out_bytes.ptr, cast(int)out_bytes.length) == 1;
}

// generate_cid — generate a random ConnectionId of the given length.
// PORT-NOTE: C++ template<size_t Len = CID_DEFAULT_LEN> → D with default arg.
ConnectionId generate_cid(size_t len = CID_DEFAULT_LEN) {
    assert(len <= CID_MAX_LEN);
    ConnectionId cid;
    cid.len = cast(ubyte)len;
    // PORT-NOTE: reinterpret_cast<unsigned char*>(cid.data) → cast to ubyte*
    random_bytes(cid.data[0 .. len]);
    return cid;
}
