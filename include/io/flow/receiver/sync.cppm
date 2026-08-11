export module io_flow_receiver:sync;

import std;
import core_events;
import core_logger;
import shared;
import interfaces;
import io_base_socket;
import utils_buffering;
import utils_errno_translator;

export namespace io::base::flow::sync {

template <typename Worker, typename Status, typename... Args>
    requires interfaces::io::SyncReceivable<Worker, Status, std::byte *, Args...>
class Receiver : public shared::HandlerBase {
  public:
    /**
     * @brief Builds a Receiver with no read callback yet and starts marked closed — nothing's
     * going to actually resume() until add_on_read()/add_on_error() + build() (or the other
     * constructors) get called.
     * @param worker the worker whose fd this receiver reads from.
     */
    Receiver(Worker &worker)
        : m_worker{worker}, m_on_read{nullptr}, m_on_error{nullptr}, m_stalled{false},
          m_closed{true} {}

    /**
     * @brief Builds a Receiver with just the error callback set, starts open (`m_closed = false`)
     * but lowkey still needs a read callback wired via add_on_read() before it can actually do
     * anything useful.
     * @param worker the worker whose fd this receiver reads from.
     * @param on_error invoked with `(fd, error value)` on a read failure.
     */
    Receiver(Worker &worker, shared::ErrorCallback on_error)
        : m_worker{worker}, m_on_read{nullptr}, m_on_error{std::move(on_error)}, m_stalled{false},
          m_closed{false} {}

    /**
     * @brief Builds a Receiver fully wired up with both callbacks and validates them via build()
     * right away — ready to resume() the moment this constructor returns.
     * @param worker the worker whose fd this receiver reads from.
     * @param on_read invoked with the buffer view every time a read completes.
     * @param on_error invoked with `(fd, error value)` on a read failure.
     */
    Receiver(Worker &worker, shared::ReadCallback on_read, shared::ErrorCallback on_error)
        : m_worker{worker}, m_on_read{std::move(on_read)}, m_on_error{std::move(on_error)},
          m_stalled{false}, m_closed{false} {
        build();
    }

    /**
     * @brief Default dtor — sync sockets get closed explicitly via set_closed()/owning code, not
     * here.
     */
    ~Receiver() override = default;

    /**
     * @brief Copy ctor deleted — holds a `reference_wrapper<Worker>` plus live buffer state,
     * copying would double-own the pool.
     */
    Receiver(const Receiver &) = delete;
    /**
     * @brief Copy assignment deleted, same reasoning as the copy ctor.
     */
    Receiver &operator=(const Receiver &) = delete;
    /**
     * @brief Move ctor — despite the `other` parameter name suggesting a real move body, this is
     * `= delete`, so it's unavailable, full stop, no partial-move semantics to worry about here.
     */
    Receiver(Receiver &&other) = delete;
    /**
     * @brief Move assignment deleted, same as the move ctor right above.
     */
    Receiver &operator=(Receiver &&other) = delete;

    /**
     * @brief Wires (or replaces) the read callback fired on every completed sync read.
     * @param on_read the new read callback.
     */
    void add_on_read(shared::ReadCallback on_read) { m_on_read = std::move(on_read); }

    /**
     * @brief Wires (or replaces) the error callback fired on a read failure.
     * @param on_error the new error callback.
     */
    void add_on_error(shared::ErrorCallback on_error) { m_on_error = std::move(on_error); }

    /**
     * @brief Validates both callbacks are set before this receiver can be used.
     * @warning Unlike the async Receiver's build(), this one does NOT flip `m_closed` or arm
     * anything — it's purely a callback-presence check. Call resume() separately to actually
     * start reading.
     * @throws std::runtime_error if either the read or error callback hasn't been set yet.
     */
    void build() {
        // Just a presence check — both callbacks gotta be wired, but nothing gets armed here,
        // that's resume()'s job.
        if (!m_on_read) {
            throw std::runtime_error("Read callback must be set before building the Receiver");
        }
        if (!m_on_error) {
            throw std::runtime_error("Error callback must be set before building the Receiver");
        }
    }

    /**
     * @brief Gets this handler's display name for the controller.
     * @return the fixed string `"Receiver - Sync"`.
     */
    [[nodiscard]] std::string_view get_name() const noexcept override { return "Receiver - Sync"; }

