module;

#include <cerrno>

export module io_flow_sender:async;

import std;
import core_events;
import core_logger;
import interfaces;
import utils_buffering;
import io_base_socket;
import shared;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace io::base::flow::async {

template <typename Worker, typename Status, typename... Args>
    requires interfaces::io::IoAsyncSend<Worker, Status, Args...>
class Sender : public shared::HandlerBase {
  public:
    /**
     * @brief Builds a Sender with no error callback yet — nothing's armed until build() sets one
     * and attaches.
     * @param worker the worker whose fd this sender writes to.
     */
    Sender(Worker &worker) : m_worker{worker}, m_on_error{nullptr}, m_fatal{false} {
    }

    /**
     * @brief Builds a Sender fully wired up with an error callback and immediately attaches it.
     * @param worker the worker whose fd this sender writes to.
     * @param on_error invoked with `(fd, errno)` on a fatal write failure.
     */
    Sender(Worker &worker, shared::ErrorCallback on_error)
        : m_worker{worker}, m_on_error{std::move(on_error)}, m_fatal{false} {
        attach();
    }

    /**
     * @brief Default dtor — teardown happens through on_released(), not here.
     */
    ~Sender() override = default;

    /**
     * @brief Copy ctor deleted — holds a `reference_wrapper<Worker>` and a live outbound buffer
     * queue, copying would double-own it.
     */
    Sender(const Sender &) = delete;
    /**
     * @brief Copy assignment deleted, same reasoning as the copy ctor.
     */
    Sender &operator=(const Sender &) = delete;
    /**
     * @brief Move ctor deleted — this handler gets `create()`'d against a controller and captures
     * `this` in its lambdas, moving it would dangle those captures.
     */
    Sender(Sender &&) = delete;
    /**
     * @brief Move assignment deleted, same `this`-capture reasoning as the move ctor.
     */
    Sender &operator=(Sender &&) = delete;

    /**
     * @brief Wires (or replaces) the error callback fired on a fatal write failure.
     * @param on_error the new error callback.
     */
    void add_on_error(shared::ErrorCallback on_error) { m_on_error = std::move(on_error); }

    /**
     * @brief Validates the error callback is set, then attaches the sender to its worker.
     * @throws std::runtime_error if the error callback hasn't been set yet.
     */
    void build() {
        // Error callback's mandatory — bail loud if it's missing.
        if (!m_on_error) {
            throw std::runtime_error("Error callback must be set before building the Sender");
        }
        // Callback confirmed — safe to attach to the reactor.
        attach();
    }

    /**
     * @brief Queues an outbound buffer for sending — doesn't write it immediately, just pushes
     * it onto the pool, actual transmission happens through resume()/arm_write().
     * @param slot the buffer node to enqueue for sending.
     */
    void send(utils::buffering::BufferNode slot) {
        core::logger::debug("io/send", "fd {} enqueue {} bytes", m_worker.get().get_fd(), slot.get_written());
        m_pool.push(std::move(slot));
    }


    /**
     * @brief Gets this handler's display name for the controller.
     * @return the fixed string `"Sender - Async"`.
     */
    [[nodiscard]] std::string_view get_name() const noexcept override { return "Sender - Async"; }

    /**
     * @brief Builds the work callback the controller runs each time this handler's turn comes up
     * — resume()s and either reschedules or releases based on the outcome.
     * @return the per-execution work callable.
     */
    shared::WorkerFunction on_execute() override {
        return [this]() {
            // Keep pumping writes while alive, release once fatally stalled.
            if (resume()) {
                core::logger::debug("io/send", "fd {} rescheduled", m_worker.get().get_fd());
                shared::this_handler::shedule();
            } else {
                core::logger::debug("io/send", "fd {} releasing", m_worker.get().get_fd());
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
            // Off the reactor first, then tear the socket down.
            detach();
            m_worker.get().async_close();
        };
    }

    /**
     * @brief Builds the exception handler for this sender — unpacks whatever got thrown and
     * routes it to `m_on_error` with the right errno, `-1` for anything else.
     * @return the error-handling callback.
     */
    shared::ErrorHandler on_error() override {
        return [this](const std::exception_ptr &eptr) {
            // Nothing thrown, nothing to route.
            if (!eptr) {
                return;
            }
            // Rethrow to recover the real type and route to m_on_error with the right code.
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
     * @brief Hands back a submitter callback that forwards straight into send() — handy for
     * passing "how do I enqueue a write" down into code that shouldn't hold a reference to the
     * whole Sender.
     * @return a callback that enqueues a `BufferNode` for sending.
     */
    shared::SendCallback get_submitter() {
        return [this](utils::buffering::BufferNode &&node) { this->send(std::move(node)); };
    }

    /**
     * @brief Arms another async write attempt if this sender isn't already dead.
     * @note Same one-way-door deal as the async Receiver's `m_fatal` — once it flips true there's
     * no resetting it on this object.
     * @return true if a write got attempted and this handler should stay scheduled, false if
     * fatally stalled and should release instead.
     */
    bool resume() {
        // No resurrecting a fatally-stalled sender.
        if (m_fatal) {
            return false;
        }
        // Otherwise attempt the next write.
        arm_write();
        return true;
    }

    /**
     * @brief Attaches the underlying worker's fd to the async reactor.
     */
    void attach() { m_worker.get().attach(); }
    /**
     * @brief Detaches the underlying worker's fd from the async reactor.
     */
    void detach() { m_worker.get().detach(); }

    /**
     * @brief Checks whether this sender hit a fatal write error and gave up for good.
     * @return true if stalled/fatal, false if still good to keep sending.
     */
    [[nodiscard]] bool get_stalled() const noexcept { return m_fatal; }

    /**
     * @brief Checks whether an error callback is set.
     * @warning Straight up inverted, no cap — this returns `!m_on_error`, so it's actually true
     * when there's NO error callback wired and false when one IS set. Read the name and you'd
     * bet it's the other way around. Whoever calls this expecting "true means I have a handler"
     * is gonna get cooked. Flagging it since this pass is comment-only, not touching the logic.
     * @return true if `m_on_error` is unset (empty), false if a callback is actually wired.
     */
    [[nodiscard]] bool has_on_error() const noexcept { return !m_on_error; }

  private:
    /**
     * @brief Pulls the front of the outbound buffer queue and, if there's anything there, kicks
     * off an async write, wiring on_write_complete() as the completion callback.
     * @warning Same raw-`this`-capture footgun as the async Receiver's arm_read() — the
     * completion lambda captures `this` by pointer, so this Sender getting destroyed while the
     * write's still in flight is a straight-up dangling-pointer UB situation. Owning code has to
     * guarantee this outlives every in-flight write.
     */
    void arm_write() {
        const auto FD = m_worker.get().get_fd();
        // Peek the front of the outbound queue — nothing queued means nothing to send.
        auto [data, size] = m_pool.get_view().front();

        if (data == nullptr || size == 0) {
            return;
        }

        // Fire the async write, wiring the completion lambda to hand the result back to
        // on_write_complete() once it lands.
        core::logger::debug("io/send", "fd {} tx attempt {} bytes", FD, size);
        m_worker.get().async_send(data, static_cast<unsigned>(size),
                                  [this](int result) mutable { on_write_complete(result); });
    }

    /**
     * @brief Completion handler for an async write — backs off quietly on EAGAIN/EWOULDBLOCK,
     * flips fatal and reports on any other negative result, otherwise consumes the written bytes
     * off the front of the queue.
     * @param result the syscall result: bytes written on success, a negative errno on failure.
     */
    void on_write_complete(int result) {
        const auto FD = m_worker.get().get_fd();
        // Transient would-block — nothing consumed, just try again next time round.
        if (result == -EAGAIN || result == -EWOULDBLOCK) {
            core::logger::debug("io/send", "fd {} would block, reschedule", FD);
            return;
        }

        // Any other negative result is a real, fatal send error.
        if (result < 0) {
            const auto ERROR_CODE = -result;
            core::logger::warning("io/send", "fd {} send error: {}", FD, ERROR_CODE);
            core::events::publish("io.send.send_error",
                                  {{"fd", std::to_string(FD)}, {"error_code", std::to_string(ERROR_CODE)}});
            m_fatal = true;
            m_on_error(FD, ERROR_CODE);
            return;
        }

        // Success — drop the bytes that actually made it off the front of the queue.
        core::logger::debug("io/send", "fd {} tx {} bytes", FD, result);
        m_pool.get_view().consume(result);
    }

    std::reference_wrapper<Worker> m_worker;
    utils::buffering::BufferWriter m_pool;
    shared::ErrorCallback m_on_error;
    bool m_fatal;
};


static_assert(interfaces::io::AsyncSendable<socket::Socket<socket::Protocol::TCP>, socket::SocketStatus>);

} // namespace io::base::flow::async

#ifdef CONGELADO_TEST
namespace io::base::flow::async::sender_async_tests {

// No instance of Sender<Worker, Status, Args...> can be constructed here (or, as far as this
// codebase's usage shows, anywhere) — this class's own compile-time constraint is self-
// contradictory, independent of any mock:
//
//   1. `interfaces::io::AsyncGetter` (include/interfaces/io/io.cppm, outside this file's edit
//      scope) requires get_fd() to return EXACTLY `void`:
//         { sock.get_fd() } noexcept -> std::same_as<void>;
//   2. But every Sender method that touches get_fd() (send(), on_execute(), on_error(),
//      arm_write()) uses its result as a real fd value — a format argument, or the first `int`
//      of ErrorCallback = move_only_function<void(int,int)>. `const auto FD =
//      m_worker.get().get_fd();` can't even name a `void`.
//   3. Sender overrides shared::HandlerBase's pure virtual on_execute()/on_released()/on_error().
//      Per [temp.inst], implicitly instantiating a class template specialization instantiates
//      the DEFINITIONS of its virtual member functions unconditionally (the vtable needs real
//      addresses for them) — so merely constructing a Sender<Worker,...>, without calling
//      anything on it, forces on_execute()'s lambda body to compile against whatever get_fd()
//      returns.
//
//   A get_fd() returning `int` fails the class's own `requires interfaces::io::IoAsyncSend<...>`
//   clause outright (the type can't be named at all). A get_fd() returning `void` satisfies that
//   clause but then fails to compile inside on_execute()/on_error()/arm_write(). No Worker can
//   satisfy both at once. Confirms this isn't a mocking gap: grepping the codebase,
//   `interfaces::io::IoAsyncSend` is referenced exactly once anywhere — this class's own
//   requires-clause — and every static_assert in this file (see above) and its Receiver/sync-
//   Sender siblings checks the plain `AsyncSendable`/`AsyncReceivable`/`SyncSendable` concept
//   instead, never a composite that drags in `AsyncGetter`/`SyncGetter`. So what follows only
//   exercises the compile-time concept machinery the class's requires-clause depends on.

class GetFdReturnsVoid {
  public:
    void get_fd() const noexcept {}
    void attach() noexcept {}
    void detach() noexcept {}
    void async_close() noexcept {}
    void async_send(const std::byte *, std::size_t, interfaces::io::IoCallback<socket::SocketStatus>) noexcept {}
};

class GetFdReturnsInt {
  public:
    int get_fd() const noexcept { return 0; }
    void attach() noexcept {}
    void detach() noexcept {}
    void async_close() noexcept {}
    void async_send(const std::byte *, std::size_t, interfaces::io::IoCallback<socket::SocketStatus>) noexcept {}
};

using namespace boost::ut;

suite<"Sender (async) — IoAsyncSend concept gating (see block comment above)"> sender_async_concept_suite = [] {
    "a get_fd() returning void satisfies AsyncGetter/IoAsyncSend as literally written"_test = [] {
        expect(interfaces::io::AsyncGetter<GetFdReturnsVoid>);
        expect((interfaces::io::IoAsyncSend<GetFdReturnsVoid, socket::SocketStatus>));
    };

    "a get_fd() returning a real, usable fd type does NOT satisfy AsyncGetter"_test = [] {
        expect(!interfaces::io::AsyncGetter<GetFdReturnsInt>);
        expect(!(interfaces::io::IoAsyncSend<GetFdReturnsInt, socket::SocketStatus>));
    };
};

// UAF-design-gap test (same structural pattern as Receiver's async.cppm / Connector's) NOT added
// here: as the block comment at the top of this suite proves, no `Sender<Worker, Status, Args...>`
// can be instantiated at all for ANY Worker — the class's own `requires
// interfaces::io::IoAsyncSend<...>` clause and its virtual-function bodies demand mutually
// exclusive `get_fd()` return types ([temp.inst] forces on_execute()/on_error()/arm_write() to
// compile against whatever `get_fd()` returns the moment a Sender is constructed, and no return
// type satisfies both the concept and those bodies). Since arm_write()'s raw-`this`-capture bug
// this test would target is unreachable without a live `Sender` instance to destroy, and none can
// exist, this finding is documented-and-skipped rather than faked with a type that wouldn't
// actually compile against the real template.

} // namespace io::base::flow::async::sender_async_tests
#endif
