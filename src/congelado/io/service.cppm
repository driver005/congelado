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

    auto read(NativeHandle handle, void* buf,
              uint32_t nbytes, int64_t offset,
              Completion cb) noexcept -> void {
        derived().do_read(handle, buf, nbytes, offset, std::move(cb));
    }

    auto readv(NativeHandle handle, const IoVec* vecs,
               uint32_t count, int64_t offset,
               Completion cb) noexcept -> void {
        derived().do_readv(handle, vecs, count, offset, std::move(cb));
    }

    auto recv(NativeHandle sock, void* buf,
              uint32_t nbytes, int flags,
              Completion cb) noexcept -> void {
        derived().do_recv(sock, buf, nbytes, flags, std::move(cb));
    }

    // ── Writes ────────────────────────────────────────────────────────────

    auto write(NativeHandle handle, const void* buf,
               uint32_t nbytes, int64_t offset,
               Completion cb) noexcept -> void {
        derived().do_write(handle, buf, nbytes, offset, std::move(cb));
    }

    auto writev(NativeHandle handle, const IoVec* vecs,
                uint32_t count, int64_t offset,
                Completion cb) noexcept -> void {
        derived().do_writev(handle, vecs, count, offset, std::move(cb));
    }

    auto send(NativeHandle sock, const void* buf,
              uint32_t nbytes, int flags,
              Completion cb) noexcept -> void {
        derived().do_send(sock, buf, nbytes, flags, std::move(cb));
    }

    // ── Connection ────────────────────────────────────────────────────────

    // result in cb = new socket fd/HANDLE cast to int, or negative errno.
    auto accept(NativeHandle listen_sock,
                IpEndpoint*  endpoint,
                Completion   cb) noexcept -> void {
        derived().do_accept(listen_sock, endpoint, std::move(cb));
    }

    auto connect(NativeHandle      sock,
                 const IpEndpoint& endpoint,
                 Completion        cb) noexcept -> void {
        derived().do_connect(sock, endpoint, std::move(cb));
    }

    // how: 0=read, 1=write, 2=both  (SHUT_RD/WR/RDWR on Linux; SD_* on Windows)
    auto shutdown(NativeHandle sock, int how,
                  Completion cb) noexcept -> void {
        derived().do_shutdown(sock, how, std::move(cb));
    }

    // ── File ops ──────────────────────────────────────────────────────────

    auto openat(NativeHandle   dir_handle,
                std::string_view path,
                OpenFlags      flags,
                uint32_t       mode,
                Completion     cb) noexcept -> void {
        derived().do_openat(dir_handle, path, flags, mode, std::move(cb));
    }

    auto close(NativeHandle handle, Completion cb) noexcept -> void {
        derived().do_close(handle, std::move(cb));
    }

    auto fsync(NativeHandle handle, bool data_only,
               Completion cb) noexcept -> void {
        derived().do_fsync(handle, data_only, std::move(cb));
    }

    // ── Timer ─────────────────────────────────────────────────────────────

    auto sleep_for(std::chrono::nanoseconds duration,
                   Completion cb) noexcept -> void {
        derived().do_sleep_for(duration, std::move(cb));
    }

    // ── Yield ─────────────────────────────────────────────────────────────
    // Re-enqueues cb through the event loop (cooperative multitasking).

    auto yield(Completion cb) noexcept -> void {
        derived().do_yield(std::move(cb));
    }

    // ── Associate ─────────────────────────────────────────────────────────
    // No-op on Linux.  Must be called on Windows before any async op.

    auto associate(NativeHandle handle) -> void {
        derived().do_associate(handle);
    }

    // ── Event loop ────────────────────────────────────────────────────────

    auto run()         -> void  { derived().do_run();  }
    auto stop() noexcept -> void { derived().do_stop(); }

private:
    [[nodiscard]] auto derived() noexcept -> Derived& {
        return *static_cast<Derived*>(this);
    }
};

} // namespace congelado::io