    /**
     * @brief Builds the work callback the controller runs each time this handler's turn comes up
     * — resume()s and either reschedules itself or releases depending on the outcome, same
     * release-on-failure pattern as the async Receiver.
     * @return the per-execution work callable.
     */
    shared::WorkerFunction on_execute() override {
        return [this]() {
            const auto FILE_DESCRIPTOR = m_worker.get().get_fd();
            // Still open — arm another read and stay in the scheduler's rotation.
            if (resume()) {
                core::logger::debug("io/recv", "fd {} rescheduled", FILE_DESCRIPTOR);
                shared::this_handler::shedule();
            } else {
                // Closed — reschedule would spin forever competing for a scheduler slot with a
                // dead connection, so release() this contract instead and free the slot.
                core::logger::debug("io/recv", "fd {} releasing", FILE_DESCRIPTOR);
                shared::this_handler::release();
            }
        };
    }

    /**
     * @brief Builds the cleanup callback for release — a flat no-op here.
     * @note Deviates from the async Receiver's on_released(), which detaches and async-closes
     * the worker. This sync variant leaves socket teardown entirely to set_closed()/owning code.
     * @return an empty no-op release callback.
     */
    shared::ReleaseFunction on_released() noexcept override {
        return [this]() noexcept {};
    }

    /**
     * @brief Builds the exception handler for this receiver — same unpack-and-route pattern as
     * the async version, routes to `m_on_error` with the right error value.
     * @return the error-handling callback.
     */
    shared::ErrorHandler on_error() override {
        return [this](const std::exception_ptr &eptr) {
            // Nothing thrown, nothing to route.
            if (!eptr) {
                return;
            }
            // Rethrow to recover the real type and dispatch to m_on_error with the right code —
            // system errors get their real value, everything else falls to -1.
            try {
                std::rethrow_exception(eptr);
            } catch (const std::system_error &e) {
                core::logger::warning("io/recv", "fd {} sys error: {} ({})",
                                      m_worker.get().get_fd(), e.what(), e.code().value());
                core::events::publish("io.recv.sys_error",
                                      {{"fd", std::to_string(m_worker.get().get_fd())},
                                       {"error", e.what()},
                                       {"code", std::to_string(e.code().value())}});
                m_on_error(m_worker.get().get_fd(), e.code().value());
            } catch (const std::exception &e) {
                core::logger::warning("io/recv", "fd {} exception: {}", m_worker.get().get_fd(),
                                      e.what());
                core::events::publish(
                    "io.recv.exception",
                    {{"fd", std::to_string(m_worker.get().get_fd())}, {"error", e.what()}});
                m_on_error(m_worker.get().get_fd(), -1);
            } catch (...) {
                core::logger::warning("io/recv", "fd {} unknown exception",
                                      m_worker.get().get_fd());
                core::events::publish("io.recv.unknown_exception",
                                      {{"fd", std::to_string(m_worker.get().get_fd())}});
                m_on_error(m_worker.get().get_fd(), -1);
            }
        };
    }

    /**
     * @brief Attempts a sync read if this receiver isn't closed and isn't already mid-read.
     * @note `m_stalled` here just guards re-entrancy within arm_read() (it flips true right
     * before the read call and back false once it resolves) — it's not a permanent "gave up"
     * flag like the async Receiver's `m_fatal`. A closed receiver can still be resumed
     * successfully if `m_closed` flips back false externally.
     * @return true if this receiver is open (whether or not a read actually landed), false if
     * closed and nothing happened.
     */
    bool resume() {
        // Idle = fully torn down; only then do we stop rescheduling. A merely-closed receiver
        // still gets one more arm_read() pass to release its armed slot ref before going idle.
        if (m_idle) {
            return false;
        }
        // Only kick off a new read if one isn't already in flight (re-entrancy guard).
        if (!m_stalled) {
            m_stalled = true;
            arm_read();
        }
        return true;
    }

    /**
     * @brief Marks this receiver closed — resume() becomes a permanent no-op after this until
     * something flips `m_closed` back manually (nothing on this class does that, no cap).
     */
    void set_closed() noexcept { m_closed = true; }

