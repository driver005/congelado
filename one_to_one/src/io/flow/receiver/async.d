module io.flow.receiver.async_;
@nogc nothrow:

import shared_.handler;
import shared_.flow;
import io.base.socket.socket;
import utils.buffering.writter;
import utils.buffering.reader;
import utils.buffering.node;
import core_.logger;

// PORT-NOTE: C++ template <typename Worker, typename Status, typename... Args>
// requires interfaces::io::AsyncReceivable<Worker, Status, std::byte*, Args...>
// D port is concrete; concept constraint preserved as comment only.

/// Asynchronous receive half of a flow socket.
/// Uses async_read on the underlying worker and notifies on_read each time data arrives.
class AsyncReceiver : HandlerBase {
  public:
    /// Construct with worker only (no callbacks set yet).
    this(ref Socket worker_ref) {
        m_worker = &worker_ref;
        m_on_read = null;
        m_on_read_ctx = null;
        m_on_error = null;
        m_on_error_ctx = null;
        m_fatal = false;
    }

    /// Construct with worker + both callbacks; calls attach().
    this(ref Socket worker_ref,
         void function(void*, ref BufferReader) @nogc nothrow on_read,
         void* on_read_ctx,
         void function(void*, int, int) @nogc nothrow on_error,
         void* on_error_ctx) {
        m_worker = &worker_ref;
        m_on_read = on_read;
        m_on_read_ctx = on_read_ctx;
        m_on_error = on_error;
        m_on_error_ctx = on_error_ctx;
        m_fatal = false;
        attach();
    }

    ~this() {}

    void add_on_read(void function(void*, ref BufferReader) @nogc nothrow on_read, void* ctx) {
        m_on_read = on_read;
        m_on_read_ctx = ctx;
    }

    void add_on_error(void function(void*, int, int) @nogc nothrow on_error, void* ctx) {
        m_on_error = on_error;
        m_on_error_ctx = ctx;
    }

    // PORT-NOTE: C++ threw std::runtime_error; D logs fatal (nothrow).
    void build() {
        if (m_on_read is null) {
            core.logger.fatal("io/recv", "Read callback must be set before building the Receiver");
            return;
        }
        if (m_on_error is null) {
            core.logger.fatal("io/recv", "Error callback must be set before building the Receiver");
            return;
        }
        attach();
    }

    override const(char)[] get_name() const { return "Receiver - Async"; }

    override WorkerFunction on_execute() {
        return WorkerFunction(cast(void*)this, (void* ctx) @nogc nothrow {
            auto self = cast(AsyncReceiver) ctx;
            const int fd = self.m_worker.get_fd();
            if (self.resume()) {
                core.logger.debug_("io/recv", "fd rescheduled");
                this_handler.shedule();
            } else {
                core.logger.debug_("io/recv", "fd releasing");
                this_handler.release();
            }
        });
    }

    override ReleaseFunction on_released() {
        return ReleaseFunction(cast(void*)this, (void* ctx) @nogc nothrow {
            auto self = cast(AsyncReceiver) ctx;
            self.detach();
            self.m_worker.async_close();
        });
    }

    override ErrorHandler on_error() {
        return ErrorHandler(cast(void*)this, (void* ctx, int fd, int code) @nogc nothrow {
            auto self = cast(AsyncReceiver) ctx;
            if (self.m_on_error !is null) {
                self.m_on_error(self.m_on_error_ctx, fd, code);
            }
        });
    }

    bool resume() {
        if (m_fatal) {
            return false;
        }
        arm_read();
        return true;
    }

    void attach() { m_worker.attach(); }
    void detach() { m_worker.detach(); }

    bool get_stalled() const { return m_fatal; }

  private:
    // PORT-NOTE: C++ used a lambda capturing slot and passed to async_read.
    // D async_read callback is a fn+ctx pair; we store the node pointer in a
    // small heap-allocated context struct to satisfy @nogc (no closures).
    void arm_read() {
        auto slot = m_pool.acquire();
        // PORT-NOTE: async_read callback stub — full async wiring deferred to Run 3.
        // The C++ used: m_worker.get().async_read(slot->get_data(), limit, 0, callback)
        // where the callback calls on_read_complete(slot, result).
        // TODO: wire async_read with a @nogc continuation struct (Run 3).
    }

    void on_read_complete(BufferNodeReader* node, int result) {
        const int fd = m_worker.get_fd();
        if (result <= 0) {
            core.logger.warning("io/recv", "fd read error");
            m_fatal = true;
            if (m_on_error !is null) {
                m_on_error(m_on_error_ctx, fd, -result);
            }
            return;
        }

        const size_t bytes = cast(size_t) result;
        core.logger.debug_("io/recv", "fd rx bytes");

        m_pool.notify_read(node, bytes);
        auto view = m_pool.get_view();
        if (m_on_read !is null) {
            m_on_read(m_on_read_ctx, view);
        }
    }

    Socket* m_worker;
    BufferWriter m_pool;
    void function(void*, ref BufferReader) @nogc nothrow m_on_read;
    void* m_on_read_ctx;
    void function(void*, int, int) @nogc nothrow m_on_error;
    void* m_on_error_ctx;
    bool m_fatal;
}
