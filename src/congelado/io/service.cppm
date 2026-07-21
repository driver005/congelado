// congelado/io/service.cppm
export module congelado.io.service;

import std;

export namespace congelado::io {

// ─── NativeHandle ────────────────────────────────────────────────────────────

#if defined(_WIN32)
using NativeHandle = void*;
constexpr NativeHandle kInvalidHandle = nullptr;
#else
using NativeHandle = int;
constexpr NativeHandle kInvalidHandle = -1;
#endif

// ─── IoVec ───────────────────────────────────────────────────────────────────
// Layout-compatible with struct iovec on Linux.
// Manually mapped to WSABUF on Windows (field order swap in backend).

struct IoVec {
    void*    m_base{};
    uint64_t m_len{};
};

// ─── IpEndpoint ──────────────────────────────────────────────────────────────

struct IpEndpoint {
    alignas(8) std::byte m_storage[128]{};
    int                  m_len{};
};

// ─── OpenFlags ───────────────────────────────────────────────────────────────

enum class OpenFlags : uint32_t {
    ReadOnly  = 0x0001,
    WriteOnly = 0x0002,
    ReadWrite = 0x0003,
    Create    = 0x0010,
    Truncate  = 0x0020,
    Append    = 0x0040,
};

constexpr auto operator|(OpenFlags a, OpenFlags b) noexcept -> OpenFlags {
    return static_cast<OpenFlags>(
        static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}
constexpr auto operator&(OpenFlags a, OpenFlags b) noexcept -> OpenFlags {
    return static_cast<OpenFlags>(
        static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
}

// ─── Completion ──────────────────────────────────────────────────────────────
// Called exactly once when an async op finishes.
// result >= 0  →  success (bytes transferred, new fd, etc.)
// result <  0  →  negated error code (errno on Linux, GetLastError on Windows)

using Completion = std::function<void(int result) noexcept>;

// ─── IoServiceBase ────────────────────────────────────────────────────────────
// CRTP base.  The concrete backend (Linux or Windows) inherits from this and
// implements each do_* method.  Call sites only see this interface.
//
// Thread-safety: single-threaded per instance.
// Scale-out: one IoService per thread + SO_REUSEPORT.

template <typename Derived>
class IoServiceBase {
public:

    // ── Reads ─────────────────────────────────────────────────────────────

    /**
     * @brief Issues an async positioned read into `buf`.
     * @param handle the file/socket handle to read from.
     * @param buf destination buffer for the bytes read.
     * @param nbytes maximum number of bytes to read.
     * @param offset file offset to read from.
     * @param cb fires exactly once with bytes read (`>= 0`) or a negated error code (`< 0`).
     */
    auto read(NativeHandle handle, void* buf,
              uint32_t nbytes, int64_t offset,
              Completion cb) noexcept -> void {
        derived().do_read(handle, buf, nbytes, offset, std::move(cb));
    }

    /**
     * @brief Issues an async positioned scatter/gather read across multiple buffers.
     * @param handle the file/socket handle to read from.
     * @param vecs the buffer vector to read into.
     * @param count number of entries in `vecs`.
     * @param offset file offset to read from.
     * @param cb fires exactly once with total bytes read (`>= 0`) or a negated error code
     * (`< 0`).
     */
    auto readv(NativeHandle handle, const IoVec* vecs,
               uint32_t count, int64_t offset,
               Completion cb) noexcept -> void {
        derived().do_readv(handle, vecs, count, offset, std::move(cb));
    }

    /**
     * @brief Issues an async socket receive.
     * @param sock the socket handle to receive on.
     * @param buf destination buffer for the received bytes.
     * @param nbytes maximum number of bytes to receive.
     * @param flags backend-specific recv flags (e.g. `MSG_*` on POSIX).
     * @param cb fires exactly once with bytes received (`>= 0`) or a negated error code (`< 0`).
     */
    auto recv(NativeHandle sock, void* buf,
              uint32_t nbytes, int flags,
              Completion cb) noexcept -> void {
        derived().do_recv(sock, buf, nbytes, flags, std::move(cb));
    }

    // ── Writes ────────────────────────────────────────────────────────────

    /**
     * @brief Issues an async positioned write from `buf`.
     * @param handle the file/socket handle to write to.
     * @param buf source buffer for the write.
     * @param nbytes number of bytes to write.
     * @param offset file offset to write at.
     * @param cb fires exactly once with bytes written (`>= 0`) or a negated error code (`< 0`).
     */
    auto write(NativeHandle handle, const void* buf,
               uint32_t nbytes, int64_t offset,
               Completion cb) noexcept -> void {
        derived().do_write(handle, buf, nbytes, offset, std::move(cb));
    }

    /**
     * @brief Issues an async positioned scatter/gather write across multiple buffers.
     * @param handle the file/socket handle to write to.
     * @param vecs the buffer vector to write from.
     * @param count number of entries in `vecs`.
     * @param offset file offset to write at.
     * @param cb fires exactly once with total bytes written (`>= 0`) or a negated error code
     * (`< 0`).
     */
    auto writev(NativeHandle handle, const IoVec* vecs,
                uint32_t count, int64_t offset,
                Completion cb) noexcept -> void {
        derived().do_writev(handle, vecs, count, offset, std::move(cb));
    }

    /**
     * @brief Issues an async socket send.
     * @param sock the socket handle to send on.
     * @param buf source buffer for the send.
     * @param nbytes number of bytes to send.
     * @param flags backend-specific send flags (e.g. `MSG_*` on POSIX).
     * @param cb fires exactly once with bytes sent (`>= 0`) or a negated error code (`< 0`).
     */
    auto send(NativeHandle sock, const void* buf,
              uint32_t nbytes, int flags,
              Completion cb) noexcept -> void {
        derived().do_send(sock, buf, nbytes, flags, std::move(cb));
    }

    // ── Connection ────────────────────────────────────────────────────────

    // result in cb = new socket fd/HANDLE cast to int, or negative errno.
    /**
     * @brief Issues an async accept on a listening socket.
     * @param listen_sock the listening socket to accept on.
     * @param endpoint gets filled in with the connecting peer's address.
     * @param cb fires exactly once with the new connection's handle cast to `int` (`>= 0`), or a
     * negated error code (`< 0`).
     */
    auto accept(NativeHandle listen_sock,
                IpEndpoint*  endpoint,
                Completion   cb) noexcept -> void {
        derived().do_accept(listen_sock, endpoint, std::move(cb));
    }

    /**
     * @brief Issues an async connect to a remote endpoint.
     * @param sock the socket to connect.
     * @param endpoint the remote address to connect to.
     * @param cb fires exactly once with `>= 0` on success or a negated error code (`< 0`).
     */
    auto connect(NativeHandle      sock,
                 const IpEndpoint& endpoint,
                 Completion        cb) noexcept -> void {
        derived().do_connect(sock, endpoint, std::move(cb));
    }

    // how: 0=read, 1=write, 2=both  (SHUT_RD/WR/RDWR on Linux; SD_* on Windows)
    /**
     * @brief Issues an async shutdown on one or both directions of a socket.
     * @param sock the socket to shut down.
     * @param how which direction(s) to shut down — `0` read, `1` write, `2` both.
     * @param cb fires exactly once with `>= 0` on success or a negated error code (`< 0`).
     */
    auto shutdown(NativeHandle sock, int how,
                  Completion cb) noexcept -> void {
        derived().do_shutdown(sock, how, std::move(cb));
    }

    // ── File ops ──────────────────────────────────────────────────────────

    /**
     * @brief Issues an async `openat`-style file open, resolved relative to `dir_handle`.
     * @param dir_handle directory handle `path` is resolved relative to.
     * @param path the path to open, relative to `dir_handle`.
     * @param flags open flags (read/write/create/truncate/append), OR'd together.
     * @param mode file mode bits used if the open creates a new file.
     * @param cb fires exactly once with the new handle cast to `int` (`>= 0`) or a negated error
     * code (`< 0`).
     */
    auto openat(NativeHandle   dir_handle,
                std::string_view path,
                OpenFlags      flags,
                uint32_t       mode,
                Completion     cb) noexcept -> void {
        derived().do_openat(dir_handle, path, flags, mode, std::move(cb));
    }

    /**
     * @brief Issues an async close of `handle`.
     * @param handle the handle to close.
     * @param cb fires exactly once with `>= 0` on success or a negated error code (`< 0`).
     */
    auto close(NativeHandle handle, Completion cb) noexcept -> void {
        derived().do_close(handle, std::move(cb));
    }

    /**
     * @brief Issues an async fsync/fdatasync on `handle`.
     * @param handle the handle to flush to disk.
     * @param data_only `true` for fdatasync-style (data only), `false` to also flush metadata.
     * @param cb fires exactly once with `>= 0` on success or a negated error code (`< 0`).
     */
    auto fsync(NativeHandle handle, bool data_only,
               Completion cb) noexcept -> void {
        derived().do_fsync(handle, data_only, std::move(cb));
    }

    // ── Timer ─────────────────────────────────────────────────────────────

    /**
     * @brief Schedules `cb` to fire after `duration` elapses.
     * @param duration how long to wait before firing.
     * @param cb fires exactly once after the delay.
     */
    auto sleep_for(std::chrono::nanoseconds duration,
                   Completion cb) noexcept -> void {
        derived().do_sleep_for(duration, std::move(cb));
    }

    // ── Yield ─────────────────────────────────────────────────────────────
    // Re-enqueues cb through the event loop (cooperative multitasking).

    /**
     * @brief Re-enqueues `cb` through the event loop instead of calling it inline — a
     * cooperative-multitasking yield point, good for not hogging the loop.
     * @param cb fires on a later turn of the event loop.
     */
    auto yield(Completion cb) noexcept -> void {
        derived().do_yield(std::move(cb));
    }

    // ── Associate ─────────────────────────────────────────────────────────
    // No-op on Linux.  Must be called on Windows before any async op.

    /**
     * @brief Registers `handle` with the backend's completion mechanism before it's used in any
     * async op.
     * @warning No-op on Linux, but mandatory on Windows (IOCP) before any async call on
     * `handle` — skip it there and every subsequent op on that handle is straight cooked.
     * @param handle the handle to associate with this service.
     */
    auto associate(NativeHandle handle) -> void {
        derived().do_associate(handle);
    }

    // ── Event loop ────────────────────────────────────────────────────────

    /// @brief Runs the event loop, dispatching completions until `stop()` is observed.
    auto run()         -> void  { derived().do_run();  }
    /// @brief Signals the event loop to wind down; `run()` returns once it picks this up.
    auto stop() noexcept -> void { derived().do_stop(); }

private:
    /**
     * @brief CRTP helper — casts `this` down to the concrete backend type.
     * @return a reference to this instance as `Derived&`.
     */
    [[nodiscard]] auto derived() noexcept -> Derived& {
        return *static_cast<Derived*>(this);
    }
};

} // namespace congelado::io
