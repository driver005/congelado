# Client Abstraction Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add `ClientHandler<Derived>` to `core/client/` — a CRTP, zero-cost HTTP client abstraction that mirrors `RouteHandler<Derived>`, wired to `EngineClient` via two connector methods.

**Architecture:** `ClientHandler<Derived>` lives in `core/client/client.cppm`. It exposes `send<Protocol>` and verb methods (`get`, `post`, etc.) at class level without a Protocol template parameter — Protocol is resolved at call site. `EngineClient` inherits `ClientHandler<EngineClient>` and provides `do_send<Protocol>` (connects to `Session::send`) and `make_request<Protocol>` (factory returning `unique_ptr<IRequest<Protocol>>`). `WorkerContext` calls through the `ClientHandler` API.

**Tech Stack:** C++26 modules, CRTP, `interfaces::IRequest<Protocol>`, `io::layer::http2::HttpRequest`, xmake build (auto-discovers `include/**/*.cppm`).

---

### Task 1: Create `include/core/client/client.cppm`

**Files:**
- Create: `include/core/client/client.cppm`

- [ ] **Step 1: Create the file**

```cpp
module;

export module core_client;

import std;
import interfaces;

export namespace core::client {

template <typename Derived>
class ClientHandler {
  public:
    using ResponseFn = std::function<void(int status, std::string body)>;

    // send<Protocol> — Protocol at call site, dispatches to Derived::do_send
    template <typename Protocol>
    void send(interfaces::IRequest<Protocol>& req, ResponseFn cb) {
        static_cast<Derived&>(*this).do_send(req, std::move(cb));
    }

    // Generic factory — method as runtime string; body optional
    template <typename Protocol>
    auto request(std::string_view method, std::string_view path,
                 std::string_view body = {}) {
        return static_cast<Derived&>(*this).template make_request<Protocol>(method, path, body);
    }

    // Verb convenience — all body-free; use request<P> directly for body
    template <typename Protocol>
    auto get(std::string_view path)   { return request<Protocol>("GET",    path); }
    template <typename Protocol>
    auto post(std::string_view path)  { return request<Protocol>("POST",   path); }
    template <typename Protocol>
    auto put(std::string_view path)   { return request<Protocol>("PUT",    path); }
    template <typename Protocol>
    auto del(std::string_view path)   { return request<Protocol>("DELETE", path); }
    template <typename Protocol>
    auto patch(std::string_view path) { return request<Protocol>("PATCH",  path); }
    template <typename Protocol>
    auto head(std::string_view path)  { return request<Protocol>("HEAD",   path); }
};

} // namespace core::client
```

- [ ] **Step 2: Verify it compiles in isolation**

```bash
xmake build congelado_lib 2>&1 | head -30
```

Expected: either success or only errors from other modified files (not from `core_client` itself).

- [ ] **Step 3: Commit**

```bash
git add include/core/client/client.cppm
git commit -m "feat(core): add ClientHandler<Derived> CRTP client abstraction"
```

---

### Task 2: Refactor `include/worker/engine_client.cppm`

**Files:**
- Modify: `include/worker/engine_client.cppm`

Four sub-changes in one file: (a) import + inherit, (b) host/port constructor, (c) rename private helper, (d) add CRTP connectors, (e) remove old `call()`.

- [ ] **Step 1: Add import and inheritance, rename private helper, update constructor**

Replace the full file with:

```cpp
module;
#include <ranges>

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

  private:
    struct PendingRequest {
        ResponseFn callback;
    };

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
        m_socket.sync_connect();
    }

    void set_self_contract(core::contract::Contract<> contract) {
        m_self_contract = std::move(contract);
    }

    // CRTP connector 1 — called by ClientHandler::send<Protocol>
    template <typename Protocol>
    void do_send(interfaces::IRequest<Protocol>& req, ResponseFn cb) {
        auto& http_req = static_cast<io::layer::http2::HttpRequest&>(req);
        m_session.send(http_req);
        m_pending.emplace(http_req.get_stream_id(), std::move(cb));
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
```

> **Note on `OutboundRequest`:** The old `OutboundRequest` stored `method/path/body` strings and built requests in `drain_outbound`. The new version stores the already-built `unique_ptr<HttpRequest>` since `make_request<Protocol>` builds it upfront. If the outbound queue path is unused after this refactor (i.e., `call()` is gone and requests go directly via `do_send`), simplify accordingly — `OutboundRequest` and `drain_outbound` can be removed if `do_send` enqueues directly.

- [ ] **Step 2: Build and fix errors**

```bash
xmake build congelado_lib 2>&1 | head -60
```

Expected: compiles cleanly. Common errors to fix:
- `HttpRequest::get_stream_id()` — verify method name (may be `get_id()` or similar; check `include/io/layer/http2/request.cppm`)
- `m_socket.sync_connect()` return value — `SocketStatus`; if connection fails it logs internally, no action needed in constructor
- `OutboundRequest` struct field mismatch — adjust to match actual `m_outbound` usage

- [ ] **Step 3: Commit**

