module;
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <cstdio>
#include <nghttp2/nghttp2.h>
#include <winsock2.h>
export module server.http2:win32;

import std;
import server.tls;
import :types;

namespace server::http2 {

// ── Session ───────────────────────────────────────────────────────────────────

Session::Session(transport::tls::Connection &&tls, Handler handler)
    : m_tls(std::move(tls)), m_handler(std::move(handler)) {

    nghttp2_session_callbacks *cbs;
    nghttp2_session_callbacks_new(&cbs);
    nghttp2_session_callbacks_set_send_callback(cbs, cb_send);
    nghttp2_session_callbacks_set_on_frame_recv_callback(cbs, cb_on_frame_recv);
    nghttp2_session_callbacks_set_on_data_chunk_recv_callback(cbs, cb_on_data_chunk_recv);
    nghttp2_session_callbacks_set_on_stream_close_callback(cbs, cb_on_stream_close);
    nghttp2_session_callbacks_set_on_header_callback(cbs, cb_on_header);
    nghttp2_session_callbacks_set_on_begin_headers_callback(cbs, cb_on_begin_headers);
    nghttp2_session_server_new(&m_session, cbs, this);
    nghttp2_session_callbacks_del(cbs);

    nghttp2_settings_entry iv{NGHTTP2_SETTINGS_MAX_CONCURRENT_STREAMS, 100};
    nghttp2_submit_settings(m_session, NGHTTP2_FLAG_NONE, &iv, 1);
}

Session::~Session() {
    if (m_session)
        nghttp2_session_del(m_session);
}

void Session::run() {
    std::array<std::byte, 16384> buf;

    while (nghttp2_session_want_read(m_session) || nghttp2_session_want_write(m_session)) {

        if (nghttp2_session_want_write(m_session)) {
            if (nghttp2_session_send(m_session) != 0)
                break;
        }

        if (nghttp2_session_want_read(m_session)) {
            std::ptrdiff_t n = 0;
            try {
                n = m_tls.recv(buf);
            } catch (...) {
                break;
            }
            if (n == 0)
                break;

            if (nghttp2_session_mem_recv(m_session, reinterpret_cast<const uint8_t *>(buf.data()),
                                         static_cast<size_t>(n)) < 0)
                break;
        }
    }
}

// ── nghttp2 callbacks ─────────────────────────────────────────────────────────

ssize_t Session::cb_send(nghttp2_session *, const uint8_t *data, size_t length, int, void *user_data) {
    auto *self = static_cast<Session *>(user_data);
    try {
        auto n = self->m_tls.send(std::span{reinterpret_cast<const std::byte *>(data), length});
        return static_cast<ssize_t>(n);
    } catch (...) {
        return NGHTTP2_ERR_CALLBACK_FAILURE;
    }
}

int Session::cb_on_begin_headers(nghttp2_session *, const nghttp2_frame *frame, void *user_data) {
    auto *self = static_cast<Session *>(user_data);
    if (frame->hd.type == NGHTTP2_HEADERS && frame->headers.cat == NGHTTP2_HCAT_REQUEST)
        self->m_streams.emplace(frame->hd.stream_id, StreamData{});
    return 0;
}

int Session::cb_on_header(nghttp2_session *, const nghttp2_frame *frame, const uint8_t *name, size_t namelen,
                          const uint8_t *value, size_t valuelen, uint8_t, void *user_data) {
    auto *self = static_cast<Session *>(user_data);
    if (frame->hd.type != NGHTTP2_HEADERS || frame->headers.cat != NGHTTP2_HCAT_REQUEST)
        return 0;

    auto it = self->m_streams.find(frame->hd.stream_id);
    if (it == self->m_streams.end())
        return 0;

    std::string n_str(reinterpret_cast<const char *>(name), namelen);
    std::string v_str(reinterpret_cast<const char *>(value), valuelen);

    auto &req = it->second.req;
    if (n_str == ":method")
        req.method = v_str;
    else if (n_str == ":path")
        req.path = v_str;
    else if (n_str == ":scheme")
        req.scheme = v_str;
    else if (n_str == ":authority")
        req.authority = v_str;
    else
        req.headers.push_back({std::move(n_str), std::move(v_str)});
    return 0;
}

int Session::cb_on_data_chunk_recv(nghttp2_session *, uint8_t, int32_t stream_id, const uint8_t *data, size_t len,
                                   void *user_data) {
    auto *self = static_cast<Session *>(user_data);
    auto it = self->m_streams.find(stream_id);
    if (it == self->m_streams.end())
        return 0;
    auto &body = it->second.req.body;
    body.insert(body.end(), reinterpret_cast<const std::byte *>(data), reinterpret_cast<const std::byte *>(data) + len);
    return 0;
}

int Session::cb_on_frame_recv(nghttp2_session *, const nghttp2_frame *frame, void *user_data) {
    auto *self = static_cast<Session *>(user_data);
    if (!(frame->hd.flags & NGHTTP2_FLAG_END_STREAM))
        return 0;

    int32_t sid = frame->hd.stream_id;
    auto it = self->m_streams.find(sid);
    if (it == self->m_streams.end())
        return 0;

    Response resp = self->m_handler(std::move(it->second.req));
    self->send_response(sid, std::move(resp));
    return 0;
}

int Session::cb_on_stream_close(nghttp2_session *, int32_t stream_id, uint32_t, void *user_data) {
    auto *self = static_cast<Session *>(user_data);
    self->m_streams.erase(stream_id);
    return 0;
}

// ── send_response ─────────────────────────────────────────────────────────────

void Session::send_response(int32_t stream_id, Response resp) {
    std::string status_str = std::to_string(resp.status);
    std::string body_len = std::to_string(resp.body.size());

    std::vector<nghttp2_nv> nva;
    nva.push_back({const_cast<uint8_t *>(reinterpret_cast<const uint8_t *>(":status")),
                   const_cast<uint8_t *>(reinterpret_cast<const uint8_t *>(status_str.c_str())), 7, status_str.size(),
                   NGHTTP2_NV_FLAG_NONE});
    nva.push_back({const_cast<uint8_t *>(reinterpret_cast<const uint8_t *>("content-length")),
                   const_cast<uint8_t *>(reinterpret_cast<const uint8_t *>(body_len.c_str())), 14, body_len.size(),
                   NGHTTP2_NV_FLAG_NONE});

    for (auto &h : resp.headers) {
        nva.push_back({const_cast<uint8_t *>(reinterpret_cast<const uint8_t *>(h.name.c_str())),
                       const_cast<uint8_t *>(reinterpret_cast<const uint8_t *>(h.value.c_str())), h.name.size(),
                       h.value.size(), NGHTTP2_NV_FLAG_NONE});
    }

    struct BodyCtx {
        std::vector<std::byte> data;
        size_t offset{0};
    };
    auto body_ctx = std::make_shared<BodyCtx>(BodyCtx{std::move(resp.body), 0});

    nghttp2_data_provider dp{};
    dp.source.ptr = body_ctx.get();
    dp.read_callback = [](nghttp2_session *, int32_t, uint8_t *buf, size_t length, uint32_t *data_flags,
                          nghttp2_data_source *src, void *) -> ssize_t {
        auto *ctx = static_cast<BodyCtx *>(src->ptr);
        size_t remaining = ctx->data.size() - ctx->offset;
        size_t to_copy = std::min(length, remaining);
        std::memcpy(buf, reinterpret_cast<const uint8_t *>(ctx->data.data()) + ctx->offset, to_copy);
        ctx->offset += to_copy;
        if (ctx->offset >= ctx->data.size())
            *data_flags |= NGHTTP2_DATA_FLAG_EOF;
        return static_cast<ssize_t>(to_copy);
    };

    nghttp2_submit_response(m_session, stream_id, nva.data(), nva.size(), body_ctx->data.empty() ? nullptr : &dp);

    m_streams[stream_id].req.body = std::move(body_ctx->data);
}

// ── Server ────────────────────────────────────────────────────────────────────

Server Server::listen(std::string_view ip, std::uint16_t port, transport::tls::SslCtx &ctx, Handler handler,
                      int backlog) {
    Server s;
    s.m_tls = transport::tls::Server::listen(ip, port, ctx, backlog);
    s.m_handler = std::move(handler);
    return s;
}

Server Server::listen(std::uint16_t port, transport::tls::SslCtx &ctx, Handler handler, int backlog) {
    return listen("0.0.0.0", port, ctx, std::move(handler), backlog);
}

void Server::run() {
    while (true) {
        transport::tls::Connection tls = m_tls.accept();
        Handler h = m_handler;

        std::thread([tls = std::move(tls), h = std::move(h)]() mutable {
            try {
                Session session(std::move(tls), std::move(h));
                session.run();
            } catch (const std::exception &e) {
                std::println(stderr, "http2 session error: {}", e.what());
            }
        }).detach();
    }
}

} // namespace server::http2
