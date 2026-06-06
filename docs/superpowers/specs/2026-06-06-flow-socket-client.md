# FlowSocket Client Abstraction Design

**Date:** 2026-06-06
**Status:** Approved

## Goal

Rename `FlowSocket` → `ServerFlowSocket` and add a symmetric `ClientFlowSocket` for outbound single-connection flow management. Both live in `include/io/flow/socket/sync.cppm`.

## Context

`FlowSocket<Controller, Protocol>` in `io::base::flow::sync` is a server-side orchestrator:
- `BaseSocket<protocol>` — listens, accepts inbound TCP connections
- `ConnectorSocket<protocol>` — async TLS handshake per accepted connection
- `WorkerSocket<protocol>` — per-connection send+receive via `Sender`/`Receiver`

The name `FlowSocket` is ambiguous now that there is also a client-side use case. Renaming to `ServerFlowSocket` makes the intent explicit and mirrors the new `ClientFlowSocket`.

## Design

### Rename: `FlowSocket` → `ServerFlowSocket`

In `include/io/flow/socket/sync.cppm`: rename the class. No behavior change.

Caller update:
- `defaults/plugins/http2/http2.cc:90` — `FlowSocket` → `ServerFlowSocket`

### New: `ClientFlowSocket<Controller, Protocol>`

Symmetric API to `ServerFlowSocket`:

| Member | `ServerFlowSocket` | `ClientFlowSocket` |
|---|---|---|
| `ConnectionEstablishedCallback` | same type | same type |
| `add_on_accept(cb)` | sets `m_on_established` | same |
| `build()` | validates + `helper()` + `start()` | validates + `helper()` |
| `on_send(fd)` | looks up worker in map | same |
| `m_workers` | `SwissHashMap<SOCKET, shared_ptr<WorkerSocket>>` | same |

**Difference from `ServerFlowSocket`:**
- No `BaseSocket` — stores `socket::Endpoint`, creates `Socket<protocol>` inline in `helper()`
- No `start()` — no accept loop to schedule; one-shot connect
- `helper()` flow:
  1. Construct `socket::Socket<protocol>{m_endpoint}`, call `set_non_blocking()` then `sync_connect()`
  2. Create `ConnectorSocket<protocol>` with the connected socket (reused as-is — `sync_handshake()` is symmetric)
  3. `ConnectorSocket` on success: create `WorkerSocket`, call `m_on_established`, insert into `m_workers`
  4. Schedule `ConnectorSocket` on `m_controller`

```cpp
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
        if (!m_on_established)
            throw std::runtime_error(
                "ConnectionEstablished callback must be set before building the ClientFlowSocket");
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
        sock.set_non_blocking();
        auto connect_status = sock.sync_connect();
        if (connect_status.get_status() != socket::VALUES::VALID) {
            core::logger::error("io/flow/client", "connect to {} failed", m_endpoint.to_string());
            return;
        }

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
```

## File Summary

| File | Change |
|---|---|
| `include/io/flow/socket/sync.cppm` | rename `FlowSocket` → `ServerFlowSocket`, add `ClientFlowSocket` |
| `defaults/plugins/http2/http2.cc` | `FlowSocket` → `ServerFlowSocket` |

## Pattern Comparison

| | `ServerFlowSocket` | `ClientFlowSocket` |
|---|---|---|
| Connection source | `BaseSocket` accept loop | `sync_connect()` in `helper()` |
| TLS handshake | `ConnectorSocket` (reused) | `ConnectorSocket` (reused) |
| I/O | `WorkerSocket` (reused) | `WorkerSocket` (reused) |
| `build()` | `helper()` + `start()` | `helper()` only |
| `m_workers` entries | N (one per accepted client) | 1 (single outbound connection) |
