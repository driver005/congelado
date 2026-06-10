module io.flow.receiver.sync;
@nogc nothrow:

import shared_.handler;
import shared_.flow;
import io.base.socket.socket;
import utils.buffering.writter;
import utils.buffering.reader;
import utils.buffering.node;
import core_.logger;

// PORT-NOTE: C++ template <typename Worker, typename Status, typename... Args>
// requires interfaces::io::SyncReceivable<Worker, Status, std::byte*, Args...>
// D port is not parameterized over Worker/Status — callers instantiate with concrete types.
// The concept SyncReceivable is preserved as a comment only; D has no direct equivalent.

/// Synchronous receive half of a flow socket.
/// Reads data from a worker (socket) into a buffer pool and dispatches to on_read.
class SyncReceiver : HandlerBase {
  public:
    /// Construct with worker reference only (no callbacks set).
    this(ref Socket m_worker_ref) {
        m_worker = &m_worker_ref;
        m_on_read = null;
        m_on_read_ctx = null;
        m_on_error = null;
        m_on_error_ctx = null;
        m_stalled = false;
        m_closed = true;
    }

    /// Construct with worker and error callback.
    this(ref Socket m_worker_ref,
         void function(void*, int, int) @nogc nothrow on_error,
         void* on_error_ctx) {
        m_worker = &m_worker_ref;
        m_on_read = null;
        m_on_read_ctx = null;
        m_on_error = on_error;
        m_on_error_ctx = on_error_ctx;
        m_stalled = false;
        m_closed = false;
    }

    /// Construct with worker, read callback, and error callback; calls build().
    this(ref Socket m_worker_ref,
         void function(void*, ref BufferReader) @nogc nothrow on_read,
         void* on_read_ctx,
         void function(void*, int, int) @nogc nothrow on_error,
         void* on_error_ctx) {
        m_worker = &m_worker_ref;
        m_on_read = on_read;
        m_on_read_ctx = on_read_ctx;
        m_on_error = on_error;
        m_on_error_ctx = on_error_ctx;
        m_stalled = false;
        m_closed = false;
        build();
    }

    ~this() {}

    // No copy, no move (matches C++ = delete)

    void add_on_read(void function(void*, ref BufferReader) @nogc nothrow on_read, void* ctx) {
        m_on_read = on_read;
        m_on_read_ctx = ctx;
    }

    void add_on_error(void function(void*, int, int) @nogc nothrow on_error, void* ctx) {
        m_on_error = on_error;
        m_on_error_ctx = ctx;
    }

    /// Validate that both callbacks are set.
    // PORT-NOTE: C++ threw std::runtime_error; D logs fatal and returns (nothrow).
    void build() {
        if (m_on_read is null) {
            core.logger.fatal("io/recv", "Read callback must be set before building the Receiver");
            return;
        }
        if (m_on_error is null) {
            core.logger.fatal("io/recv", "Error callback must be set before building the Receiver");
            return;
        }
    }

    override const(char)[] get_name() const { return "Receiver - Sync"; }

    override WorkerFunction on_execute() {
        return WorkerFunction(cast(void*)this, (void* ctx) @nogc nothrow {
            auto self = cast(SyncReceiver) ctx;
            if (self.resume()) {
                core.logger.debug_("io/recv", "fd rescheduled");
                this_handler.shedule();
            }
        });
    }

    override ReleaseFunction on_released() {
        return ReleaseFunction(cast(void*)this, (void*) @nogc nothrow {});
    }

    override ErrorHandler on_error() {
        return ErrorHandler(cast(void*)this, (void* ctx, int fd, int code) @nogc nothrow {
            auto self = cast(SyncReceiver) ctx;
            if (self.m_on_error !is null) {
                self.m_on_error(self.m_on_error_ctx, fd, code);
            }
        });
    }

    bool resume() {
        if (m_closed) {
            return false;
        }
        if (!m_stalled) {
            m_stalled = true;
            arm_read();
        }
        return true;
    }

    void set_closed() {
        m_closed = true;
    }

    bool get_stalled() const { return m_stalled; }
    bool get_closed() const { return m_closed; }

  private:
    void arm_read() {
        const auto fd = m_worker.get_fd();
        if (m_closed) {
            core.logger.warning("io/recv", "fd read on closed");
            m_stalled = false;
            return;
        }

        auto slot = m_pool.acquire();

        auto result_pair = m_worker.sync_receive(slot.get_data(), cast(uint) slot.get_limit(), 0);
        int result = result_pair[0];
        auto status = result_pair[1];

        switch (status.get_status()) {
        case SocketValues.VALID: {
            const size_t bytes = cast(size_t) result;
            core.logger.debug_("io/recv", "fd rx bytes");
            m_pool.notify_read(slot, bytes);
            auto view = m_pool.get_view();
            if (m_on_read !is null) {
                m_on_read(m_on_read_ctx, view);
            }
            m_stalled = false;
            return;
        }
        case SocketValues.NON_BLOCKING_WOULD_HAVE_BLOCKED:
            core.logger.debug_("io/recv", "fd would block");
            m_stalled = false;
            return;
        case SocketValues.ERRORED:
        case SocketValues.CLEANLY_DISCONNECTED:
        case SocketValues.TIMED_OUT: {
            core.logger.warning("io/recv", "fd read error");
            m_closed = true;
            if (m_on_error !is null) {
                m_on_error(m_on_error_ctx, fd, status.get_value());
            }
            return;
        }
        default:
            return;
        }
    }

    Socket* m_worker;
    BufferWriter m_pool;
    void function(void*, ref BufferReader) @nogc nothrow m_on_read;
    void* m_on_read_ctx;
    void function(void*, int, int) @nogc nothrow m_on_error;
    void* m_on_error_ctx;
    bool m_stalled;
    bool m_closed;
}
