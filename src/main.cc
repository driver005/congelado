#include <stdio.h>

import std;
import tls;
import congelado;
import server;

int main() {

    auto res = app::Request{.path = "/api/testing"};
    auto rsp = app::Response{};
    app::server.match(transport::server::Method::GET, "/testing/api/settings", res, rsp);
    std::println("Response Body: {}", rsp.body);
    // try {
    //     // Prepare a mock request for the parameterized route
    //     // Request is exported from the router module
    //     app::Request profile_request{.path = "/user/gemini_user"};
    //
    //     std::println("--- Dispatching Request ---");
    //
    //     // Match and execute the second route
    //     // This will trigger: Logger (Global) -> Auth (Local) -> User Handler
    //     app::app_router.match(transport::server::Method::GET, "/user/gemini_user", profile_request);
    //
    //     std::println("\n--- Dispatching Home ---");
    //     app::Request home_request{.path = "/"};
    //     app::app_router.match(transport::server::Method::GET, "/", home_request);
    //
    // } catch (const std::exception &e) {
    //     std::println(std::cerr, "Routing Error: {}", e.what());
    //     return 1;
    // }

    return 0;
    // try {
    //     // Use the namespace found by your compiler
    //     auto ctx = transport::tls::http2::make_ctx();
    //
    //     app::SimpleHttp2Server server("0.0.0.0", 9000, ctx);
    //     server.run();
    //
    // } catch (const std::exception &e) {
    //     std::println(stderr, "Fatal error: {}", e.what());
    //     return 1;
    // }
    //
    // return 0;
}
