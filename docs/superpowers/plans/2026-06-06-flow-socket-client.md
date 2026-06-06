# FlowSocket Client Abstraction Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Rename `FlowSocket` → `ServerFlowSocket` and add a symmetric `ClientFlowSocket` for outbound single-connection flow management.

**Architecture:** All three classes (`BaseSocket`, `ConnectorSocket`, `WorkerSocket`) are reused unchanged. `ClientFlowSocket` replaces `BaseSocket`'s listen/accept loop with a `sync_connect()` call in `build()`, then delegates to the same `ConnectorSocket` + `WorkerSocket` pipeline. The public API — `ConnectionEstablishedCallback`, `add_on_accept()`, `build()`, `on_send(fd)` — is identical between server and client variants.

**Tech Stack:** C++26 modules, xmake, `io::base::socket::Socket<TLS>`, `io::base::flow::sync`

---

## File Map

| File | Change |
|---|---|
| `include/io/flow/socket/sync.cppm` | rename `FlowSocket` → `ServerFlowSocket`; append `ClientFlowSocket` |
| `defaults/plugins/http2/http2.cc` | update one reference: `FlowSocket` → `ServerFlowSocket` |

---

### Task 1: Rename `FlowSocket` → `ServerFlowSocket` in `sync.cppm`

**Files:**
- Modify: `include/io/flow/socket/sync.cppm:252-368`

The class starts at line 252 with the comment `// Wrapper that connects...` and the rename touches the class name, constructor, destructor, and copy/move special members.

- [ ] **Step 1: Apply the rename**

In `include/io/flow/socket/sync.cppm`, make these exact text replacements:

```
// before → after
"class FlowSocket {"              → "class ServerFlowSocket {"
"FlowSocket(socket::Endpoint"     → "ServerFlowSocket(socket::Endpoint"
"~FlowSocket()"                   → "~ServerFlowSocket()"
"FlowSocket(const FlowSocket &)"  → "ServerFlowSocket(const ServerFlowSocket &)"
"FlowSocket &operator=(const FlowSocket &)" → "ServerFlowSocket &operator=(const ServerFlowSocket &)"
"FlowSocket(FlowSocket &&other)"  → "ServerFlowSocket(ServerFlowSocket &&other)"
"FlowSocket &operator=(FlowSocket &&other)" → "ServerFlowSocket &operator=(ServerFlowSocket &&other)"
```

After these replacements the class declaration block (lines ~252–287) should read:

```cpp
// Wrapper that connects the base cocket to the worker and manages the types for the thread model.
template <shared::HandlerController Controller, socket::Protocol protocol>
class ServerFlowSocket {
  public:
    using ConnectionEstablishedCallback =
        std::move_only_function<shared::ReadCallback(shared::SendCallback, shared::CloseCallback)>;

    ServerFlowSocket(socket::Endpoint end, Leverager &leverager, Controller &controller)
        : m_base_socket{std::move(end), leverager}, m_leverager{leverager}, m_controller{controller}, m_workers{},
          m_on_established{nullptr} {
    };

    ~ServerFlowSocket() {
        m_base_socket.set_closed();
        core::logger::debug("io/flow", "closing base socket {}", m_base_socket.get_endpoint().to_string());
        for (auto &[fd, worker] : m_workers) {
            worker->close();
        }
    }

    ServerFlowSocket(const ServerFlowSocket &) = delete;
    ServerFlowSocket &operator=(const ServerFlowSocket &) = delete;
    ServerFlowSocket(ServerFlowSocket &&other)
        : m_base_socket{std::move(other.m_base_socket)}, m_leverager{std::move(other.m_leverager)},
          m_controller{std::move(other.m_controller)}, m_workers{std::move(other.m_workers)},
          m_on_established(std::move(other.m_on_established)) {}
    ServerFlowSocket &operator=(ServerFlowSocket &&other) {
        if (this != &other) {
            m_base_socket = std::move(other.m_base_socket);
            m_leverager = std::move(other.m_leverager);
            m_controller = std::move(other.m_controller);
            m_workers = std::move(other.m_workers);
            m_on_established = std::move(other.m_on_established);
        }
        return *this;
    };
```

- [ ] **Step 2: Build to verify rename compiles**

```bash
xmake build 2>&1 | tail -20
```

Expected: build fails because `defaults/plugins/http2/http2.cc` still references `FlowSocket`. Error will name that file. If any other error appears, fix it before continuing.

---

### Task 2: Update caller in `http2.cc`

**Files:**
- Modify: `defaults/plugins/http2/http2.cc:90`

- [ ] **Step 1: Update the type reference**

In `defaults/plugins/http2/http2.cc`, line 90 currently reads:

```cpp
    std::optional<io::base::flow::sync::FlowSocket<core::contract::ContractGroup<>,
                                                   io::base::socket::Protocol::TLS>>
        m_socket_flow;
```

Change to:

```cpp
    std::optional<io::base::flow::sync::ServerFlowSocket<core::contract::ContractGroup<>,
                                                         io::base::socket::Protocol::TLS>>
        m_socket_flow;
```

- [ ] **Step 2: Build to verify**

```bash
xmake build 2>&1 | tail -20
```

Expected: build succeeds with no errors.

- [ ] **Step 3: Commit**

```bash
git add include/io/flow/socket/sync.cppm defaults/plugins/http2/http2.cc
git commit -m "refactor(flow): rename FlowSocket to ServerFlowSocket"
```

