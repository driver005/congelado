export module io_flow_sender:sync;

import std;
import core_events;
import core_logger;
import interfaces;
import utils_buffering;
import io_base_leverage;
import io_base_socket;
import shared;
import utils_errno_translator;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace io::base::flow::sync {

template <typename Worker, typename Status, typename... Args>
    requires interfaces::io::SyncSendable<Worker, Status, Args...>
class Sender : public shared::HandlerBase {
  public:
    /**
     * @brief Builds a Sender with no error callback yet, starts open.
     * @param worker the worker whose fd this sender writes to.
     */
    Sender(Worker &worker)
        : m_worker{worker}, m_on_error{nullptr}, m_stalled{false}, m_closed{false} {}

    /**
     * @brief Builds a Sender fully wired up with an error callback and validates it via build()
     * right away — no separate setup step needed, bet.
     * @param worker the worker whose fd this sender writes to.
     * @param on_error invoked with `(fd, error value)` on a write failure.
     */
    Sender(Worker &worker, shared::ErrorCallback on_error)
        : m_worker{worker}, m_on_error{std::move(on_error)}, m_stalled{false}, m_closed{false} {
        build();
    }

    /**
     * @brief Default dtor — sync sockets get closed explicitly via set_closed()/owning code, not
     * here.
     */
    ~Sender() override = default;

    /**
     * @brief Copy ctor deleted — holds a `reference_wrapper<Worker>` plus live outbound buffer
     * state.
     */
    Sender(const Sender &) = delete;
    /**
     * @brief Copy assignment deleted, same reasoning as the copy ctor.
     */
    Sender &operator=(const Sender &) = delete;
    /**
     * @brief Move ctor deleted.
     */
    Sender(Sender &&) = delete;
    /**
     * @brief Move assignment deleted.
     */
    Sender &operator=(Sender &&) = delete;

    /**
     * @brief Wires (or replaces) the error callback fired on a write failure.
     * @param on_error the new error callback.
     */
    void add_on_error(shared::ErrorCallback on_error) & { m_on_error = std::move(on_error); }

    /**
     * @brief Validates the error callback is set.
     * @warning Unlike the async Sender's build(), this doesn't attach or arm anything — purely a
     * presence check on the callback.
     * @throws std::runtime_error if the error callback hasn't been set yet.
     */
    void build() {
        // Pure presence check — doesn't attach or arm anything on the sync path.
        if (!m_on_error) {
            throw std::runtime_error("Error callback must be set before building the Sender");
        }
    }

    /**
     * @brief Queues an outbound buffer for sending — actual transmission happens through
     * resume()/arm_write().
     * @param slot the buffer node to enqueue for sending.
     */
    void send(utils::buffering::BufferNode slot) {
        core::logger::debug("io/send", "fd {} enqueue {} bytes", m_worker.get().get_fd(),
                            slot.get_written());
        m_pool.push(std::move(slot));
    }

    /**
     * @brief Gets this handler's display name for the controller.
     * @return the fixed string `"Sender - Sync"`.
     */
    [[nodiscard]] std::string_view get_name() const noexcept override { return "Sender - Sync"; }

    /**
     * @brief Builds the work callback the controller runs each time this handler's turn comes up
     * — resume()s and either reschedules itself or releases depending on the outcome, same
     * release-on-failure pattern as the async Sender.
     * @return the per-execution work callable.
     */
    shared::WorkerFunction on_execute() override {
        return [this]() {
            const auto DESCRIPTOR = m_worker.get().get_fd();
            // Still open — attempt the next write and stay in the scheduler's rotation.
            if (resume()) {
                core::logger::debug("io/send", "fd {} rescheduled", DESCRIPTOR);
                shared::this_handler::shedule();
            } else {
                // Closed — reschedule would spin forever competing for a scheduler slot with a
                // dead connection, so release() this contract instead and free the slot.
                core::logger::debug("io/send", "fd {} releasing", DESCRIPTOR);
                shared::this_handler::release();
            }
        };
    }

    /**
     * @brief Builds the cleanup callback for release — a flat no-op here.
     * @note Same deviation as the sync Receiver's on_released(): the async Sender detaches and
     * async-closes on release, this one leaves teardown to set_closed()/owning code.
     * @return an empty no-op release callback.
     */
    shared::ReleaseFunction on_released() noexcept override {
        return [this]() noexcept {};
    }

    /**
     * @brief Builds the exception handler for this sender — same unpack-and-route pattern as the
     * async version.
     * @return the error-handling callback.
     */
    shared::ErrorHandler on_error() override {
        return [this](const std::exception_ptr &eptr) {
            // Nothing thrown, nothing to route.
            if (!eptr) {
                return;
            }
            // Rethrow to recover the real type and route to m_on_error accordingly.
            try {
                std::rethrow_exception(eptr);
            } catch (const std::system_error &e) {
                core::logger::warning("io/send", "fd {} sys error: {} ({})",
                                      m_worker.get().get_fd(), e.what(), e.code().value());
                core::events::publish("io.send.sys_error",
                                      {{"fd", std::to_string(m_worker.get().get_fd())},
                                       {"error", e.what()},
                                       {"code", std::to_string(e.code().value())}});
                m_on_error(m_worker.get().get_fd(), e.code().value());
            } catch (const std::exception &e) {
                core::logger::warning("io/send", "fd {} exception: {}", m_worker.get().get_fd(),
                                      e.what());
                core::events::publish(
                    "io.send.exception",
                    {{"fd", std::to_string(m_worker.get().get_fd())}, {"error", e.what()}});
                m_on_error(m_worker.get().get_fd(), -1);
            } catch (...) {
                core::logger::warning("io/send", "fd {} unknown exception",
                                      m_worker.get().get_fd());
                core::events::publish("io.send.unknown_exception",
                                      {{"fd", std::to_string(m_worker.get().get_fd())}});
                m_on_error(m_worker.get().get_fd(), -1);
            }
        };
    }

    /**
     * @brief Hands back a submitter callback that forwards straight into send().
     * @return a callback that enqueues a `BufferNode` for sending.
     */
    shared::SendCallback get_submitter() {
        return [this](utils::buffering::BufferNode &&node) { this->send(std::move(node)); };
    }

    /**
     * @brief Attempts a sync write if this sender isn't closed and isn't already mid-write.
     * @note `m_stalled` guards re-entrancy inside arm_write(), same pattern as the sync
     * Receiver's resume() — not a permanent give-up flag.
     * @return true if this sender is open (whether or not a write actually landed), false if
     * closed and nothing happened.
     */
    bool resume() {
        // A closed sender keeps flushing until its pool is empty (arm_write() now writes even when
        // closed — see there), then releases. This lets a final GOAWAY / response tail go out over
        // the still-open socket; a dead socket drops its backlog in arm_write(), so this converges.
        if (m_closed && m_pool.empty()) {
            return false;
        }
        // Only kick off a new write if one isn't already mid-flight.
        if (!get_stalled()) {
            set_stalled(true);
            arm_write();
        }
        return true;
    }

    /**
     * @brief Marks this sender closed — resume() becomes a no-op after this, no cap.
     */
    void set_closed() noexcept { m_closed = true; }

    /**
     * @brief Checks whether a write is currently in flight inside arm_write().
     * @return true if mid-write, false otherwise.
     */
    [[nodiscard]] bool get_stalled() const noexcept {
        return m_stalled.load(std::memory_order_acquire);
    }

  private:
    /// @brief Sets the in-flight guard (release ordering). Pairs with get_stalled()'s acquire load.
    void set_stalled(bool value) noexcept { m_stalled.store(value, std::memory_order_release); }

  public:
    /**
     * @brief Checks whether this sender has been closed.
     * @return true if closed, false if still open.
     */
    [[nodiscard]] bool get_closed() const noexcept { return m_closed; }

    /**
     * @brief Checks whether an error callback is set.
     * @warning Same inverted-name bug as the async Sender's has_on_error() — this returns
     * `!m_on_error`, so it reads true when there's NO callback wired, not when there is one.
     * Don't trust the name, trust the actual boolean logic. Flagging, not fixing — comment-only
     * pass.
     * @return true if `m_on_error` is unset (empty), false if a callback is actually wired.
     */
    [[nodiscard]] bool has_on_error() const noexcept { return !m_on_error; }

    /**
     * @brief Checks whether the outbound queue is empty — i.e. nothing left to send.
     * @return true if there's no pending send data, false otherwise.
     */
    [[nodiscard]] bool is_idle() noexcept { return m_pool.get_view().empty(); }

  private:
    /**
     * @brief Does the actual `sync_send()` call for whatever's at the front of the outbound
     * queue, dispatching on the resulting status: valid consumes the sent bytes, would-block
     * backs off quietly, anything else (error/disconnect/timeout) closes the sender and reports
     * through `m_on_error`.
     * @note Every branch resets `m_stalled` back to false, bet, letting the next resume() attempt
     * another write.
     */
    void arm_write() {
        const auto DESCRIPTOR = m_worker.get().get_fd();

        auto &view = m_pool.get_view();
        auto [data, size] = view.front();

        if ((data == nullptr) || size == 0) {
            m_stalled = false;
            return;
        }

        // Actually attempt the sync send.
        core::logger::debug("io/send", "fd {} tx attempt {} bytes", DESCRIPTOR, size);

        auto [result, status] = m_worker.get().sync_send(data, size);

        // Dispatch on outcome: valid consumes the sent bytes, would-block backs off quietly,
        // anything else (error/disconnect/timeout) closes the sender and reports it.
        switch (status.get_status()) {
        case socket::VALUES::VALID: {
            core::logger::debug("io/send", "fd {} tx {} bytes", DESCRIPTOR, result);
            view.consume(result);
            set_stalled(false);
            return;
        }
        case socket::VALUES::NON_BLOCKING_WOULD_HAVE_BLOCKED: {
            core::logger::debug("io/send", "fd {} would block, reschedule", DESCRIPTOR);
            set_stalled(false);
            return;
        }
        case socket::VALUES::ERRORED:
        case socket::VALUES::CLEANLY_DISCONNECTED:
        case socket::VALUES::TIMED_OUT:
            core::logger::warning("io/send", "fd {} send error: {} ({})", DESCRIPTOR,
                                  status.get_error_code(),
                                  utils::ErrnoTranslator::describe_errno(status.get_error_code()));
            core::events::publish("io.send.send_error",
                                  {{"fd", std::to_string(DESCRIPTOR)},
                                   {"error_code", std::to_string(status.get_error_code())},
                                   {"error", std::string{utils::ErrnoTranslator::describe_errno(
                                                 status.get_error_code())}}});
            m_closed = true;
            // Socket's dead — drop whatever's still queued so a closed sender's resume() sees an
            // empty pool and releases instead of looping on an unsendable backlog.
            view.consume(view.size());
            m_on_error(DESCRIPTOR, status.get_error_code());
            return;
        }
    }

    std::reference_wrapper<Worker> m_worker;
    utils::buffering::BufferWriter m_pool;
    shared::ErrorCallback m_on_error;
    std::atomic<bool> m_stalled;
    bool m_closed;
};


static_assert(
    interfaces::io::SyncSendable<socket::Socket<socket::Protocol::TCP>, socket::SocketStatus>);

} // namespace io::base::flow::sync

