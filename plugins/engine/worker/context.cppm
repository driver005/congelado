export module worker:context;

import std;
import core_logger;
import core_client;
import interfaces;
// TODO: interfaces/core_logger are still imported directly here (not via sdk) — the
// task-execution core already moved into congelado_worker; the HTTP-client-to-engine half
// below is this module's own remaining concern ("the API part"), but should eventually
// also route through sdk re-exports once those exist.
import congelado_worker;

export namespace worker {

using TaskInput = congelado::worker::TaskInput;
using TaskOutput = congelado::worker::TaskOutput;
using ITaskWorker = congelado::worker::ITaskWorker;

// Worker identity + custom task worker registry + engine communication.
// Task loading/registry is delegated to congelado::worker::TaskRunner (sdk-owned, no HTTP
// surface) — this class adds the HTTP-client-to-engine half on top ("the API part"), which
// is deliberately excluded from the sdk-owned TaskRunner.
// Workers are stateless — no database or cache; all persistence lives on the engine side.
class WorkerContext {
  public:
    struct EngineResponse {
        int m_status{0};
        std::string m_body;
    };

    /** @brief Default ctor — no worker id set yet, TaskRunner starts out empty. */
    WorkerContext() = default;
    /**
     * @brief Builds a worker context already carrying an id, forwarded straight into the
     * underlying TaskRunner.
     * @param worker_id the id this worker identifies as.
     */
    explicit WorkerContext(std::string_view worker_id) : m_task_runner(worker_id) {}

    /**
     * @brief Deleted — this thing owns a std::mutex and hands out `this`-capturing dispatch
     * callbacks via make_dispatch(), so copying would leave stale callbacks pointing at a ghost.
     */
    WorkerContext(WorkerContext const &) = delete;
    /** @brief Deleted, same reasoning as the copy ctor right above. */
    WorkerContext &operator=(WorkerContext const &) = delete;
    /**
     * @brief Deleted — moving would strand any already-handed-out dispatch lambda's `this`
     * pointer, and std::mutex isn't movable to begin with.
     */
    WorkerContext(WorkerContext &&) = delete;
    /** @brief Deleted, same reasoning as the move ctor right above. */
    WorkerContext &operator=(WorkerContext &&) = delete;
    /** @brief Default dtor — every member is a normal RAII type, nothing needs manual teardown. */
    ~WorkerContext() = default;

    /**
     * @brief Sets the worker id, delegates straight to the underlying TaskRunner.
     * @param worker_id the new id to identify this worker as.
     */
    void set_worker_id(std::string_view worker_id) { m_task_runner.setWorkerId(worker_id); }

    // ── Task worker registry (delegates to congelado::worker::TaskRunner) ─────

    /**
     * @brief Loads task worker plugins found under `external_directory` (if given) and
     * `internal_directory` into the registry, via TaskRunner.
     * @param external_directory optional user-chosen directory for custom, non-built-in
     * workers. This is the only argument a normal caller should ever pass.
     * @param internal_directory the built-in workers directory (defaults to `"workers"`). Do
     * NOT change how this is populated or defaulted under normal circumstances.
     */
    void load_workers(const std::optional<std::filesystem::path> &external_directory = std::nullopt,
                      const std::filesystem::path &internal_directory = "workers") {
        m_task_runner.load_workers(external_directory, internal_directory);
    }

    /**
     * @brief Gets this worker's id.
     * @return the worker id, straight from the TaskRunner.
     */
    [[nodiscard]] std::string_view get_worker_id() const noexcept {
        return m_task_runner.getWorkerId();
    }

    /**
     * @brief Looks up the registered task worker for a given task type.
     * @param task_type the task type to look up.
     * @return pointer to the matching ITaskWorker, or nullptr if nothing's registered for that
     * type — check it before touching it, easy one to skip.
     */
    [[nodiscard]] ITaskWorker *get_task_worker(std::string_view task_type) const noexcept {
        return m_task_runner.getTaskWorker(task_type);
    }

    /**
     * @brief Runs a registered task synchronously by type, straight through the TaskRunner.
     * @param task_type the task type to execute.
     * @param input the task input payload.
     * @return the task output on success, or std::nullopt if the task type isn't registered or
     * execution comes back cooked.
     */
    [[nodiscard]] std::optional<TaskOutput> run_task(std::string_view task_type,
                                                     TaskInput const &input) {
        return m_task_runner.execute(task_type, input);
    }

    /**
     * @brief Gets every task type this worker has registered.
     * @return the registered task type names.
     */
    [[nodiscard]] std::vector<std::string_view> get_task_types() const noexcept {
        return m_task_runner.getTaskTypes();
    }

    // ── Engine communication ──────────────────────────────────────────

    /**
     * @brief Sets the engine client used for outbound HTTP calls to the engine.
     * @param engine the client to use; only a non-owning pointer is kept, so it's on the caller
     * to keep it alive for as long as this context is.
     */
    void set_engine(interfaces::IClient &engine) { m_engine = &engine; }

    /**
     * @brief Gets the currently configured engine client.
     * @return pointer to the engine client, or nullptr if set_engine() was never called.
     */
    [[nodiscard]] interfaces::IClient *get_engine() const noexcept { return m_engine; }

