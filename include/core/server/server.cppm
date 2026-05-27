export module core_server:server;

import std;
import io_base_leverage;
import interfaces;
import :router;

export namespace core::server {

template <typename Derived>
class Server {
  public:
    explicit constexpr Server(RouteHandler<Derived> &&handler) : m_handler(std::move(handler)) {}

    constexpr void match(Method method, std::string_view path, interfaces::IRequest<Derived> &req,
                         interfaces::IResponse<Derived> &res) {
        m_handler.match(method, path, req, res);
    }

  private:
    RouteHandler<Derived> m_handler;
    io::base::leverage::Leverager<io::base::leverage::Context> m_leverager;
};


} // namespace core::server
