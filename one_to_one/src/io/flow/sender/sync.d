module io.flow.sender.sync;
@nogc nothrow:

import shared_.handler;
import shared_.flow;
import io.base.socket.socket;
import utils.buffering.writter;
import utils.buffering.reader;
import utils.buffering.node;
import core_.logger;

// PORT-NOTE: C++ template <typename Worker, typename Status, typename... Args>
// requires interfaces::io::SyncSendable<Worker, Status, Args...>
// D port is concrete over Socket/SocketStatus; concept preserved as comment only.

/// Synchronous send half of a flow socket.
/// Drains a buffer pool onto a worker socket one frame at a time.
class SyncSender : HandlerBase {
  public:
    this(ref Socket worker_ref) {
        m_worker = &worker_ref;
        m_on_error = null;
        m_on_error_ctx = null;
        m_stalled = false;
        m_closed = false;
    }

    this(ref Socket worker_ref,
         void function(void*, int, int) @nogc nothrow on_error,
         void* on_error_ctx) {
        m_worker = &worker_ref;
        m_on_error = on_error;
        m_on_error_ctx = on_error_ctx;
        m_stalled = false;
        m_closed = false;
        build();
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
        }
    }

    void send(BufferNode slot) {
        core.logger.debug_("io/send", "fd enqueue bytes");
        m_pool.push(slot);
    }

    override const(char)[] get_name() const { return "Sender - Sync"; }

    override WorkerFunction on_execute() {
        return WorkerFunction(cast(void*)this, (void* ctx) @nogc nothrow {
            auto self = cast(SyncSender) ctx;
            if (self.resume()) {
                core.logger.debug_("io/send", "fd rescheduled");
                this_handler.shedule();
            }
        });
    }

    override ReleaseFunction on_released() {
        return ReleaseFunction(cast(void*)this, (void*) @nogc nothrow {});
    }

    override ErrorHandler on_error() {
        return ErrorHandler(cast(void*)this, (void* ctx, int fd, int code) @nogc nothrow {
            auto self = cast(SyncSender) ctx;
            if (self.m_on_error !is null) {
                self.m_on_error(self.m_on_error_ctx, fd, code);
            }
        });
    }

    /// Return a send callback that enqueues nodes into this sender's pool.
    SendCallback get_submitter() {
        return SendCallback(cast(void*)this, (void* ctx, ref BufferNode node) @nogc nothrow {
            (cast(SyncSender) ctx).send(node);
        });
    }

    bool resume() {
        if (m_closed) {
            return false;
        }
        if (!m_stalled) {
            m_stalled = true;
            arm_write();
        }
        return true;
    }

    void set_closed() {
        m_closed = true;
    }

    bool get_stalled() const { return m_stalled; }
    bool get_closed() const { return m_closed; }

    bool has_on_error() const { return m_on_error is null; }

  private:
    void arm_write() {
        const int fd = m_worker.get_fd();
        if (m_closed) {
            core.logger.warning("io/send", "fd write on closed");
            m_stalled = false;
            return;
        }

        auto view = m_pool.get_view();
        auto front = view.front();
        const(ubyte)* data = front[0];
        size_t sz = front[1];

        if (data is null || sz == 0) {
            m_stalled = false;
            return;
        }

        core.logger.debug_("io/send", "fd tx attempt bytes");

        auto result_pair = m_worker.sync_send(data, sz);
        int result = result_pair[0];
        auto status = result_pair[1];

        switch (status.get_status()) {
        case SocketValues.VALID: {
            core.logger.debug_("io/send", "fd tx bytes");
            view.consume(result);
            m_stalled = false;
            return;
        }
        case SocketValues.NON_BLOCKING_WOULD_HAVE_BLOCKED: {
            core.logger.debug_("io/send", "fd would block, reschedule");
            m_stalled = false;
            return;
        }
        case SocketValues.ERRORED:
        case SocketValues.CLEANLY_DISCONNECTED:
        case SocketValues.TIMED_OUT:
            core.logger.warning("io/send", "fd send error");
            m_closed = true;
            if (m_on_error !is null) {
                m_on_error(m_on_error_ctx, fd, status.get_value());
            }
            return;
        default:
            return;
        }
    }

    Socket* m_worker;
    BufferWriter m_pool;
    void function(void*, int, int) @nogc nothrow m_on_error;
    void* m_on_error_ctx;
    bool m_stalled;
    bool m_closed;
}
