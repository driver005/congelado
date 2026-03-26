module;

#include <openssl/ssl.h>

export module tls:http2;

import :types;

inline int alpn_cb(::SSL *, const unsigned char **out, unsigned char *outlen, const unsigned char *in,
                   unsigned int inlen, void *) {
    // Protocol list for negotiation: h2 followed by http/1.1
    static constexpr unsigned char protos[] = {2, 'h', '2', 8, 'h', 't', 't', 'p', '/', '1', '.', '1'};
    if (::SSL_select_next_proto(const_cast<unsigned char **>(out), outlen, protos, sizeof(protos), in, inlen) ==
        OPENSSL_NPN_NEGOTIATED) {
        return SSL_TLSEXT_ERR_OK;
    }
    return SSL_TLSEXT_ERR_NOACK;
}

export namespace transport::tls::http2 {

// Produces a SslCtx configured for h2/http1.1 over TLS.
// Pass the result directly to transport::tls::Server::listen().
[[nodiscard]] transport::tls::SslCtx make_ctx(std::string_view cert_pem, std::string_view key_pem) {
    return transport::tls::SslCtx::from_files(cert_pem, key_pem, ::TLS_server_method(), alpn_cb);
}

} // namespace transport::tls::http2