```bash
git add include/worker/engine_client.cppm
git commit -m "feat(worker): refactor EngineClient to use ClientHandler<Derived>"
```

---

### Task 3: Update `include/worker/context.cppm`

**Files:**
- Modify: `include/worker/context.cppm`

- [ ] **Step 1: Update the file**

```cpp
export module worker:context;

import std;
import core_logger;
import io_shared;
import :task_worker;
import :engine_client;

export namespace worker {

class WorkerContext {
  public:
    using EngineResponseFn = std::function<void(int status, std::string body)>;

    void set_worker_id(std::string_view worker_id) { m_worker_id = worker_id; }

    void set_engine_client(EngineClient &client) noexcept { m_engine_client = &client; }

    void add_task_worker(ITaskWorker *worker) {
        for (auto &entry : m_workers) {
            if (entry->get_task_type() == worker->get_task_type()) {
                entry = worker;
                return;
            }
        }
        m_workers.push_back(worker);
    }

    void call_engine(std::string_view method, std::string_view path, std::string_view body,
                     EngineResponseFn callback) noexcept {
        if (!m_engine_client) {
            core::logger::error("worker/context", "call_engine before engine client is set");
            return;
        }
        using P = io::shared::http::Protocol;
        auto req = m_engine_client->request<P>(method, path, body);
        std::promise<std::pair<int, std::string>> promise;
        auto future = promise.get_future();
        m_engine_client->send<P>(*req,
            [p = std::move(promise)](int status, std::string response_body) mutable {
                p.set_value({status, std::move(response_body)});
            });
        try {
            auto [status, response_body] = future.get();
            callback(status, std::move(response_body));
        } catch (...) {
            core::logger::error("worker/context", "engine call failed");
            callback(500, "engine communication error");
        }
    }

    [[nodiscard]] std::string_view get_worker_id() const noexcept { return m_worker_id; }

    [[nodiscard]] ITaskWorker *get_task_worker(std::string_view task_type) const noexcept {
        for (auto *entry : m_workers) {
            if (entry->get_task_type() == task_type) {
                return entry;
            }
        }
        return nullptr;
    }

    [[nodiscard]] std::optional<TaskOutput> run_task(std::string_view task_type,
                                                     TaskInput const &input) {
        auto *worker = get_task_worker(task_type);
        if (worker == nullptr) return std::nullopt;
        auto release  = worker->on_released();
        auto on_error = worker->on_error();
        try {
            auto output = worker->execute(input);
            if (release) release();
            return output;
        } catch (...) {
            if (on_error) on_error(std::current_exception());
            if (release) release();
            return std::nullopt;
        }
    }

    [[nodiscard]] std::vector<std::string_view> get_task_types() const noexcept {
        std::vector<std::string_view> types;
        types.reserve(m_workers.size());
        for (auto *entry : m_workers) {
            types.push_back(entry->get_task_type());
        }
        return types;
    }

  private:
    EngineClient *m_engine_client{nullptr};
    std::string m_worker_id;
    std::vector<ITaskWorker *> m_workers;
};

} // namespace worker
```

- [ ] **Step 2: Build**

```bash
xmake build congelado_lib 2>&1 | head -60
```

Expected: clean build.

- [ ] **Step 3: Commit**

```bash
git add include/worker/context.cppm
git commit -m "feat(worker): update WorkerContext to use ClientHandler send/request API"
```

---

### Task 4: Final build verification

- [ ] **Step 1: Full clean build**

```bash
xmake clean && xmake build congelado_lib 2>&1 | tail -20
```

Expected: no errors. Warnings about unused variables or debug `std::println` in router are pre-existing — ignore.

- [ ] **Step 2: Verify module exports are correct**

```bash
xmake build congelado 2>&1 | head -40
```

Expected: binary target also builds cleanly.

- [ ] **Step 3: Commit if any fixups were needed**

```bash
git add -p
git commit -m "fix(client): resolve compilation issues from client abstraction refactor"
```

---

## Self-Review Notes

**Spec coverage:**
- ✅ `ClientHandler<Derived>` in `core/client/client.cppm`
- ✅ Protocol template on function, not class
- ✅ `do_send<Protocol>` connector (transport hook)
- ✅ `make_request<Protocol>` connector (factory)
- ✅ `EngineClient` inherits `ClientHandler<EngineClient>` + `HandlerBase`
- ✅ Host + port constructor (blocking `sync_connect()` in body)
- ✅ Private `make_request()` renamed to `build_http_request()`
- ✅ `WorkerContext::call_engine()` uses `request<P>` + `send<P>`

**Known implementation risks:**
- `OutboundRequest` in the new `EngineClient` — the old queue-based outbound path was driven by the old `call()`. With CRTP `do_send`, requests may go directly to the session. If `m_outbound` queue is no longer used, remove it and simplify `drain_outbound` / `on_execute`.
- `HttpRequest::get_stream_id()` — name must be verified against actual `HttpRequest` definition.
- `TlsSocket` move semantics — `m_socket{Endpoint{host, port}}` creates the socket; `sync_connect()` must be called after all session/handshake members are initialized (constructor body is correct placement).
