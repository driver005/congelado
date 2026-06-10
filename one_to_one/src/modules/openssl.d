module modules.openssl;
@nogc nothrow:

// Re-export the SSL types and functions used by the C++ codebase, mirroring
// the C++ module that wraps <openssl/err.h> and <openssl/ssl.h> and exports
// only SSL_CTX and SSL_new.
//
// D bindings for OpenSSL are in deimos/openssl. If that package is not
// available, declare the minimal extern(C) bindings needed below.

version (Have_deimos_openssl) {
    public import deimos.openssl.ssl : SSL_CTX, SSL_new;
} else {
    // Minimal hand-written bindings for the two symbols consumed by this codebase.
    extern(C):

    struct ssl_ctx_st;
    alias SSL_CTX = ssl_ctx_st;

    struct ssl_st;
    alias SSL = ssl_st;

    SSL* SSL_new(SSL_CTX* ctx);

    // export using ::SSL_library_init;  // commented out in original
}
