module;

#include <stdio.h>

export module congelado;

import std;
import io_tls;
import io_layer_http2;
import core_server;


export namespace app {

// struct Request {
//     std::string_view path;
// };
//
// struct Response {
//     std::string body;
// };
// void get_users(Request &, Response &) noexcept { std::println(">>> Get Users Handler: Fetching user list..."); }
// void create_user(Request &, Response &) noexcept { std::println(">>> Create User Handler: Creating a new user..."); }
// void api_wild(Request &, Response &) noexcept { std::println("Wildcard"); }
// void get_settings(Request &, Response &) noexcept {
//     std::println(">>> Get Settings Handler: Fetching user settings...");
// }
//
// void logger_mw(app::Request &req, app::Response &res, auto next) noexcept {
//     std::println("[Log] Incoming request to: testing");
//     next(req, res);
// }
//
// // // 3. Construct the Router Tree using the Fluent API
// auto router_ctx = io::server::RouterContext<Request, Response>{};
// auto bulder =
//     io::server::Router<Request, Response>(router_ctx, "/testing")
//         .use(logger_mw)
//         .add_router(
//             io::server::Router<Request, Response>(router_ctx, std::string_view{"/api"})
//                 .add_route(io::server::Route<Request, Response>{"/version"}.get(get_users).post(create_user))
//                 .add_route(io::server::Route<Request, Response>{"*"}.get(api_wild))
//                 .add_route(io::server::Route<Request, Response>{"/settings"}.get(get_settings))
//                 .add_router(io::server::Router<Request, Response>(router_ctx, std::string_view{"/admin"})
//                                 .add_route(io::server::Route<Request, Response>{"/dashboard"}.get(
//                                     [](Request &, Response &) noexcept {
//                                         std::println(">>> Admin Dashboard Handler: Displaying admin dashboard...");
//                                     }))));


// auto server = io::server::ServerBuilder<Request,
// Response>{}.address("localhost").port(8080).build(router_ctx);


class SimpleHttp2Server {
  public:
    // Fixed namespace: io::base::tls::SslCtx
    SimpleHttp2Server(std::string_view ip, std::uint16_t port, io::base::tls::SslCtx &ctx)
        : m_server(io::base::tls::basic::Server::listen(ip, port, ctx)), m_port{port} {}

    void run() {
        std::println("Server listening on port {}...", m_port);

        // while (m_server.valid()) {
        //     auto conn = m_server.accept();
        //     if (conn) {
        //         handle_client(std::move(*conn));
        //     }
        // }
    }

  private:
    void handle_client(io::base::tls::basic::Connection &&conn) {
        try {
            if (conn.alpn() != "h2") {
                std::println("Client did not negotiate h2. Closing.");
                conn.close();
                return;
            }

            std::println("New HTTP/2 connection established.");

            // Explicitly using the full namespace to help the compiler
            io::layer::http2::Session<true> session(std::move(conn));
            session.loop();

        } catch (const std::exception &e) {
            // Fix: Use std::cerr or std::println for errors (std::stderr is a C macro)
            std::println(stderr, "Session error: {}", e.what());
        }
    }

    io::base::tls::basic::Server m_server;
    std::uint16_t m_port;
};

} // namespace app
