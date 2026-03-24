module;
#include <nghttp2/nghttp2.h>
export module server.http2:types;

import std;
import tls;

namespace server::http2 {

// ── Request / Response ────────────────────────────────────────────────────────

export struct Header {
    std::string name;
    std::string value;
};

export struct Request {
    std::string method;
    std::string path;
    std::string scheme;
    std::string authority;
    std::vector<Header> headers;
    std::vector<std::byte> body;
};

export struct Response {
    int status{200};
    std::vector<Header> headers;
    std::vector<std::byte> body;

    // Convenience: set body from string
    static Response text(int status, std::string_view body, std::string_view content_type = "text/plain") {
        Response r;
        r.status = status;
        r.headers.push_back({"content-type", std::string(content_type)});
        r.body.assign(reinterpret_cast<const std::byte *>(body.data()),
                      reinterpret_cast<const std::byte *>(body.data() + body.size()));
        return r;
    }
};

// ── Handler type ──────────────────────────────────────────────────────────────

export using Handler = std::function<Response(Request)>;

// ── Session — one HTTP/2 session per TLS connection ──────────────────────────

export class Session {
  public:
    explicit Session(transport::tls::Connection &&tls, Handler handler);
    ~Session();

    Session(const Session &) = delete;
    Session &operator=(const Session &) = delete;
    Session(Session &&) = delete;
    Session &operator=(Session &&) = delete;

    void run();

  private:
    static ssize_t cb_send(nghttp2_session *, const uint8_t *data, size_t length, int, void *user_data);
    static int cb_on_frame_recv(nghttp2_session *, const nghttp2_frame *frame, void *user_data);
    static int cb_on_data_chunk_recv(nghttp2_session *, uint8_t, int32_t stream_id, const uint8_t *data, size_t len,
                                     void *user_data);
    static int cb_on_stream_close(nghttp2_session *, int32_t stream_id, uint32_t, void *user_data);
    static int cb_on_header(nghttp2_session *, const nghttp2_frame *frame, const uint8_t *name, size_t namelen,
                            const uint8_t *value, size_t valuelen, uint8_t, void *user_data);
    static int cb_on_begin_headers(nghttp2_session *, const nghttp2_frame *frame, void *user_data);

    void send_response(int32_t stream_id, Response resp);

    transport::tls::Connection m_tls;
    Handler m_handler;
    nghttp2_session *m_session{nullptr};

    // ── Per-stream state ──
    // Added fields to keep response memory alive for nghttp2 async processing
    struct StreamData {
        Request req{};

        std::string res_status;   // Anchors :status string
        std::string res_body_len; // Anchors content-length string
        std::vector<std::byte> res_body;
        size_t res_offset{0}; // Tracks progress in data_provider
    };
    std::unordered_map<int32_t, StreamData> m_streams;
};

// ── Server ────────────────────────────────────────────────────────────────────

export class Server {
  public:
    static Server listen(std::uint16_t port, transport::tls::SslCtx &ctx, Handler handler, int backlog = 128);
    static Server listen(std::string_view ip, std::uint16_t port, transport::tls::SslCtx &ctx, Handler handler,
                         int backlog = 128);

    Server() = default;
    Server(const Server &) = delete;
    Server &operator=(const Server &) = delete;
    Server(Server &&) noexcept = default;
    Server &operator=(Server &&) noexcept = default;
    ~Server() = default;

    void run();

    [[nodiscard]] bool valid() const noexcept { return m_tls.valid(); }
    void close() noexcept { m_tls.close(); }

  private:
    transport::tls::Server m_tls{};
    Handler m_handler{};
};

} // namespace server::http2