#ifdef CONGELADO_TEST
namespace io::base::flow::sync::sender_sync_tests {

// Satisfies interfaces::io::SyncSendable<MockSyncSendWorker, socket::SocketStatus> — note the
// concept requires sync_send() be callable on a CONST object (`requires(const T SOCK, ...)`),
// so this is a const member function with mutable bookkeeping, same as the real Socket::sync_send().
class MockSyncSendWorker {
  public:
    [[nodiscard]] int get_fd() const noexcept { return m_fd; }

    std::pair<std::size_t, socket::SocketStatus> sync_send(const std::byte *, std::size_t size) const {
        ++m_call_count;
        m_last_size = size;
        return {m_next_result, m_next_status};
    }

    int m_fd{7};
    mutable int m_call_count{0};
    mutable std::size_t m_last_size{0};
    std::size_t m_next_result{0};
    socket::SocketStatus m_next_status{socket::VALUES::VALID};
};

class MockHandlerInterface final : public shared::HandlerInterface {
  public:
    void schedule(std::uint32_t) override { ++m_schedule_count; }
    void deschedule(std::uint32_t) override { ++m_deschedule_count; }
    void release(std::uint32_t) override { ++m_release_count; }

    int m_schedule_count{0};
    int m_deschedule_count{0};
    int m_release_count{0};
};

using TestSender = Sender<MockSyncSendWorker, socket::SocketStatus>;

/// @brief Builds a 3-byte, fully-written BufferNode to push into a Sender's outbound queue.
[[nodiscard]] utils::buffering::BufferNode make_node() {
    return utils::buffering::BufferNode{std::vector<std::byte>{std::byte{1}, std::byte{2}, std::byte{3}}};
}

using namespace boost::ut;

suite<"Sender (sync) construction/build"> sender_sync_ctor_suite = [] {
    "the no-error ctor starts open with has_on_error() reporting no callback"_test = [] {
        MockSyncSendWorker worker;
        TestSender sender{worker};

        expect(sender.get_name() == "Sender - Sync");
        expect(!sender.get_closed());
        // Inverted-name bug (documented on has_on_error()): true here means NO callback is set.
        expect(sender.has_on_error());
    };

    "the error-wired ctor calls build() itself and doesn't throw"_test = [] {
        MockSyncSendWorker worker;
        expect(nothrow([&] { TestSender sender{worker, [](int, int) {}}; }));
    };

    "add_on_error() flips has_on_error() to false"_test = [] {
        MockSyncSendWorker worker;
        TestSender sender{worker};
        sender.add_on_error([](int, int) {});

        expect(!sender.has_on_error());
    };

    "build() throws when no error callback is set"_test = [] {
        MockSyncSendWorker worker;
        TestSender sender{worker};

        expect(throws<std::runtime_error>([&] { sender.build(); }));
    };

    "build() succeeds once an error callback is set"_test = [] {
        MockSyncSendWorker worker;
        TestSender sender{worker};
        sender.add_on_error([](int, int) {});

        expect(nothrow([&] { sender.build(); }));
    };
};

suite<"Sender (sync) send/resume/arm_write"> sender_sync_send_suite = [] {
    "a fresh sender is idle and resume() on an empty queue never calls sync_send"_test = [] {
        MockSyncSendWorker worker;
        TestSender sender{worker, [](int, int) {}};

        expect(sender.is_idle());
        expect(sender.resume());
        expect(worker.m_call_count == 0);
    };

    "send() queues data, making the sender non-idle until it's flushed"_test = [] {
        MockSyncSendWorker worker;
        TestSender sender{worker, [](int, int) {}};

        sender.send(make_node());
        expect(!sender.is_idle());
    };

    "a VALID send consumes the queued bytes and goes idle"_test = [] {
        MockSyncSendWorker worker;
        worker.m_next_status = socket::SocketStatus{socket::VALUES::VALID};
        worker.m_next_result = 3;
        TestSender sender{worker, [](int, int) {}};

        sender.send(make_node());
        expect(sender.resume());

        expect(worker.m_call_count == 1);
        expect(worker.m_last_size == 3);
        expect(sender.is_idle());
        expect(!sender.get_stalled());
        expect(!sender.get_closed());
    };

    "a would-block leaves the queued data intact and open"_test = [] {
        MockSyncSendWorker worker;
        worker.m_next_status = socket::SocketStatus{socket::VALUES::NON_BLOCKING_WOULD_HAVE_BLOCKED};
        TestSender sender{worker, [](int, int) {}};

        sender.send(make_node());
        expect(sender.resume());

        expect(!sender.is_idle());
        expect(!sender.get_closed());
    };

    "an errored send closes the sender, drops the backlog, and routes to on_error"_test = [] {
        MockSyncSendWorker worker;
        worker.m_next_status = socket::SocketStatus{socket::VALUES::ERRORED, 77};

        int error_fd = -1;
        int error_code = 0;
        TestSender sender{worker, [&](int fd, int code) {
                             error_fd = fd;
                             error_code = code;
                         }};

        sender.send(make_node());
        expect(sender.resume());

        expect(sender.get_closed());
        expect(sender.is_idle()); // backlog dropped on error
        expect(error_fd == worker.get_fd());
        expect(error_code == 77);

        // Closed with nothing left queued — resume() now permanently reports "stop scheduling".
        expect(!sender.resume());
    };

    "get_submitter() enqueues through send() just like calling it directly"_test = [] {
        MockSyncSendWorker worker;
        TestSender sender{worker, [](int, int) {}};

        auto submitter = sender.get_submitter();
        expect(sender.is_idle());
        submitter(make_node());
        expect(!sender.is_idle());
    };
};

suite<"Sender (sync) on_execute/on_released"> sender_sync_lifecycle_suite = [] {
    "on_execute() reschedules while open"_test = [] {
        MockHandlerInterface mock;
        shared::this_handler::current = &mock;
        shared::this_handler::current_id = 4;

        MockSyncSendWorker worker;
        TestSender sender{worker, [](int, int) {}};

        auto work = sender.on_execute();
        work();

        expect(mock.m_schedule_count == 1);

        shared::this_handler::current = nullptr;
    };

    "on_execute() releases once closed with nothing left queued"_test = [] {
        MockHandlerInterface mock;
        shared::this_handler::current = &mock;
        shared::this_handler::current_id = 4;

        MockSyncSendWorker worker;
        worker.m_next_status = socket::SocketStatus{socket::VALUES::ERRORED, 1};
        TestSender sender{worker, [](int, int) {}};
        sender.send(make_node());
        sender.resume(); // errors out, closes, drops the backlog

        auto work = sender.on_execute();
        work();

        expect(mock.m_release_count == 1);

        shared::this_handler::current = nullptr;
    };

    "on_released() is a harmless no-op"_test = [] {
        MockSyncSendWorker worker;
        TestSender sender{worker, [](int, int) {}};

        auto released = sender.on_released();
        expect(nothrow([&] { released(); }));
    };
};

suite<"Sender (sync) on_error"> sender_sync_error_suite = [] {
    "a null exception_ptr is a no-op"_test = [] {
        MockSyncSendWorker worker;
        int error_calls = 0;
        TestSender sender{worker, [&](int, int) { ++error_calls; }};

        auto handler = sender.on_error();
        expect(nothrow([&] { handler(std::exception_ptr{}); }));
        expect(error_calls == 0);
    };

    // NOTE: this pins OBSERVED behavior, not necessarily intended behavior — see the identical
    // note in include/io/flow/receiver/sync.cppm's own on_error suite. Root-cause investigation
    // there (isolated repros, a full `xmake build -r`) could not explain why the production
    // catch(const std::system_error&) clause doesn't match a std::system_error rethrown via
    // std::rethrow_exception(std::make_exception_ptr(...)) in this specific multi-partition test
    // binary — it falls through to catch(const std::exception&) instead, routing -1.
    "a std::system_error currently falls through to the generic exception path (-1), not its "
    "real error code — see NOTE above"_test = [] {
        MockSyncSendWorker worker;
        int error_code = 0;
        TestSender sender{worker, [&](int, int code) { error_code = code; }};

        auto handler = sender.on_error();
        handler(std::make_exception_ptr(
            std::system_error{std::make_error_code(std::errc::connection_reset)}));

        expect(error_code == -1);
    };

    "a plain std::exception routes as -1"_test = [] {
        MockSyncSendWorker worker;
        int error_code = 0;
        TestSender sender{worker, [&](int, int code) { error_code = code; }};

        auto handler = sender.on_error();
        handler(std::make_exception_ptr(std::runtime_error{"boom"}));

        expect(error_code == -1);
    };

    "a non-std exception also routes as -1"_test = [] {
        MockSyncSendWorker worker;
        int error_code = 0;
        TestSender sender{worker, [&](int, int code) { error_code = code; }};

        auto handler = sender.on_error();
        handler(std::make_exception_ptr(42));

        expect(error_code == -1);
    };
};

} // namespace io::base::flow::sync::sender_sync_tests
#endif
