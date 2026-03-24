// module;
// #include <nghttp3/nghttp3.h>
// #include <ngtcp2/ngtcp2.h>
// #include <openssl/ssl.h>
// export module server.http3:types;
//
// import std;
// import server.http2; // reuse Header, Request, Response, Handler
//
// namespace server::http3 {
//
// // Re-export the shared HTTP types so callers only need server.http3
// export using server::http2::Header;
// export using server::http2::Request;
// export using server::http2::Response;
// export using server::http2::Handler;
//
// // ── QuicError ─────────────────────────────────────────────────────────────────
//
// export struct QuicError : std::runtime_error {
//   explicit QuicError(std::string_view ctx, int err = 0)
//       : std::runtime_error(std::string(ctx) + (err ? ": " + std::string(ngtcp2_strerror(err)) : "")) {}
// };
//
// // ── SslCtx — quictls SSL_CTX configured for QUIC ─────────────────────────────
//
// export class SslCtx {
// public:
//   static SslCtx from_files(std::string_view cert_pem, std::string_view key_pem);
//
//   SslCtx() = default;
//   ~SslCtx() {
//     if (m_ctx)
//       ::SSL_CTX_free(m_ctx);
//   }
//
//   SslCtx(const SslCtx &) = delete;
//   SslCtx &operator=(const SslCtx &) = delete;
//   SslCtx(SslCtx &&o) noexcept : m_ctx(o.m_ctx) { o.m_ctx = nullptr; }
//   SslCtx &operator=(SslCtx &&o) noexcept {
//     if (this != &o) {
//       if (m_ctx)
//         ::SSL_CTX_free(m_ctx);
//       m_ctx = o.m_ctx;
//       o.m_ctx = nullptr;
//     }
//     return *this;
//   }
//
//   [[nodiscard]] SSL_CTX *get() const noexcept { return m_ctx; }
//   [[nodiscard]] bool valid() const noexcept { return m_ctx != nullptr; }
//
// private:
//   explicit SslCtx(SSL_CTX *ctx) : m_ctx(ctx) {}
//   SSL_CTX *m_ctx{nullptr};
// };
//
// // ── Connection — one QUIC connection ─────────────────────────────────────────
// //
// // Owns the ngtcp2_conn + nghttp3_conn for one client. Not copyable or movable
// // after construction — Session holds it by unique_ptr and shares the address
// // with ngtcp2/nghttp3 callbacks.
//
// export class Connection {
// public:
//   Connection(int udp_fd, const sockaddr_storage &peer_addr, socklen_t peer_addrlen, const ngtcp2_pkt_hd &hd,
//              SSL_CTX *ssl_ctx, Handler handler);
//   ~Connection();
//
//   Connection(const Connection &) = delete;
//   Connection &operator=(const Connection &) = delete;
//   Connection(Connection &&) = delete;
//   Connection &operator=(Connection &&) = delete;
//
//   // Feed an inbound UDP datagram. Returns false if the connection is done.
//   [[nodiscard]] bool recv_packet(const uint8_t *data, size_t len, const sockaddr_storage &addr, socklen_t addrlen);
//
//   // Flush outbound QUIC packets. Returns false on fatal error.
//   [[nodiscard]] bool flush();
//
//   // Handle ngtcp2 timer expiry.
//   void on_timer();
//
//   // Nanoseconds until the next ngtcp2 timer fires (for poll timeout).
//   [[nodiscard]] std::int64_t next_expiry_ns() const noexcept;
//
//   [[nodiscard]] bool closed() const noexcept { return m_closed; }
//
//   // The DCID used to demux incoming datagrams to this connection.
//   [[nodiscard]] ngtcp2_cid dcid() const noexcept { return m_dcid; }
//
// private:
//   // ── ngtcp2 callbacks ───────────────────────────────────────────────────────
//   static int cb_recv_client_initial(ngtcp2_conn *, const ngtcp2_cid *, void *);
//   static int cb_recv_crypto_data(ngtcp2_conn *, ngtcp2_crypto_level, uint64_t, const uint8_t *, size_t, void *);
//   static int cb_encrypt(ngtcp2_conn *, uint8_t *, const ngtcp2_crypto_aead *, const ngtcp2_crypto_aead_ctx *,
//                         const uint8_t *, size_t, const uint8_t *, size_t, const uint8_t *, size_t, void *);
//   static int cb_decrypt(ngtcp2_conn *, uint8_t *, const ngtcp2_crypto_aead *, const ngtcp2_crypto_aead_ctx *,
//                         const uint8_t *, size_t, const uint8_t *, size_t, const uint8_t *, size_t, void *);
//   static int cb_hp_mask(ngtcp2_conn *, uint8_t *, const ngtcp2_crypto_cipher *, const ngtcp2_crypto_cipher_ctx *,
//                         const uint8_t *, const uint8_t *, void *);
//   static int cb_stream_close(ngtcp2_conn *, uint32_t, int64_t, uint64_t, void *, void *);
//   static int cb_stream_reset(ngtcp2_conn *, int64_t, uint64_t, uint64_t, void *, void *);
//   static int cb_recv_stream_data(ngtcp2_conn *, uint32_t, int64_t, uint64_t, const uint8_t *, size_t, void *, void
//   *); static int cb_acked_stream_data_offset(ngtcp2_conn *, int64_t, uint64_t, uint64_t, void *, void *); static int
//   cb_extend_max_local_streams_bidi(ngtcp2_conn *, uint64_t, void *); static void cb_rand(uint8_t *, size_t, const
//   ngtcp2_rand_ctx *); static int cb_get_new_connection_id(ngtcp2_conn *, ngtcp2_cid *, uint8_t *, size_t, void *);
//
//   // ── nghttp3 callbacks ──────────────────────────────────────────────────────
//   static int h3_recv_header(nghttp3_conn *, int64_t stream_id, int32_t, nghttp3_rcbuf *, nghttp3_rcbuf *, uint8_t,
//                             void *);
//   static int h3_end_headers(nghttp3_conn *, int64_t stream_id, int, void *);
//   static int h3_recv_data(nghttp3_conn *, int64_t stream_id, const uint8_t *, size_t, void *);
//   static int h3_end_stream(nghttp3_conn *, int64_t stream_id, void *);
//   static int h3_send_stop_sending(nghttp3_conn *, int64_t stream_id, uint64_t, void *);
//   static int h3_deferred_consume(nghttp3_conn *, int64_t, size_t, void *);
//
//   // ── helpers ────────────────────────────────────────────────────────────────
//   void setup_http3();
//   void send_response(int64_t stream_id, Response resp);
//   int send_packets();
//
//   // ── state ──────────────────────────────────────────────────────────────────
//   int m_fd{-1};
//   sockaddr_storage m_peer_addr{};
//   socklen_t m_peer_addrlen{};
//   ngtcp2_cid m_dcid{};
//
//   SSL *m_ssl{nullptr};
//   ngtcp2_conn *m_conn{nullptr};
//   nghttp3_conn *m_h3conn{nullptr};
//
//   Handler m_handler;
//   bool m_closed{false};
//   bool m_h3_setup{false};
//
//   struct StreamData {
//     Request req{};
//   };
//   std::unordered_map<int64_t, StreamData> m_streams;
// };
//
// // ── Server ────────────────────────────────────────────────────────────────────
//
// export class Server {
// public:
//   static Server listen(std::uint16_t port, SslCtx &ctx, Handler handler);
//
//   static Server listen(std::string_view ip, std::uint16_t port, SslCtx &ctx, Handler handler);
//
//   Server() = default;
//   ~Server();
//
//   Server(const Server &) = delete;
//   Server &operator=(const Server &) = delete;
//   Server(Server &&) noexcept;
//   Server &operator=(Server &&) noexcept;
//
//   // Blocking event loop — one thread per connection.
//   void run();
//
//   [[nodiscard]] bool valid() const noexcept { return m_fd >= 0; }
//   void close() noexcept;
//
// private:
//   int m_fd{-1};
//   SSL_CTX *m_ctx{nullptr};
//   Handler m_handler;
//
//   // Active connections demuxed by DCID
//   std::unordered_map<std::string, std::unique_ptr<Connection>> m_conns;
// };
//
// } // namespace server::http3
