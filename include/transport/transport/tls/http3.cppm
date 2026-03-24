module;

#include <openssl/ssl.h>

export module tls:http3;

import :tls;

int alpn_cb(SSL *, const unsigned char **out, unsigned char *outlen, const unsigned char *in, unsigned int inlen,
            void *) {
    static constexpr unsigned char h3[] = {2, 'h', '3'};
    return ::SSL_select_next_proto(const_cast<unsigned char **>(out), outlen, h3, sizeof(h3), in, inlen) ==
                   OPENSSL_NPN_NEGOTIATED
               ? SSL_TLSEXT_ERR_OK
               : SSL_TLSEXT_ERR_NOACK;
}

export namespace transport::tls::http3 {

// Produces a SslCtx configured for h3 over QUIC.
// Pass the result to your QUIC server layer (SSL_new / SSL_listen).
[[nodiscard]] transport::tls::SslCtx make_ctx(std::string_view cert_pem, std::string_view key_pem) {
    return transport::tls::SslCtx::from_files(cert_pem, key_pem, ::OSSL_QUIC_server_method(), alpn_cb);
}

} // namespace transport::tls::http3
