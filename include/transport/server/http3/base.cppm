// module;
// #ifdef _WIN32
// #ifndef WIN32_LEAN_AND_MEAN
// #define WIN32_LEAN_AND_MEAN
// #endif
// #include <winsock2.h>
// #include <ws2tcpip.h>
// #pragma comment(lib, "ws2_32.lib")
// #else
// #include <arpa/inet.h>
// #include <fcntl.h>
// #include <netinet/in.h>
// #include <sys/select.h>
// #include <sys/socket.h>
// #include <unistd.h>
// #endif
// #include <cerrno>
// #include <cstring>
// #include <openssl/quic.h>
// #include <openssl/ssl.h>
// export module server.http3;
//
// import std;
// import quic;
// import server.http2;
//
// // HTTP/3 server (RFC 9114) on top of OpenSSL 3.6 native QUIC.
// //
// // Accept model:
// //   SSL_new_listener()         — passive QUIC endpoint on the UDP fd
// //   SSL_accept_connection()    — one Connection SSL* per QUIC client
// //   Connection::tick()         — drives each connection's QUIC state machine
// //   SSL_get_event_timeout()    — schedules next tick (retransmits, idle)
// //
// // HTTP/3 layer:
// //   Each connection gets a Session.
// //   Session opens a server control stream and sends SETTINGS.
// //   Incoming bidi streams carry H3 HEADERS + DATA frames.
// //   HEADERS decoded via QPACK static table, routed to handler, response sent.
// //
// // QPACK: OpenSSL has no QPACK API — it operates at QUIC transport layer only.
// // We implement static-table-only QPACK in quic:qpack (no dynamic table).
//
// namespace server::http3 {
//
// // ── H3 frame types (RFC 9114 §7.2) ───────────────────────────────────────────
//
// enum class H3FrameType : quic::VarInt {
//   Data = 0x00,
//   Headers = 0x01,
//   Settings = 0x04,
//   GoAway = 0x07,
// };
//
// // ── Frame builder ─────────────────────────────────────────────────────────────
//
// static std::vector<std::byte> build_frame(H3FrameType type, std::span<const std::byte> payload) {
//   std::array<std::byte, 16> hdr{};
//   quic::ByteWriter w{hdr};
//   w.write_varint(static_cast<quic::VarInt>(type));
//   w.write_varint(payload.size());
//   std::vector<std::byte> out;
//   out.reserve(w.written() + payload.size());
//   out.insert(out.end(), hdr.data(), hdr.data() + w.written());
//   out.insert(out.end(), payload.begin(), payload.end());
//   return out;
// }
//
// // ── Stream accumulator ────────────────────────────────────────────────────────
//
// struct StreamAccum {
//   std::vector<std::byte> buf;
//   server::http2::Request req;
//   bool headers_done{false};
//   quic::qpack::Decoder qpack;
// };
//
// static bool parse_h3_frames(StreamAccum &acc) {
//   auto &buf = acc.buf;
//   while (!buf.empty()) {
//     quic::ByteReader r{buf};
//     quic::VarInt type_val, length;
//     if (!r.read_varint(type_val))
//       break;
//     if (!r.read_varint(length))
//       break;
//     const auto plen = static_cast<std::size_t>(length);
//     if (r.remaining() < plen)
//       break;
//
//     const auto payload = r.read_bytes(plen);
//     const auto ft = static_cast<H3FrameType>(type_val);
//
//     if (ft == H3FrameType::Headers && !acc.headers_done) {
//       auto decoded = acc.qpack.decode(payload);
//       if (!decoded)
//         return false;
//       for (auto &[name, value] : *decoded) {
//         if (name == ":method")
//           acc.req.method = value;
//         else if (name == ":path")
//           acc.req.path = value;
//         else
//           acc.req.headers.push_back({name, value});
//       }
//       acc.headers_done = true;
//     } else if (ft == H3FrameType::Data) {
//       acc.req.body.insert(acc.req.body.end(), payload.begin(), payload.end());
//     } else if (ft == H3FrameType::GoAway) {
//       return false;
//     }
//     // Settings + unknown frames silently skipped (RFC 9114 §9)
//
//     buf.erase(buf.begin(), buf.begin() + static_cast<std::ptrdiff_t>(r.pos()));
//   }
//   return true;
// }
//
// // ── Session — per-connection HTTP/3 state ────────────────────────────────────
//
// class Session {
// public:
//   explicit Session(quic::Connection &conn, server::http2::Handler handler)
//       : m_conn(conn), m_handler(std::move(handler)) {
//
//     // Send H3 control stream immediately after QUIC handshake completes,
//     // before any request streams arrive. Avoids iterator invalidation
//     // (send_stream inserts into m_streams) and OpenSSL re-entrancy issues.
//     m_conn.on_connected([this]() { send_control_stream(); });
//   }
//
//   void on_stream_data(std::uint64_t id, std::vector<std::byte> data, bool fin) {
//     // Unidirectional streams are client control/push — ignore.
//     if (quic::stream_is_uni(id))
//       return;
//
//     auto &acc = m_streams[id];
//     acc.buf.insert(acc.buf.end(), data.begin(), data.end());
//     parse_h3_frames(acc);
//
//     if (fin && acc.headers_done) {
//       respond(id, acc.req);
//       m_streams.erase(id);
//     }
//   }
//
// private:
//   void send_control_stream() {
//     // Unidirectional stream: 1 byte stream type (0x00 = control) + SETTINGS.
//     auto settings = build_frame(H3FrameType::Settings, {});
//     std::vector<std::byte> ctrl;
//     ctrl.push_back(static_cast<std::byte>(0x00));
//     ctrl.insert(ctrl.end(), settings.begin(), settings.end());
//     // fin=false — control stream must remain open for connection lifetime.
//     m_conn.send_stream(ctrl, /*fin=*/false, /*unidirectional=*/true);
//   }
//
// private:
//   void respond(std::uint64_t stream_id, const server::http2::Request &req) {
//     server::http2::Response resp;
//     try {
//       resp = m_handler(req);
//     } catch (...) {
//       resp = server::http2::Response::text(500, "Internal Server Error");
//     }
//
//     // Build QPACK-encoded HEADERS frame.
//     std::vector<std::pair<std::string, std::string>> hdrs;
//     hdrs.push_back({":status", std::to_string(resp.status)});
//     for (auto &[k, v] : resp.headers)
//       hdrs.push_back({k, v});
//     if (!resp.body.empty())
//       hdrs.push_back({"content-length", std::to_string(resp.body.size())});
//
//     quic::qpack::Encoder enc;
//     const auto qblock = enc.encode(hdrs);
//     auto hframe = build_frame(H3FrameType::Headers, qblock);
//
//     std::vector<std::byte> wire;
//     wire.insert(wire.end(), hframe.begin(), hframe.end());
//
//     if (!resp.body.empty()) {
//       auto dframe = build_frame(H3FrameType::Data,
//                                 std::span{reinterpret_cast<const std::byte *>(resp.body.data()), resp.body.size()});
//       wire.insert(wire.end(), dframe.begin(), dframe.end());
//     }
//
//     m_conn.write_stream(stream_id, wire, /*fin=*/true);
//   }
//
//   quic::Connection &m_conn;
//   server::http2::Handler m_handler;
//   std::unordered_map<std::uint64_t, StreamAccum> m_streams;
// };
//
// // ── Server ────────────────────────────────────────────────────────────────────
//
// export class Server {
// public:
//   explicit Server(std::string_view cert_pem, std::string_view key_pem, std::uint16_t port = 443)
//       : m_tls(quic::tls::TlsContext::from_files(cert_pem, key_pem)), m_port(port) {}
//
//   void get(std::string_view p, server::http2::Handler h) { route("GET", p, h); }
//   void post(std::string_view p, server::http2::Handler h) { route("POST", p, h); }
//
//   void route(std::string_view method, std::string_view path, server::http2::Handler h) {
//     m_routes[std::string(method)][std::string(path)] = std::move(h);
//   }
//
//   void run() {
//     // Create and bind a non-blocking UDP socket.
// #ifdef _WIN32
//     WSADATA wsa{};
//     ::WSAStartup(MAKEWORD(2, 2), &wsa);
//     const SOCKET raw = ::socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
//     if (raw == INVALID_SOCKET)
//       throw std::runtime_error("socket() failed");
//     u_long nb = 1;
//     ::ioctlsocket(raw, FIONBIO, &nb);
//     const int fd = static_cast<int>(raw);
// #else
//     const int fd = ::socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
//     if (fd < 0)
//       throw std::runtime_error(std::string("socket() failed: ") + std::strerror(errno));
//     ::fcntl(fd, F_SETFL, ::fcntl(fd, F_GETFL) | O_NONBLOCK);
// #endif
//
//     // Allow dual-stack (IPv4 + IPv6) and port reuse.
//     {
//       int off = 0;
//       ::setsockopt(fd, IPPROTO_IPV6, IPV6_V6ONLY, reinterpret_cast<const char *>(&off), sizeof(off));
//       int on = 1;
//       ::setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char *>(&on), sizeof(on));
//       int rcvbuf = 4 * 1024 * 1024;
//       ::setsockopt(fd, SOL_SOCKET, SO_RCVBUF, reinterpret_cast<const char *>(&rcvbuf), sizeof(rcvbuf));
//     }
//
//     sockaddr_in6 addr{};
//     addr.sin6_family = AF_INET6;
//     addr.sin6_addr = in6addr_any;
//     addr.sin6_port = htons(m_port);
//     if (::bind(fd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) != 0)
//       throw std::runtime_error(std::string("bind() failed: ") + std::strerror(errno));
//
//     // Create QUIC listener — OpenSSL reads/writes UDP packets on fd directly.
//     SSL *listener = ::SSL_new_listener(m_tls.get(), 0);
//     if (!listener)
//       throw std::runtime_error("SSL_new_listener failed");
//     ::SSL_set_fd(listener, fd);
//
//     // Non-blocking mode is required for QUIC — inherited by all accepted
//     // connection SSL objects. Without this, SSL_accept_connection blocks
//     // indefinitely instead of returning NULL when no client is waiting.
//     if (::SSL_set_blocking_mode(listener, 0) != 1)
//       throw std::runtime_error("SSL_set_blocking_mode failed");
//
//     if (::SSL_listen(listener) != 1)
//       throw std::runtime_error("SSL_listen failed");
//
//     struct ListenerGuard {
//       SSL *l;
//       ~ListenerGuard() { ::SSL_free(l); }
//     } lg{listener};
// #ifdef _WIN32
//     struct FdGuard {
//       SOCKET s;
//       ~FdGuard() { ::closesocket(s); }
//     } fg{raw};
// #else
//     struct FdGuard {
//       int s;
//       ~FdGuard() { ::close(s); }
//     } fg{fd};
// #endif
//
//     using ConnPtr = std::unique_ptr<quic::Connection>;
//     using SessPtr = std::unique_ptr<Session>;
//
//     std::vector<ConnPtr> conns;
//     std::unordered_map<quic::Connection *, SessPtr> sessions;
//
//     for (;;) {
//       // Accept all pending new QUIC connections (non-blocking).
//       for (;;) {
//         SSL *csssl = ::SSL_accept_connection(listener, SSL_ACCEPT_CONNECTION_NO_BLOCK);
//         if (!csssl)
//           break;
//
//         auto conn = std::make_unique<quic::Connection>(csssl);
//         auto sess = std::make_unique<Session>(*conn, make_handler());
//         // Session ctor wires on_connected (control stream) and we wire on_stream.
//         Session *raw = sess.get();
//         conn->on_stream([raw](std::uint64_t id, std::vector<std::byte> data, bool fin) {
//           raw->on_stream_data(id, std::move(data), fin);
//         });
//
//         sessions[conn.get()] = std::move(sess);
//         conns.push_back(std::move(conn));
//       }
//
//       // Tick connections, compute earliest SSL timer deadline.
//       struct timeval earliest{1, 0};
//       for (auto &c : conns) {
//         c->tick();
//         struct timeval tv{};
//         int infinite = 0;
//         if (::SSL_get_event_timeout(c->native(), &tv, &infinite) == 1 && !infinite && timercmp(&tv, &earliest, <))
//           earliest = tv;
//       }
//
//       // Prune closed connections and their sessions.
//       std::erase_if(conns, [&](const ConnPtr &c) {
//         if (c->state() != quic::ConnState::Closed)
//           return false;
//         sessions.erase(c.get());
//         return true;
//       });
//
//       // Sleep until the fd is readable/writable or the earliest timer fires.
//       // Use SSL_net_read_desired / SSL_net_write_desired on each connection
//       // (and the listener) to decide which directions to watch — the QUIC
//       // state machine may need to write (retransmits) even with no new reads.
//       fd_set rfds, wfds;
//       FD_ZERO(&rfds);
//       FD_ZERO(&wfds);
//
//       // Always watch the listener for new incoming connections.
//       if (::SSL_net_read_desired(listener))
//         FD_SET(fd, &rfds);
//       if (::SSL_net_write_desired(listener))
//         FD_SET(fd, &wfds);
//       // Fallback: if neither flag is set, still check for readability.
//       if (!FD_ISSET(fd, &rfds) && !FD_ISSET(fd, &wfds))
//         FD_SET(fd, &rfds);
//
//       ::select(fd + 1, &rfds, &wfds, nullptr, &earliest);
//     }
//   }
//
// private:
//   server::http2::Handler make_handler() const {
//     return [this](const server::http2::Request &req) -> server::http2::Response {
//       auto mit = m_routes.find(req.method);
//       if (mit == m_routes.end())
//         return server::http2::Response::text(405, "Method Not Allowed");
//       auto pit = mit->second.find(req.path);
//       if (pit == mit->second.end())
//         return server::http2::Response::text(404, "Not Found");
//       return pit->second(req);
//     };
//   }
//
//   quic::tls::TlsContext m_tls;
//   std::uint16_t m_port;
//   std::unordered_map<std::string, std::unordered_map<std::string, server::http2::Handler>> m_routes;
// };
//
// } // namespace server::http3
