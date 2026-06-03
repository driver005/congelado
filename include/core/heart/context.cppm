export module core_heart:context;

import std;
import interfaces;
import io_shared;
import core_server;
import core_contract;
import io_base_leverage;

export namespace core::heart {

using Protocol = io::shared::http::Protocol;

class AppContext {
  public:
    AppContext() {
        m_thread_pool.emplace(m_contract_group,
                              static_cast<std::size_t>(std::thread::hardware_concurrency()));
    }

    [[nodiscard]] core::server::RouterContext<Protocol> *get_router() noexcept { return &m_router; }

    [[nodiscard]] core::contract::ContractGroup<> &get_contract_group() noexcept {
        return m_contract_group;
    }

    [[nodiscard]] io::base::leverage::Leverager<io::base::leverage::Context> &
    get_leverager() noexcept {
        return m_leverager;
    }

    void build() {
        m_server.emplace(core::server::ServerBuilder<Protocol>{}.build(std::move(m_router)));
    }

    [[nodiscard]] interfaces::DispatchFn get_dispatch() noexcept {
        return [this](interfaces::IRequest<Protocol> &req, interfaces::IResponse<Protocol> &res) {
            if (!m_server)
                return;

            auto http_method = io::shared::http::parse_method(req.get_method());
            core::server::Method method{};

            switch (http_method) {
            case io::shared::http::HttpMethod::GET:
                method = core::server::Method::GET;
                break;
            case io::shared::http::HttpMethod::POST:
                method = core::server::Method::POST;
                break;
            case io::shared::http::HttpMethod::PUT:
                method = core::server::Method::PUT;
                break;
            case io::shared::http::HttpMethod::DELETE:
                method = core::server::Method::DELETE;
                break;
            case io::shared::http::HttpMethod::PATCH:
                method = core::server::Method::PATCH;
                break;
            case io::shared::http::HttpMethod::HEAD:
                method = core::server::Method::HEAD;
                break;
            case io::shared::http::HttpMethod::OPTIONS:
                method = core::server::Method::OPTIONS;
                break;
            default:
                res.set_status(interfaces::Status::METHOD_NOT_ALLOWED);
                return;
            }

            try {
                m_server->match(method, req.get_target(), req, res);
            } catch (const std::runtime_error &) {
                res.set_status(interfaces::Status::NOT_FOUND);
            }
        };
    }

  private:
    core::contract::ContractGroup<> m_contract_group;
    io::base::leverage::Leverager<io::base::leverage::Context> m_leverager;
    std::optional<core::contract::ContractThreadPool<>> m_thread_pool;

    core::server::RouterContext<Protocol> m_router;
    std::optional<core::server::Server<Protocol>> m_server;
};

} // namespace core::heart
