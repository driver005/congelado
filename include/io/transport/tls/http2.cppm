module;

#include <openssl/ssl.h>

export module io_tls:http2;

import :types;

inline int alpn_cb_h2(::SSL *, const unsigned char **out, unsigned char *outlen, const unsigned char *in,
                      unsigned int inlen, void *) {
    // Protocol list for negotiation: h2 followed by http/1.1
    static constexpr unsigned char protos[] = {2, 'h', '2', 8, 'h', 't', 't', 'p', '/', '1', '.', '1'};
    if (::SSL_select_next_proto(const_cast<unsigned char **>(out), outlen, protos, sizeof(protos), in, inlen) ==
        OPENSSL_NPN_NEGOTIATED) {
        return SSL_TLSEXT_ERR_OK;
    }
    return SSL_TLSEXT_ERR_NOACK;
}

export namespace transport::base::tls::http2 {

// Produces a SslCtx configured for h2/http1.1 over TLS.
// Pass the result directly to  tls::Server::listen().
[[nodiscard]] tls::SslCtx make_ctx(std::string_view path = ".") {
    tls::SslCtx ctx{::TLS_server_method(), path};

    ctx.set_alpn_callback(alpn_cb_h2);

    return ctx;
    // return  tls::SslCtx::from_files(cert_pem, key_pem, ::TLS_server_method(), alpn_cb_h2);
}

} // namespace transport::base::tls::http2
