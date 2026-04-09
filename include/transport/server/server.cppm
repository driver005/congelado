export module server:server;

import std;
import :router;

export namespace transport::server {

template <typename Request, typename Response>
class Server {
  public:
    explicit constexpr Server(RouteHandler<Request, Response> &&handler) : m_handler(std::move(handler)) {}

    constexpr void match(Method method, std::string_view path, Request &req, Response &res) {
        m_handler.match(method, path, req, res);
    }

  private:
    RouteHandler<Request, Response> m_handler;
};


} // namespace transport::server
