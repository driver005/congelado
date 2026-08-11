export module core_router:executor;

import std;
import interfaces;
import shared;
import :router;

export namespace core::router {

/**
 * @brief Async request executor: the http2 dispatch enqueues a request here and returns, freeing
 * the receiver thread; this handler — registered as a `core::contract` worker like
 * `connector::Connector` — drains the queue on a worker thread, running `match()` (route + handler
 * + `send`) per request. Queued `IRequest`/`IResponse` are references into the live per-stream
 * objects the http2 `Session` owns (valid while the connection stays up during the match).
 */
class RouterExecutor : public shared::HandlerBase {
  public:
    /**
     * @brief Binds the executor to the route table it dispatches against.
     * @param route_handler the built `RouteHandler` to run `match()` on; must outlive this
     * executor (it's the http2 `Server`'s own `m_server`, sitting right next to this).
     */
    explicit RouterExecutor(RouteHandler<> *route_handler) noexcept
        : m_route_handler{route_handler} {}

    /**
     * @brief Installs the callback `enqueue()` fires to schedule this handler when it was idle —
     * the contract's `schedule()`. No-op if unset. Mirrors `connector::Connector::set_wake`.
     * @param wake the wake callback.
     */
    void set_wake(std::move_only_function<void()> wake) noexcept { m_wake = std::move(wake); }

    /// @brief Contract-handler identity.
    [[nodiscard]] std::string_view get_name() const noexcept override { return "router_executor"; }

    /**
     * @brief Queues a request for the executor to dispatch, then wakes the contract if it was
     * idle. Called by the http2 dispatch closure on the receiver thread, which returns straight
     * after — the actual `match()`/handler runs later on a worker thread.
     * @param req the request; a reference into the live stream, valid until its match completes.
     * @param res the response to fill in; same lifetime as `req`.
     * @param send the stream's reply callback, invoked by the handler (or the 404 path).
     */
    void enqueue(interfaces::io::IRequest &req, interfaces::io::IResponse &res,
                 std::function<void()> send) {
        std::lock_guard lock{m_pending_mutex};
        // Wake only on the idle->work transition, and not during a reentrant push.
        const bool was_idle = m_pending.empty();
        m_pending.push(PendingRequest{.req = &req, .res = &res, .send = std::move(send)});
        if (was_idle && !m_executing && m_wake) {
            m_wake();
        }
    }

    /**
     * @brief Checks whether the executor has any pending or in-flight work.
     * @return true if the queue is empty and no request is currently being matched/handled.
     */
    [[nodiscard]] bool is_idle() const noexcept {
        std::lock_guard lock{m_pending_mutex};
        return m_pending.empty() && !m_executing;
    }

    /**
     * @brief The per-tick work: pop one queued request, run `match()` on it (route + handler +
     * `send`), then self-reschedule if more remain. Empty queue → quietly idle until `enqueue()`
     * wakes it again. Mirrors `connector::Connector::on_execute`.
     * @return the callable the contract worker invokes on schedule.
     */
    shared::WorkerFunction on_execute() override {
        return [this]() {
            PendingRequest pending;
            {
                std::lock_guard lock{m_pending_mutex};
                if (m_pending.empty()) {
                    return;
                }
                pending = std::move(m_pending.front());
                m_pending.pop();
                m_executing = true;
            }

            // Route + run the handler; a routing miss (match() throws) becomes a 404.
            auto method = interfaces::io::types::parse_method(pending.req->get_method());
            try {
                m_route_handler->match(method, pending.req->get_path(), *pending.req, *pending.res,
                                       pending.send);
            } catch (const std::runtime_error &) {
                pending.res->set_status(interfaces::io::types::Status::NOT_FOUND);
                pending.send();
            }

            bool has_more = false;
            {
                std::lock_guard lock{m_pending_mutex};
                m_executing = false;
                has_more = !m_pending.empty();
            }
            if (has_more) {
                shared::this_handler::shedule();
            }
        };
    }

  private:
    struct PendingRequest {
        interfaces::io::IRequest *req{nullptr};
        interfaces::io::IResponse *res{nullptr};
        std::function<void()> send;
    };

    RouteHandler<> *m_route_handler;
    std::queue<PendingRequest> m_pending;
    mutable std::mutex m_pending_mutex;
    std::move_only_function<void()> m_wake;
    // True while on_execute() is running a popped request, so a reentrant enqueue() doesn't wake.
    bool m_executing{false};
};

} // namespace core::router
