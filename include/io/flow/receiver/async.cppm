export module io_flow_receiver:async;

import std;
import core_events;
import core_logger;
import shared;
import interfaces;
import io_base_socket;
import utils_buffering;

export namespace io::base::flow::async {

template <typename Worker, typename Status, typename... Args>
    requires interfaces::io::AsyncReceivable<Worker, Status, std::byte *, Args...>
class Receiver : public shared::HandlerBase {
  public:
    /**
     * @brief Builds a Receiver with no callbacks wired up yet — read/error callbacks start null,
     * nothing armed until build() or the other constructor sets them and calls attach().
     * @param worker the worker whose fd this receiver reads from.
     */
    Receiver(Worker &worker) : m_worker{worker}, m_on_read{nullptr}, m_on_error{nullptr}, m_fatal{false} {
    }

    /**
     * @brief Builds a Receiver fully wired up with both callbacks and immediately attaches it —
     * bet, ready to go the moment this constructor returns, no separate build() call needed.
     * @param worker the worker whose fd this receiver reads from.
     * @param on_read invoked with the buffer view every time a read completes.
     * @param on_error invoked with `(fd, errno)` on a fatal read failure.
     */
    Receiver(Worker &worker, shared::ReadCallback on_read, shared::ErrorCallback on_error)
        : m_worker{worker}, m_on_read{std::move(on_read)}, m_on_error{std::move(on_error)}, m_fatal{false} {
        attach();
    }

    /**
     * @brief Default dtor — no special teardown here, `detach()`/`async_close()` run through
     * `on_released()` instead, not the destructor. Don't expect close-on-destroy for free.
     */
    ~Receiver() override = default;

    /**
     * @brief Copy ctor deleted — this thing holds a `reference_wrapper<Worker>` and owns a live
     * buffer pool mid-flight, copying it would be straight nonsense.
     */
    Receiver(const Receiver &) = delete;
    /**
     * @brief Copy assignment deleted, same reasoning as the copy ctor.
     */
    Receiver &operator=(const Receiver &) = delete;
    /**
     * @brief Move ctor deleted — this handler gets `create()`'d against a live controller and
     * captures `this` in its lambdas, so moving it out from under those captures would leave
     * dangling references. Not worth the risk, deleted instead.
     */
    Receiver(Receiver &&) = delete;
    /**
     * @brief Move assignment deleted, same `this`-capture reasoning as the move ctor.
     */
    Receiver &operator=(Receiver &&) = delete;

    /**
     * @brief Wires (or replaces) the read callback fired on every completed read.
     * @param on_read the new read callback.
     */
    void add_on_read(shared::ReadCallback on_read) { m_on_read = std::move(on_read); }

    /**
     * @brief Wires (or replaces) the error callback fired on a fatal read failure.
     * @param on_error the new error callback.
     */
    void add_on_error(shared::ErrorCallback on_error) { m_on_error = std::move(on_error); }

    /**
     * @brief Validates both callbacks are set, then attaches the receiver to its worker — call
     * this after using the no-callback constructor plus add_on_read()/add_on_error().
     * @throws std::runtime_error if either the read or error callback hasn't been set yet.
     */
    void build() {
        // Both callbacks gotta be wired before this thing's usable — bail loud if either's missing.
        if (!m_on_read) {
            throw std::runtime_error("Read callback must be set before building the Receiver");
        }
        if (!m_on_error) {
            throw std::runtime_error("Error callback must be set before building the Receiver");
        }
        // Callbacks confirmed present — safe to attach to the reactor now.
        attach();
    }


    /**
     * @brief Gets this handler's display name for the controller.
     * @return the fixed string `"Receiver - Async"`.
     */
    [[nodiscard]] std::string_view get_name() const noexcept override { return "Receiver - Async"; }

    /**
     * @brief Builds the work callback the controller runs each time this handler's turn comes up
     * — attempts a resume() and either reschedules itself or releases depending on the outcome.
     * @return the per-execution work callable.
     */
    shared::WorkerFunction on_execute() override {
        return [this]() {
            const auto DESCRIPTOR = m_worker.get().get_fd();
            // Still alive — arm another read and stay in the scheduler's rotation.
            if (resume()) {
                core::logger::debug("io/recv", "fd {} rescheduled", DESCRIPTOR);
                shared::this_handler::shedule();
            } else {
                // Fatal error already hit — nothing left to do but release this handler.
                core::logger::debug("io/recv", "fd {} releasing", DESCRIPTOR);
                shared::this_handler::release();
            }
        };
    }

    /**
     * @brief Builds the cleanup callback that runs once this handler is released — detaches from
     * the worker and closes it out async.
     * @return the release callback.
     */
    shared::ReleaseFunction on_released() noexcept override {
        return [this]() noexcept {
            // Pull the fd off the reactor before tearing the socket down underneath it.
            detach();
            m_worker.get().async_close();
        };
    }