---

### Task 3: Add `ClientFlowSocket` to `sync.cppm`

**Files:**
- Modify: `include/io/flow/socket/sync.cppm` — append before the closing `}`  of namespace `io::base::flow::sync`

- [ ] **Step 1: Append `ClientFlowSocket` before the closing namespace brace**

The file currently ends (lines ~367–369) with:

```cpp
    ConnectionEstablishedCallback m_on_established;
};

} // namespace io::base::flow::sync
```

Insert the new class between the closing `};` of `ServerFlowSocket` and `} // namespace io::base::flow::sync`:

```cpp
    ConnectionEstablishedCallback m_on_established;
};

template <shared::HandlerController Controller, socket::Protocol protocol>
class ClientFlowSocket {
  public:
    using ConnectionEstablishedCallback =
        std::move_only_function<shared::ReadCallback(shared::SendCallback, shared::CloseCallback)>;

    ClientFlowSocket(socket::Endpoint end, Leverager &leverager, Controller &controller)
        : m_endpoint{std::move(end)}, m_leverager{leverager}, m_controller{controller},
          m_workers{}, m_on_established{nullptr} {}

    ~ClientFlowSocket() {
        for (auto &[fd, worker] : m_workers) {
            worker->close();
        }
    }

    ClientFlowSocket(const ClientFlowSocket &) = delete;
    ClientFlowSocket &operator=(const ClientFlowSocket &) = delete;
    ClientFlowSocket(ClientFlowSocket &&other) noexcept
        : m_endpoint{std::move(other.m_endpoint)}, m_leverager{std::move(other.m_leverager)},
          m_controller{std::move(other.m_controller)}, m_workers{std::move(other.m_workers)},
          m_on_established{std::move(other.m_on_established)} {}
    ClientFlowSocket &operator=(ClientFlowSocket &&other) noexcept {
        if (this != &other) {
            m_endpoint = std::move(other.m_endpoint);
            m_leverager = std::move(other.m_leverager);
            m_controller = std::move(other.m_controller);
            m_workers = std::move(other.m_workers);
            m_on_established = std::move(other.m_on_established);
        }
        return *this;
    }

    void add_on_accept(ConnectionEstablishedCallback established) & {
        m_on_established = std::move(established);
    }

    void build() & {
        if (!m_on_established) {
            throw std::runtime_error(
                "ConnectionEstablished callback must be set before building the ClientFlowSocket");
        }
        helper();
    }

    shared::SendCallback on_send(socket::SOCKET fd) {
        auto value = m_workers.find(fd);
        if (value) {
            return [sender = &value->get_sender()](utils::buffering::BufferNode &&node) {
                sender->send(std::move(node));
            };
        }
        return nullptr;
    }

  private:
    void helper() {
        socket::Socket<protocol> sock{m_endpoint};
        auto connect_status = sock.sync_connect();
        if (connect_status.get_status() != socket::VALUES::VALID) {
            core::logger::error("io/flow/client", "connect to {} failed", m_endpoint.to_string());
            return;
        }
        sock.set_non_blocking();

        auto connector = std::make_unique<ConnectorSocket<protocol>>(
            std::move(sock), [this](socket::Socket<protocol> encrypted_socket) mutable {
                auto worker = std::make_shared<WorkerSocket<protocol>>(
                    std::move(encrypted_socket),
                    [this](socket::SOCKET fd, int err) {
                        m_workers.erase(fd);
                        std::println("Error while sending data on socket {}: {}", fd, err);
                    },
                    [this](socket::SOCKET fd, int err) {
                        m_workers.erase(fd);
                        std::println("Error while receiving data on socket {}: {}", fd, err);
                    });

                auto read_callback = m_on_established(
                    [worker](utils::buffering::BufferNode &&node) {
                        worker->get_sender().send(std::move(node));
                    },
                    [this, worker]() {
                        core::logger::info("io/worker", "fd {} closed", worker->get_fd());
                        m_workers.erase(worker->get_fd());
                    });

                worker->add_on_read(std::move(read_callback));
                worker->build();
                worker->template start<Controller>(m_controller);

                core::logger::debug("io/flow/client", "fd {} connected to {}",
                                    worker->get_fd(), m_endpoint.to_string());

                m_workers.insert(worker->get_fd(), std::move(worker));
            });

        connector->template create<Controller>(m_controller);
        connector->what_this_is_me(std::move(connector));
    }

    socket::Endpoint m_endpoint;
    std::reference_wrapper<Leverager> m_leverager;  // kept for API symmetry with ServerFlowSocket; unused internally
    std::reference_wrapper<Controller> m_controller;
    hashmap::swiss::SwissHashMap<socket::SOCKET, std::shared_ptr<WorkerSocket<protocol>>> m_workers;
    ConnectionEstablishedCallback m_on_established;
};

} // namespace io::base::flow::sync
```

- [ ] **Step 2: Build to verify**

```bash
xmake build 2>&1 | tail -30
```

Expected: build succeeds with no errors. If `socket::VALUES::VALID` is not found, check the import — the module is `io_base_socket`, already imported at line 3 of `sync.cppm`. If `sync_connect()` returns a different status type, check `EngineClient`'s usage at `include/worker/engine_client.cppm:42` for the exact return type and accessor.

- [ ] **Step 3: Commit**

```bash
git add include/io/flow/socket/sync.cppm
git commit -m "feat(flow): add ClientFlowSocket for outbound single-connection flow"
```
