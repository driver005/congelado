export module worker:engine_client;

import std;
import shared;
import core_contract;
import io_base_socket;
import io_layer_http2;
import io_shared;
import utils_buffering;
import core_logger;
import interfaces;
import core_client;

export namespace worker {

class EngineClient : public core::client::ClientHandler<EngineClient>,
                     public shared::HandlerBase {
  public:
    using ResponseFn = core::client::ClientHandler<EngineClient>::ResponseFn;
    using TlsSocket  = io::base::socket::Socket<io::base::socket::Protocol::TLS>;

  public:
    explicit EngineClient(std::string_view host, std::uint16_t port)
        : m_socket{io::base::socket::Endpoint{host, port}},
          m_pool{},
          m_discard_next{false},
          m_session{
              [this](utils::buffering::BufferNode &&node) noexcept { flush_node(std::move(node)); },
              [this]() noexcept {
                  core::logger::info("engine_client", "session closed by engine");
              },
              [this](interfaces::IRequest<io::shared::http::Protocol> &req,
                     interfaces::IResponse<io::shared::http::Protocol> &) {
                  dispatch_response(req);
              }},
          m_handshake{
              m_session.get_local_settings(),
              [this](utils::buffering::BufferNode &&node) noexcept {
                  m_session.send_node(std::move(node));
              }},
          m_handshake_done{false} {
        auto status = m_socket.sync_connect();
        if (status.get_status() != io::base::socket::VALUES::VALID) {
            core::logger::error("engine_client", "failed to connect to engine");
        }
    }

    void set_self_contract(core::contract::Contract<> contract) {
        m_self_contract = std::move(contract);
    }

    // CRTP connector 1 — called by ClientHandler::send<Protocol>
    template <typename Protocol>
    void do_send(std::unique_ptr<interfaces::IRequest<Protocol>> req, ResponseFn cb) {
        auto http_req = std::unique_ptr<io::layer::http2::HttpRequest>(
            static_cast<io::layer::http2::HttpRequest*>(req.release()));
        {
            std::lock_guard lock{m_mutex};
            m_outbound.push(OutboundRequest{std::move(http_req), std::move(cb)});
        }
        if (m_self_contract) {
            m_self_contract->schedule();
        }
    }

    // CRTP connector 2 — called by ClientHandler::request<Protocol>
    template <typename Protocol>
    std::unique_ptr<interfaces::IRequest<Protocol>>
    make_request(std::string_view method, std::string_view path,
                 std::string_view body = {}) {
        return std::make_unique<io::layer::http2::HttpRequest>(
            build_http_request(method, path, body));
    }

    std::string_view get_name() const noexcept override { return "EngineClient"; }

    shared::WorkerFunction on_execute() override {
        return [this]() {
            if (!m_handshake_done) {
                utils::buffering::BufferReader empty{};
                if (m_handshake.process(empty) == io::layer::http2::HandshakeState::COMPLETED) {
                    m_handshake_done = true;
                }
            }
            drain_outbound();
            receive_once();
            // m_outbound is mutex-protected (multi-threaded writes via do_send).
            // m_pending is only accessed on this contract thread (serialized by claim).
            std::lock_guard lock{m_mutex};
            if (!m_pending.empty() || !m_outbound.empty()) {
                shared::this_handler::shedule();
            }
        };
    }

    shared::ReleaseFunction on_released() noexcept override {
        return [this]() noexcept { m_socket.sync_close(); };
    }

  private:
    struct OutboundRequest {
        std::unique_ptr<io::layer::http2::HttpRequest> request;
        ResponseFn callback;
    };

    void drain_outbound() {
        std::queue<OutboundRequest> local;
        {
            std::lock_guard lock{m_mutex};
            std::swap(local, m_outbound);
        }
        while (!local.empty()) {
            auto item = std::move(local.front());
            local.pop();
            m_session.send(*item.request);
            m_pending.emplace(item.request->get_stream_id(), std::move(item.callback));
        }
    }

    void receive_once() {
        auto *slot = m_pool.acquire();
        auto [result, status] =
            m_socket.sync_receive(slot->get_data(), static_cast<unsigned>(slot->get_limit()), 0);
        if (status.get_status() == io::base::socket::VALUES::VALID && result > 0) {
            m_pool.notify_read(slot, static_cast<std::size_t>(result));
            m_session.receive(m_pool.get_view());
        } else {
            m_pool.notify_read(slot, 0);
        }
    }

    void flush_node(utils::buffering::BufferNode &&node) noexcept {
        if (m_discard_next) {
            m_discard_next = false;
            return;
        }
        auto *data = node.get_data();
        auto remaining = node.get_written();
        while (remaining > 0) {
            auto [sent, status] = m_socket.sync_send(data, remaining);
            if (status.get_status() == io::base::socket::VALUES::VALID && sent > 0) {
                data += sent;
                remaining -= sent;
            } else {
                core::logger::error("engine_client", "send failed, bytes_lost={}", remaining);
                return;
            }
        }
    }

    void dispatch_response(interfaces::IRequest<io::shared::http::Protocol> &req) {
        m_discard_next = true;

        auto &http_req = static_cast<io::layer::http2::HttpRequest &>(req);
        auto stream_id = http_req.get_stream_id();

        auto status_sv = req.find_header(":status");
        int status_code = 500;
        if (!status_sv.empty()) {
            std::from_chars(status_sv.data(), status_sv.data() + status_sv.size(), status_code);
        }

        std::string body;
        for (auto byte : req.get_body()) {
            body.push_back(static_cast<char>(byte));
        }

        auto it = m_pending.find(stream_id);
        if (it != m_pending.end()) {
            auto cb = std::move(it->second);
            m_pending.erase(it);
            cb(status_code, std::move(body));
        } else {
            core::logger::warning("engine_client", "no callback for stream {}", stream_id);
        }
    }

    // Renamed from make_request() to avoid conflict with ClientHandler::make_request<Protocol>
    io::layer::http2::HttpRequest build_http_request(std::string_view method,
                                                      std::string_view path,
                                                      std::string_view body) {
        using namespace io::layer::http2;
        auto req = (method == "POST")   ? HttpRequest::post(0, path)
                 : (method == "PUT")    ? HttpRequest::put(0, path)
                 : (method == "DELETE") ? HttpRequest::del(0, path)
                 : (method == "PATCH")  ? HttpRequest::patch(0, path)
                 : (method == "HEAD")   ? HttpRequest::head(0, path)
                                        : HttpRequest::get(0, path);
        if (!body.empty()) {
            // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
            auto *body_node = new utils::buffering::BufferNode{body.size()};
            for (char c : body) {
                body_node->push_back(static_cast<std::byte>(c));
            }
            req.get_body().push_back(body_node, 0, body.size());
        }
        return req;
    }

    TlsSocket m_socket;
    utils::buffering::BufferWriter m_pool;
    bool m_discard_next;
    io::layer::http2::Session m_session;
    io::layer::http2::Handshake<false> m_handshake;
    bool m_handshake_done;
    std::map<std::uint32_t, ResponseFn> m_pending;
    std::queue<OutboundRequest> m_outbound;
    std::mutex m_mutex;
    std::optional<core::contract::Contract<>> m_self_contract;
};

} // namespace worker