    /**
     * @brief Builds the exception handler for this receiver — unpacks whatever got thrown mid-
     * execution and routes it to `m_on_error` with the right errno, or `-1` for anything that
     * isn't a `std::system_error`.
     * @return the error-handling callback.
     */
    shared::ErrorHandler on_error() override {
        return [this](const std::exception_ptr &eptr) {
            const auto DESCRIPTOR = m_worker.get().get_fd();
            // Nothing actually thrown — nothing to do.
            if (!eptr) {
                return;
            }
            // Rethrow to recover the concrete exception type, then route by specificity: system
            // errors carry a real error code, anything else just gets -1.
            try {
                std::rethrow_exception(eptr);
            } catch (const std::system_error &e) {
                core::logger::warning("io/recv", "fd {} sys error: {} (code: {})", DESCRIPTOR, e.what(), e.code().value());
                core::events::publish("io.recv.sys_error",
                                      {{"fd", std::to_string(DESCRIPTOR)},
                                       {"error", e.what()},
                                       {"code", std::to_string(e.code().value())}});
                m_on_error(DESCRIPTOR, e.code().value());
            } catch (const std::exception &e) {
                core::logger::warning("io/recv", "fd {} exception: {}", DESCRIPTOR, e.what());
                core::events::publish("io.recv.exception",
                                      {{"fd", std::to_string(DESCRIPTOR)}, {"error", e.what()}});
                m_on_error(DESCRIPTOR, -1);
            } catch (...) {
                core::logger::warning("io/recv", "fd {} unknown exception", DESCRIPTOR);
                core::events::publish("io.recv.unknown_exception", {{"fd", std::to_string(DESCRIPTOR)}});
                m_on_error(DESCRIPTOR, -1);
            }
        };
    }

    /**
     * @brief Arms another async read if this receiver isn't already dead.
     * @note Once `m_fatal` flips true there's no coming back — no reset method exists on this
     * class, so a fatal read error permanently stalls this receiver. Build a new one if you need
     * to keep going.
     * @return true if a read got armed and this handler should stay scheduled, false if it's
     * fatally stalled and should release instead.
     */
    bool resume() {
        // Once fatal there's no coming back — say so and let the caller release.
        if (m_fatal) {
            return false;
        }

        // Still good — queue up the next read.
        arm_read();
        return true;
    }

    /**
     * @brief Attaches the underlying worker's fd to whatever async reactor it needs to be on.
     */
    void attach() { m_worker.get().attach(); }
    /**
     * @brief Detaches the underlying worker's fd from the async reactor.
     */
    void detach() { m_worker.get().detach(); }

    /**
     * @brief Checks whether this receiver hit a fatal read error and gave up for good, lowkey
     * game over for this instance.
     * @return true if stalled/fatal, false if still good to keep reading.
     */
    [[nodiscard]] bool get_stalled() const noexcept { return m_fatal; }

  private:
    /**
     * @brief Acquires a fresh buffer slot from the pool and kicks off an async read into it,
     * wiring on_read_complete() as the completion callback.
     * @warning Captures `slot` by value into the completion lambda but `this` by raw pointer —
     * if this Receiver gets destroyed while the read is still in flight (io_uring completion
     * hasn't fired yet), that completion lambda blows up on a dangling `this`. Lifetime's on
     * whoever owns this Receiver to guarantee it outlives every in-flight async op. No cap, this
     * is the sharpest edge in the whole async receiver.
     */
    void arm_read() {
        // Grab a free buffer slot from the pool to read into.
        auto *slot = m_pool.acquire();
        // Fire the actual async read, wiring the completion lambda to hand the result back to
        // on_read_complete() once io_uring reports it's done.
        m_worker.get().async_read(slot->get_data(), static_cast<unsigned>(slot->get_limit()), 0,
                                  [this, slot](int result) mutable { on_read_complete(slot, result); });
    }

    /**
     * @brief Completion handler for an async read — on failure flips `m_fatal` and reports the
     * error, on success notifies the buffer pool of what got written and forwards the view to
     * `m_on_read`.
     * @param node the buffer slot the read landed in.
     * @param result the io_uring/syscall result: byte count on success, `<= 0` on error, no cap.
     */
    void on_read_complete(utils::buffering::NodeReader *node, int result) {
        const auto DESCRIPTOR = m_worker.get().get_fd();
        // `result <= 0` means the read failed — flip fatal for good and report it, no retrying.
        if (result <= 0) {
            core::logger::warning("io/recv", "fd {} read error: {}", DESCRIPTOR, result);
            core::events::publish("io.recv.read_error",
                                  {{"fd", std::to_string(DESCRIPTOR)}, {"result", std::to_string(result)}});
            m_fatal = true;
            m_on_error(DESCRIPTOR, -result);
            return;
        }

        // Success — tell the buffer pool how many bytes actually landed, then forward the view
        // to whoever's consuming reads.
        const auto BYTES = static_cast<std::size_t>(result);

        core::logger::debug("io/recv", "fd {} rx {} bytes", DESCRIPTOR, BYTES);

        m_pool.notify_read(node, BYTES);
        m_on_read(m_pool.get_view());
    }

    std::reference_wrapper<Worker> m_worker;
    utils::buffering::BufferWriter m_pool;
    shared::ReadCallback m_on_read;
    shared::ErrorCallback m_on_error;
    bool m_fatal;
};


static_assert(
    interfaces::io::AsyncReceivable<socket::Socket<socket::Protocol::TCP>, socket::SocketStatus, std::byte *>);

} // namespace io::base::flow::async
