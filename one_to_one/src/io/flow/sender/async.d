module io.flow.sender.async_;
@nogc nothrow:

import shared_.handler;
import shared_.flow;
import io.base.socket.socket;
import utils.buffering.writter;
import utils.buffering.reader;
import utils.buffering.node;
import core_.logger;

// PORT-NOTE: C++ template <typename Worker, typename Status, typename... Args>
// requires interfaces::io::IoAsyncSend<Worker, Status, Args...>
// D port is concrete; concept preserved as comment only.

/// Asynchronous send half of a flow socket.
/// Uses async_send on the underlying worker.
class AsyncSender : HandlerBase {
  public:
    this(ref Socket worker_ref) {
        m_worker = &worker_ref;
        m_on_error = null;
        m_on_error_ctx = null;
        m_fatal = false;
    }

    this(ref Socket worker_ref,
         void function(void*, int, int) @nogc nothrow on_error,
         void* on_error_ctx) {
        m_worker = &worker_ref;
        m_on_error = on_error;
        m_on_error_ctx = on_error_ctx;
        m_fatal = false;
        attach();
    }

    ~this() {}

    void add_on_error(void function(void*, int, int) @nogc nothrow on_error, void* ctx) {
        m_on_error = on_error;
        m_on_error_ctx = ctx;
    }

    // PORT-NOTE: C++ threw std::runtime_error; D logs fatal (nothrow).
    void build() {
        if (m_on_error is null) {
            core.logger.fatal("io/send", "Error callback must be set before building the Sender");
            return;
        }
        attach();
    }

    void send(BufferNode slot) {
        core.logger.debug_("io/send", "fd enqueue bytes");
        m_pool.push(slot);
    }

    override const(char)[] get_name() const { return "Sender - Async"; }

    override WorkerFunction on_execute() {
        return WorkerFunction(cast(void*)this, (void* ctx) @nogc nothrow {
            auto self = cast(AsyncSender) ctx;
            if (self.resume()) {
                core.logger.debug_("io/send", "fd rescheduled");
                this_handler.shedule();
            } else {
                core.logger.debug_("io/send", "fd releasing");
                this_handler.release();
            }
        });
    }

    override ReleaseFunction on_released() {
        return ReleaseFunction(cast(void*)this, (void* ctx) @nogc nothrow {
            auto self = cast(AsyncSender) ctx;
            self.detach();
            self.m_worker.async_close();
        });
    }

    override ErrorHandler on_error() {
        return ErrorHandler(cast(void*)this, (void* ctx, int fd, int code) @nogc nothrow {
            auto self = cast(AsyncSender) ctx;
            if (self.m_on_error !is null) {
                self.m_on_error(self.m_on_error_ctx, fd, code);
            }
        });
    }

    /// Return a send callback that enqueues nodes into this sender's pool.
    SendCallback get_submitter() {
        return SendCallback(cast(void*)this, (void* ctx, ref BufferNode node) @nogc nothrow {
            (cast(AsyncSender) ctx).send(node);
        });
    }

    bool resume() {
        if (m_fatal) {
            return false;
        }
        arm_write();
        return true;
    }

    void attach() { m_worker.attach(); }
    void detach() { m_worker.detach(); }

    bool get_stalled() const { return m_fatal; }
    bool has_on_error() const { return m_on_error is null; }

  private:
    void arm_write() {
        const int fd = m_worker.get_fd();
        auto front = m_pool.get_view().front();
        const(ubyte)* data = front[0];
        size_t sz = front[1];

        if (data is null || sz == 0) {
            return;
        }

        core.logger.debug_("io/send", "fd tx attempt bytes");
        // PORT-NOTE: async_send callback is a @nogc fn+ctx pair.
        // C++ used a lambda; D stub defers full wiring to Run 3.
        // m_worker.async_send(data, cast(uint) sz, &on_write_complete_thunk, this);
        // TODO: wire async_send with @nogc continuation struct (Run 3).
    }

    void on_write_complete(int result) {
        const int fd = m_worker.get_fd();

        // C++ checked -EAGAIN / -EWOULDBLOCK
        version (Posix) {
            import core.sys.posix.errno : EAGAIN, EWOULDBLOCK;
            if (result == -EAGAIN || result == -EWOULDBLOCK) {
                core.logger.debug_("io/send", "fd would block, reschedule");
                return;
            }
        }

        if (result < 0) {
            const int error_code = -result;
            core.logger.warning("io/send", "fd send error");
            m_fatal = true;
            if (m_on_error !is null) {
                m_on_error(m_on_error_ctx, fd, error_code);
            }
            return;
        }

        core.logger.debug_("io/send", "fd tx bytes");
        m_pool.get_view().consume(result);
    }

    Socket* m_worker;
    BufferWriter m_pool;
    void function(void*, int, int) @nogc nothrow m_on_error;
    void* m_on_error_ctx;
    bool m_fatal;
}
