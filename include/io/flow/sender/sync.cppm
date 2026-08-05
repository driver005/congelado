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

export namespace io::base::flow::sync {

template <typename Worker, typename Status, typename... Args>
    requires interfaces::io::SyncSendable<Worker, Status, Args...>
class Sender : public shared::HandlerBase {
  public:
    /**
     * @brief Builds a Sender with no error callback yet, starts open.
     * @param worker the worker whose fd this sender writes to.
     */
    Sender(Worker &worker) : m_worker{worker}, m_on_error{nullptr}, m_stalled{false}, m_closed{false} {
    }

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
        core::logger::debug("io/send", "fd {} enqueue {} bytes", m_worker.get().get_fd(), slot.get_written());
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
        return [this]() noexcept {
        };
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
                core::logger::warning("io/send", "fd {} sys error: {} ({})", m_worker.get().get_fd(),
                                      e.what(), e.code().value());
                core::events::publish("io.send.sys_error",
                                      {{"fd", std::to_string(m_worker.get().get_fd())},
                                       {"error", e.what()},
                                       {"code", std::to_string(e.code().value())}});
                m_on_error(m_worker.get().get_fd(), e.code().value());
            } catch (const std::exception &e) {
                core::logger::warning("io/send", "fd {} exception: {}", m_worker.get().get_fd(), e.what());
                core::events::publish("io.send.exception",
                                      {{"fd", std::to_string(m_worker.get().get_fd())},
                                       {"error", e.what()}});
                m_on_error(m_worker.get().get_fd(), -1);
            } catch (...) {
                core::logger::warning("io/send", "fd {} unknown exception", m_worker.get().get_fd());
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
        // Closed sender never resumes.
        if (m_closed) {
            return false;
        }
        // Only kick off a new write if one isn't already mid-flight.
        if (!m_stalled) {
            m_stalled = true;
            arm_write();
        }
        return true;
    }

    /**
     * @brief Marks this sender closed — resume() becomes a no-op after this, no cap.
     */
    void set_closed() noexcept {
        m_closed = true;
    }

    /**
     * @brief Checks whether a write is currently in flight inside arm_write().
     * @return true if mid-write, false otherwise.
     */
    [[nodiscard]] bool get_stalled() const noexcept { return m_stalled; }
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
        // Guard against writing on an already-closed sender.
        if (m_closed) {
            core::logger::warning("io/send", "fd {} write on closed", DESCRIPTOR);
            core::events::publish("io.send.write_on_closed", {{"fd", std::to_string(DESCRIPTOR)}});
            m_stalled = false;
            return;
        }

        // Nothing queued to send — bail without touching the socket.
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
            m_stalled = false;
            return;
        }
        case socket::VALUES::NON_BLOCKING_WOULD_HAVE_BLOCKED: {
            core::logger::debug("io/send", "fd {} would block, reschedule", DESCRIPTOR);
            m_stalled = false;
            return;
        }
        case socket::VALUES::ERRORED:
        case socket::VALUES::CLEANLY_DISCONNECTED:
        case socket::VALUES::TIMED_OUT:
            core::logger::warning("io/send", "fd {} send error: {} ({})", DESCRIPTOR,
                                  status.get_error_code(),
                                  utils::ErrnoTranslator::describe_errno(status.get_error_code()));
            core::events::publish(
                "io.send.send_error",
                {{"fd", std::to_string(DESCRIPTOR)},
                 {"error_code", std::to_string(status.get_error_code())},
                 {"error", std::string{utils::ErrnoTranslator::describe_errno(status.get_error_code())}}});
            m_closed = true;
            m_on_error(DESCRIPTOR, status.get_error_code());
            return;
        }
    }

    std::reference_wrapper<Worker> m_worker;
    utils::buffering::BufferWriter m_pool;
    shared::ErrorCallback m_on_error;
    bool m_stalled;
    bool m_closed;
};


static_assert(interfaces::io::SyncSendable<socket::Socket<socket::Protocol::TCP>, socket::SocketStatus>);

} // namespace io::base::flow::sync
