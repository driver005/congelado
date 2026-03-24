// #include <cstdint>
// #include <stdio.h>
//
// import std;
// import server.http2;
// import server.http3;
// import tls;
//
// // ── helpers ───────────────────────────────────────────────────────────────────
//
// static std::string find_header(const std::vector<server::http2::Header> &hdrs, std::string_view name,
//                                std::string_view fallback = "") {
//   for (const auto &h : hdrs)
//     if (h.name == name)
//       return h.value;
//   return std::string(fallback);
// }
//
// void ensure_certs(std::string_view cert, std::string_view key) {
//   if (std::filesystem::exists(cert) && std::filesystem::exists(key)) {
//     return;
//   }
//
//   std::println("Certs not found. Generating self-signed certificates...");
//
//
//   // Constructing the openssl command
//   std::string cmd = std::format("openssl req -x509 -newkey rsa:2048 -keyout {} -out {} "
//                                 "-days 365 -nodes -subj \"/CN=localhost\" 2>/dev/null",
//                                 key, cert);
//
//   int result = std::system(cmd.c_str());
//
//   if (result != 0 || !std::filesystem::exists(cert)) {
//     throw std::runtime_error("Failed to generate self-signed certificates. Is openssl installed?");
//   }
//
//   std::println("Generated {} and {}", cert, key);
// }
//
// // ── shared request handler ────────────────────────────────────────────────────
//
// static server::http2::Response handle(server::http2::Request req) {
//   std::println("[{}] {} {}", req.authority, req.method, req.path);
//
//   if (req.method == "GET" && req.path == "/")
//     return server::http2::Response::text(200, "<!doctype html><h1>congelado HTTP/2 + HTTP/3</h1>", "text/html");
//
//   if (req.method == "GET" && req.path == "/health")
//     return server::http2::Response::text(200, R"({"status":"ok"})", "application/json");
//
//   if (req.method == "POST" && req.path == "/echo") {
//     auto ct = find_header(req.headers, "content-type", "application/octet-stream");
//     server::http2::Response resp;
//     resp.status = 200;
//     resp.headers.push_back({"content-type", std::move(ct)});
//     resp.body = std::move(req.body);
//     return resp;
//   }
//
//   if (req.method == "GET" && req.path == "/headers") {
//     std::string json = R"({"headers":{)";
//     bool first = true;
//     for (const auto &h : req.headers) {
//       if (!first)
//         json += ',';
//       json += '"' + h.name + "\":\"" + h.value + '"';
//       first = false;
//     }
//     json += "}}";
//     return server::http2::Response::text(200, json, "application/json");
//   }
//
//   return server::http2::Response::text(404, R"({"error":"not found"})", "application/json");
// }
//
// // ── main ──────────────────────────────────────────────────────────────────────
//
// int main(int argc, char *argv[]) {
//   std::string cert = "cert.pem";
//   std::string key = "key.pem";
//   std::uint16_t h2port = 8443;
//   std::uint16_t h3port = 8444;
//
//   if (argv != nullptr) {
//     if (argc > 1)
//       cert = argv[1];
//     if (argc > 2)
//       key = argv[2];
//     if (argc > 3)
//       h2port = static_cast<std::uint16_t>(std::stoi(argv[3]));
//     if (argc > 4)
//       h3port = static_cast<std::uint16_t>(std::stoi(argv[4]));
//   }
//
//   try {
//     ensure_certs(cert, key);
//   } catch (const std::exception &e) {
//     std::println(stderr, "Error: {}", e.what());
//     return 1;
//   }
//
//   // ── HTTP/2 TLS context (ALPN: h2, managed by server.tls) ─────────────────
//   transport::tls::SslCtx h2ctx;
//   try {
//     h2ctx = transport::tls::SslCtx::from_files(cert, key);
//   } catch (const std::exception &e) {
//     std::println(stderr, "TLS init failed: {}", e.what());
//     return 1;
//   }
//
//   // ── HTTP/3 server (ALPN: h3, TLS 1.3, QUIC via OpenSSL 3.6) ─────────────
//   // TLS context is managed internally by server::http3::Server —
//   // cert/key are passed directly to the constructor.
//   server::http3::Server h3srv{cert, key, h3port};
//   h3srv.get("/", handle);
//   h3srv.get("/health", handle);
//   h3srv.post("/echo", handle);
//   h3srv.get("/headers", handle);
//
//   // HTTP/3 event loop runs on a dedicated thread — OpenSSL QUIC is
//   // single-threaded per listener; the loop blocks on select().
//   std::jthread h3thread([&h3srv]() {
//     try {
//       h3srv.run();
//     } catch (const std::exception &e) {
//       std::println(stderr, "HTTP/3 fatal: {}", e.what());
//     }
//   });
//
//   std::println("HTTP/3 (QUIC) listening on :{}", h3port);
//
//   // ── HTTP/2 server (main thread, one std::thread per connection) ───────────
//   try {
//     auto h2srv = server::http2::Server::listen(h2port, h2ctx, handle);
//     std::println("HTTP/2  (TLS) listening on :{}", h2port);
//     std::println("Routes:");
//     std::println("  GET  /         — HTML home page");
//     std::println("  GET  /health   — JSON health check");
//     std::println("  POST /echo     — echo request body");
//     std::println("  GET  /headers  — dump request headers");
//     h2srv.run();
//   } catch (const std::exception &e) {
//     std::println(stderr, "HTTP/2 fatal: {}", e.what());
//     return 1;
//   }
//   // h3thread joins automatically via jthread destructor
// }
//
//
import std;

int main() {
  std::println("Hello, congelado!");
  return 0;
}