    // Blocking call to engine. Creates HttpRequest, sends via IClient, blocks until response.
    // The response arrives on a separate thread via the dispatch callback → resolve_response().
    /**
     * @brief Sends a request to the engine and blocks the calling thread until the matching
     * response lands via resolve_response(). Stream id auto-increments by 2 each call, no cap,
     * that's just the HTTP/2-style id scheme this uses.
     * @param method the HTTP method string (e.g. "GET", "POST", "DELETE").
     * @param path the request path/target on the engine.
     * @param body optional request body; when non-empty, Content-Type/Content-Length headers get
     * set and the bytes get buffered onto the request.
     * @return the engine's response once it shows up.
     * @throws std::runtime_error if no engine client has been configured via set_engine() —
     * checked first thing, before any of the stream bookkeeping happens.
     * @warning There's no timeout on the underlying future — if the engine never responds (dead
     * connection, dropped packet, whatever), this thread blocks forever and the pending-promise
     * entry just sits in `m_pending` for the rest of time. Also, never call this from the same
     * thread that pumps the dispatch callback (the one wired up by make_dispatch()) — that's an
     * instant deadlock since nothing would ever get around to resolving the promise. Straight L
     * if you get that wrong.
     */
    [[nodiscard]] EngineResponse call_engine(std::string_view method, std::string_view path,
                                             std::string_view body = "") {
        // Guard clause — no engine client, no point going any further.
        if (m_engine == nullptr) {
            throw std::runtime_error("worker: no engine client set");
        }

        // Next stream id for this request, HTTP/2-style (always +2, no reuse, no cap).
        auto stream_id = m_next_stream_id.fetch_add(2, std::memory_order_relaxed);

        // The caller blocks on this future until resolve_response() shows up with a match.
        std::promise<EngineResponse> promise;
        auto future = promise.get_future();

        {
            // Register the promise under lock so resolve_response() (fired from another
            // thread) can find it by stream id.
            std::scoped_lock lock(m_mutex);
            m_pending[stream_id] = &promise;
        }

        // Build the outbound request through the client abstraction — it owns method/path/
        // header/body buffering and the actual send(), leaving stream-id bookkeeping and
        // response correlation (the promise/future dance) as this class's own concern.
        auto client = core::client::Client::custom(method, path)
                          .with_runtime(*m_engine)
                          .with_stream_id(stream_id);

        if (!body.empty()) {
            // Only stamp content headers and buffer a body when there's actually one to send.
            client.add_header(interfaces::io::types::Token::CONTENT_TYPE, "application/json");
            client.add_header(interfaces::io::types::Token::CONTENT_LENGTH,
                              std::to_string(body.size()));
            client.add_body(body);
        }

        // Fire it off — the response lands async, on a different thread, via make_dispatch().
        client.send();

        // Block here till resolve_response() wakes this promise up. No timeout, straight L
        // if the engine ghosts us — see the warning above.
        return future.get();
    }

    // Called by the dispatch callback when an engine response arrives.
    /**
     * @brief Matches an incoming engine response back to its pending call_engine() promise by
     * stream id, and resolves it — this is what actually wakes up the blocked caller.
     * @param stream_id the stream id the response came in on.
     * @param status the HTTP status code the engine responded with.
     * @param body the response body.
     * @note Runs on whatever thread the dispatch callback fires on — not the thread that called
     * call_engine(). If `stream_id` isn't found in `m_pending` (already resolved, never
     * registered, whatever), the response just gets logged and dropped on the floor — no
     * exception, no retry, silent L.
     */
    void resolve_response(std::uint32_t stream_id, int status, std::string body) {
        std::promise<EngineResponse> *promise = nullptr;
        {
            // Look up (and immediately remove) the pending promise for this stream id, all
            // under lock — one shot, no double-resolve possible.
            std::scoped_lock lock(m_mutex);
            if (auto it = m_pending.find(stream_id); it != m_pending.end()) {
                promise = it->second;
                m_pending.erase(it);
            }
        }
        if (promise != nullptr) {
            // Found it — resolve, which wakes up whatever thread is blocked in call_engine().
            promise->set_value({.m_status = status, .m_body = std::move(body)});
        } else {
            // Nothing was waiting on this stream id — log and drop it on the floor, silent L.
            core::logger::warning("worker/context", "no pending promise for stream_id={}",
                                  stream_id);
        }
    }

    // Returns a ReceiveDispatchFn that routes engine responses to pending promises.
    /**
     * @brief Builds the callback that gets wired into the transport layer's receive dispatch —
     * every time a response comes back on this worker's connection, it lands here and gets
     * forwarded to resolve_response().
     * @return a ReceiveDispatchFn bound to this WorkerContext instance.
     * @warning The returned lambda captures `this` by reference — if the WorkerContext gets
     * destroyed while the dispatch fn is still registered somewhere, the next response that
     * arrives dereferences a dangling pointer. Straight UB, no safety net. Register once, keep
     * the WorkerContext alive at least as long as whatever holds this callback.
     */
    [[nodiscard]] interfaces::io::ReceiveDispatchFn make_dispatch() {
        return [this](interfaces::io::IRequest &req, interfaces::io::IResponse &res) {
            // Pull the stream id + body bytes off the response, repackage the body as a
            // plain std::string.
            auto stream_id = req.get_stream_id();
            auto body_bytes = res.get_body();
            std::string body(body_bytes.begin(), body_bytes.end());
            // Forward everything to resolve_response() — that's what actually wakes the
            // blocked caller up.
            auto status = static_cast<int>(interfaces::io::types::status_code(res.get_status()));
            resolve_response(stream_id, status, std::move(body));
        };
    }

  private:
    congelado::worker::TaskRunner m_task_runner;
    interfaces::IClient *m_engine{nullptr};
    std::mutex m_mutex;
    std::atomic<std::uint32_t> m_next_stream_id{1};
    std::unordered_map<std::uint32_t, std::promise<EngineResponse> *> m_pending;
};

} // namespace worker
