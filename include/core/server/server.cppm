export module core_server:server;

import std;
import io_base_leverage;
import :router;

export namespace core::server {

template <typename Request, typename Response>
class Server {
  public:
    explicit constexpr Server(RouteHandler<Request, Response> &&handler) : m_handler(std::move(handler)) {}

    constexpr void match(Method method, std::string_view path, Request &req, Response &res) {
        m_handler.match(method, path, req, res);
    }

  private:
    RouteHandler<Request, Response> m_handler;
    io::base::leverage::Leverager<io::base::leverage::Context> m_leverager;
};


} // namespace core::server