    /**
     * @brief Checks whether a read is currently in flight inside arm_read().
     * @return true if mid-read, false otherwise.
     */
    [[nodiscard]] bool get_stalled() const noexcept { return m_stalled; }
    /**
     * @brief Checks whether this receiver has been closed.
     * @return true if closed, false if still open.
     */
    [[nodiscard]] bool get_closed() const noexcept { return m_closed; }
    /**
     * @brief Whether this receiver has finished its final closed pass — armed slot released and
     * done rescheduling. Owning code waits on this before tearing the socket down.
     * @return true once fully idle.
     */
    [[nodiscard]] bool get_idle() const noexcept { return m_idle; }

  private:
    /**
     * @brief Does the actual `sync_receive()` call into a fresh buffer slot and dispatches based
     * on the resulting status: valid data gets forwarded to `m_on_read`, a would-block quietly
     * backs off, and anything else (error/disconnect/timeout) closes the receiver and reports
     * through `m_on_error`.
     * @note Every branch resets `m_stalled` back to false before returning — bet, that's what
     * lets the next resume() call actually attempt another read instead of getting stuck
     * thinking one's still in flight.
     */
    void arm_read() {
        const auto FILE_DESCRIPTOR = m_worker.get().get_fd();

        // Grab a buffer slot and do the actual blocking-ish sync receive call. acquire() only
        // took a fresh slot ref if it allocated a new node; a reused tail carries no extra ref.
        auto *slot = m_pool.acquire();

        auto [result, status] = m_worker.get().sync_receive(
            slot->get_data(), static_cast<unsigned>(slot->get_limit()), 0);

        // Dispatch on what actually happened: real data forwards to m_on_read, a would-block
        // just backs off quietly, and anything else (error/disconnect/timeout) closes the
        // receiver and reports through m_on_error.
        switch (status.get_status()) {
        case socket::VALUES::VALID: {
            const auto BYTES = static_cast<std::size_t>(result);

            core::logger::debug("io/recv", "fd {} rx {} bytes", FILE_DESCRIPTOR, BYTES);

            // notify_read() folds the bytes in and drops the slot ref taken on allocation.
            m_pool.notify_read(slot, BYTES);
            m_on_read(m_pool.get_view());
            m_stalled = false;
            return;
        }
        case socket::VALUES::NON_BLOCKING_WOULD_HAVE_BLOCKED: {
            core::logger::debug("io/recv", "fd {} would block", FILE_DESCRIPTOR);
            // No bytes landed. Normally keep the slot armed for the next read; but if we're closing
            // this is the final pass — release the armed slot ref and go idle so resume() stops.
            if (m_closed) {
                m_pool.release(slot);
                m_idle = true;
            }
            m_stalled = false;
            return;
        }
        case socket::VALUES::ERRORED:
        case socket::VALUES::CLEANLY_DISCONNECTED:
        case socket::VALUES::TIMED_OUT: {
            core::logger::warning("io/recv", "fd {} read error: {} ({})", FILE_DESCRIPTOR,
                                  status.get_error_code(),
                                  utils::ErrnoTranslator::describe_errno(status.get_error_code()));
            core::events::publish("io.recv.read_error",
                                  {{"fd", std::to_string(FILE_DESCRIPTOR)},
                                   {"error_code", std::to_string(status.get_error_code())},
                                   {"error", std::string{utils::ErrnoTranslator::describe_errno(
                                                 status.get_error_code())}}});
            // Read failed — drop this slot ref and finish teardown in one shot (closed + idle),
            // so resume() releases the contract without a second cleanup pass.
            m_closed = true;
            m_idle = true;
            m_pool.release(slot);
            m_on_error(FILE_DESCRIPTOR, status.get_error_code());
            return;
        }
        }
    }

    std::reference_wrapper<Worker> m_worker;
    utils::buffering::BufferWriter m_pool;
    shared::ReadCallback m_on_read;
    shared::ErrorCallback m_on_error;
    bool m_stalled;
    bool m_closed;
    // Two-phase teardown: m_closed = "stop serving, do a final cleanup pass"; m_idle = "fully
    // done, stop rescheduling". resume() keeps running arm_read() until m_idle, so the armed slot
    // ref gets released on that final closed pass instead of leaking (see DEBUG.md hunt).
    bool m_idle = false;
};

static_assert(interfaces::io::SyncReceivable<socket::Socket<socket::Protocol::TCP>,
                                             socket::SocketStatus, std::byte *>);

} // namespace io::base::flow::sync
