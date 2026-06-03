export module core_server:server;

import std;
import interfaces;
import :router;

export namespace core::server {

template <typename Derived>
class Server {
  public:
    explicit Server(RouteHandler<Derived> &&handler) : m_handler(std::move(handler)) {}

    Server(Server &&) = default;
    Server &operator=(Server &&) = default;

    void match(Method method, std::string_view path, interfaces::IRequest<Derived> &req,
               interfaces::IResponse<Derived> &res) {
        m_handler.match(method, path, req, res);
    }

  private:
    RouteHandler<Derived> m_handler;
};

} // namespace core::server
