module;

#include <openssl/ssl.h>

export module tls:http3;

import :types;

inline int alpn_cb_h3(::SSL *, const unsigned char **out, unsigned char *outlen, const unsigned char *in,
                      unsigned int inlen, void *) {
    static constexpr unsigned char h3_proto[] = {2, 'h', '3'};
    if (::SSL_select_next_proto(const_cast<unsigned char **>(out), outlen, h3_proto, sizeof(h3_proto), in, inlen) ==
        OPENSSL_NPN_NEGOTIATED) {
        return SSL_TLSEXT_ERR_OK;
    }
    return SSL_TLSEXT_ERR_NOACK;
}

export namespace transport::tls::http3 {

// Produces a SslCtx configured for h3 over QUIC.
// Pass the result to your QUIC server layer (SSL_new / SSL_listen).
[[nodiscard]] SslCtx make_ctx(std::string_view path = ".") {
    tls::SslCtx ctx{::OSSL_QUIC_server_method(), path};

    ctx.set_alpn_callback(alpn_cb_h3);

    return ctx;
}

} // namespace transport::tls::http3
