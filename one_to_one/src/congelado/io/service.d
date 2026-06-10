module congelado.io.service;
@nogc nothrow:

// PORT-NOTE: C++ module congelado.io.service used template CRTP IoServiceBase<Derived>.
// D port preserves the CRTP pattern as a class template.
// std::function<void(int) noexcept> Completion → fn+ctx pair to stay @nogc.
// std::string_view path → const(char)[] in openat.
// std::chrono::nanoseconds → long (nanoseconds count), avoiding core.time import overhead.

// ─── NativeHandle ────────────────────────────────────────────────────────────
version (Windows) {
    alias NativeHandle = void*;
    static immutable NativeHandle kInvalidHandle = null;
} else {
    alias NativeHandle = int;
    static immutable NativeHandle kInvalidHandle = -1;
}

// ─── IoVec ───────────────────────────────────────────────────────────────────
// Layout-compatible with struct iovec on Linux.
// Manually mapped to WSABUF on Windows (field order swap in backend).
extern(C) struct IoVec {
    // PORT-NOTE: ABI POD (matches struct iovec layout on Linux/macOS).
    void*    m_base;
    ulong    m_len;
}

// ─── IpEndpoint ──────────────────────────────────────────────────────────────
struct IpEndpoint {
    // PORT-NOTE: ABI POD value wrapper, exempt from class-only rule.
    align(8) ubyte[128] m_storage;
    int                 m_len;
}

// ─── OpenFlags ───────────────────────────────────────────────────────────────
enum OpenFlags : uint {
    ReadOnly  = 0x0001,
    WriteOnly = 0x0002,
    ReadWrite = 0x0003,
    Create    = 0x0010,
    Truncate  = 0x0020,
    Append    = 0x0040,
}

OpenFlags opOr(OpenFlags a, OpenFlags b) {
    return cast(OpenFlags)(cast(uint) a | cast(uint) b);
}
OpenFlags opAnd(OpenFlags a, OpenFlags b) {
    return cast(OpenFlags)(cast(uint) a & cast(uint) b);
}

// ─── Completion ──────────────────────────────────────────────────────────────
// Called exactly once when an async op finishes.
// result >= 0  →  success (bytes transferred, new fd, etc.)
// result <  0  →  negated error code (errno on Linux, GetLastError on Windows)
// PORT-NOTE: C++ std::function<void(int) noexcept> → fn+ctx pair for @nogc.

struct Completion {
    // PORT-NOTE: value wrapper, exempt from class-only rule.
    void function(void* ctx, int result) @nogc nothrow fn;
    void* ctx;
}

// ─── IoServiceBase ────────────────────────────────────────────────────────────
// CRTP base.  The concrete backend (Linux or Windows) inherits from this and
// implements each do_* method.  Call sites only see this interface.
//
// Thread-safety: single-threaded per instance.
// Scale-out: one IoService per thread + SO_REUSEPORT.

class IoServiceBase(Derived) {
  public:

    // ── Reads ─────────────────────────────────────────────────────────────

    void read(NativeHandle handle, void* buf, uint nbytes, long offset,
              Completion cb) {
        derived().do_read(handle, buf, nbytes, offset, cb);
    }

    void readv(NativeHandle handle, const(IoVec)* vecs, uint count,
               long offset, Completion cb) {
        derived().do_readv(handle, vecs, count, offset, cb);
    }

    void recv(NativeHandle sock, void* buf, uint nbytes, int flags,
              Completion cb) {
        derived().do_recv(sock, buf, nbytes, flags, cb);
    }

    // ── Writes ────────────────────────────────────────────────────────────

    void write(NativeHandle handle, const(void)* buf, uint nbytes,
               long offset, Completion cb) {
        derived().do_write(handle, buf, nbytes, offset, cb);
    }

    void writev(NativeHandle handle, const(IoVec)* vecs, uint count,
                long offset, Completion cb) {
        derived().do_writev(handle, vecs, count, offset, cb);
    }

    void send(NativeHandle sock, const(void)* buf, uint nbytes, int flags,
              Completion cb) {
        derived().do_send(sock, buf, nbytes, flags, cb);
    }

    // ── Connection ────────────────────────────────────────────────────────

    // result in cb = new socket fd/HANDLE cast to int, or negative errno.
    void accept(NativeHandle listen_sock, IpEndpoint* endpoint, Completion cb) {
        derived().do_accept(listen_sock, endpoint, cb);
    }

    void connect(NativeHandle sock, ref const(IpEndpoint) endpoint, Completion cb) {
        derived().do_connect(sock, endpoint, cb);
    }

    // how: 0=read, 1=write, 2=both  (SHUT_RD/WR/RDWR on Linux; SD_* on Windows)
    void shutdown(NativeHandle sock, int how, Completion cb) {
        derived().do_shutdown(sock, how, cb);
    }

    // ── File ops ──────────────────────────────────────────────────────────

    void openat(NativeHandle dir_handle, const(char)[] path,
                OpenFlags flags, uint mode, Completion cb) {
        derived().do_openat(dir_handle, path, flags, mode, cb);
    }

    void close(NativeHandle handle, Completion cb) {
        derived().do_close(handle, cb);
    }

    void fsync(NativeHandle handle, bool data_only, Completion cb) {
        derived().do_fsync(handle, data_only, cb);
    }

    // ── Timer ─────────────────────────────────────────────────────────────
    // PORT-NOTE: C++ std::chrono::nanoseconds → plain long (nanosecond count).

    void sleep_for(long duration_ns, Completion cb) {
        derived().do_sleep_for(duration_ns, cb);
    }

    // ── Yield ─────────────────────────────────────────────────────────────
    // Re-enqueues cb through the event loop (cooperative multitasking).

    void yield(Completion cb) {
        derived().do_yield(cb);
    }

    // ── Associate ─────────────────────────────────────────────────────────
    // No-op on Linux.  Must be called on Windows before any async op.

    void associate(NativeHandle handle) {
        derived().do_associate(handle);
    }

    // ── Event loop ────────────────────────────────────────────────────────

    void run()  { derived().do_run(); }
    void stop() { derived().do_stop(); }

  private:
    Derived derived() { return cast(Derived) this; }
}
