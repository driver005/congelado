// SYCL Buffer Backend — async buffer allocation over ice::builder
// Owns USM device memory with SYCL event-based synchronization.
// NO ATen / c10 / PyTorch dependencies.

module;

#include <sycl/sycl.hpp>

export module cc_ice_sycl_backend:buffer;

import std;
import cc_ice_builder_intern:status;
import cc_ice_sycl_backend:tensor; // AsyncResult, XPU_AsyncEvent

export namespace ice::builder {

// ---------------------------------------------------------------------------
// XPU_AsyncBuffer — owns a single USM device allocation
// ---------------------------------------------------------------------------
class XPU_AsyncBuffer
{
public:
    XPU_AsyncBuffer() = default;

    // Allocate bytes on device immediately (sync).
    XPU_AsyncBuffer(size_t bytes, sycl::queue& q) :
        queue_(&q),
        bytes_(bytes)
    {
        data_ = sycl::malloc_device(bytes, q);
    }

    ~XPU_AsyncBuffer()
    {
        free_usm();
    }

    // Non-copyable; movable
    XPU_AsyncBuffer(const XPU_AsyncBuffer&) = delete;
    XPU_AsyncBuffer& operator=(const XPU_AsyncBuffer&) = delete;

    XPU_AsyncBuffer(XPU_AsyncBuffer&& o) noexcept :
        queue_(o.queue_),
        data_(o.data_),
        bytes_(o.bytes_)
    {
        o.data_ = nullptr;
        o.bytes_ = 0;
    }

    XPU_AsyncBuffer& operator=(XPU_AsyncBuffer&& o) noexcept
    {
        if (this != &o) {
            free_usm();
            queue_ = o.queue_;
            data_ = o.data_;
            bytes_ = o.bytes_;
            o.data_ = nullptr;
            o.bytes_ = 0;
        }
        return *this;
    }

    // ---- sync alloc/free -------------------------------------------------

    void allocate(size_t bytes)
    {
        free_usm();
        if (!queue_) {
            return;
        }
        data_ = sycl::malloc_device(bytes, *queue_);
        bytes_ = data_ ? bytes : 0;
    }

    void deallocate() noexcept
    {
        free_usm();
    }

    // ---- async alloc result type -----------------------------------------

    struct AllocResult
    {
        sycl::event event;
        std::expected<void, ice::Status> status;
        XPU_AsyncBuffer buffer;

        [[nodiscard]] std::expected<void, ice::Status> wait() noexcept
        {
            try {
                event.wait_and_throw();
                return status;
            } catch (const sycl::exception& e) {
                return std::unexpected(ice::Status::from_message(e.what()));
            }
        }
    };

    struct FreeResult
    {
        sycl::event event;
        std::expected<void, ice::Status> status;

        [[nodiscard]] std::expected<void, ice::Status> wait() noexcept
        {
            try {
                event.wait_and_throw();
                return status;
            } catch (const sycl::exception& e) {
                return std::unexpected(ice::Status::from_message(e.what()));
            }
        }
    };

    struct CopyResult
    {
        sycl::event event;
        std::expected<void, ice::Status> status;

        [[nodiscard]] std::expected<void, ice::Status> wait() noexcept
        {
            try {
                event.wait_and_throw();
                return status;
            } catch (const sycl::exception& e) {
                return std::unexpected(ice::Status::from_message(e.what()));
            }
        }
    };

    // ---- async factory ---------------------------------------------------

    [[nodiscard]] static AllocResult allocate_async(size_t bytes, sycl::queue& q) noexcept
    {
        AllocResult r;
        try {
            // Allocation itself is synchronous in SYCL USM; we submit a no-op
            // barrier so callers can chain on the returned event.
            void* ptr = sycl::malloc_device(bytes, q);
            r.event = q.ext_oneapi_submit_barrier();
            if (ptr) {
                r.buffer.queue_ = &q;
                r.buffer.data_ = ptr;
                r.buffer.bytes_ = bytes;
                r.status = {};
            } else {
                r.status = std::unexpected(ice::Status::from_message("sycl::malloc_device OOM"));
            }
        } catch (const sycl::exception& e) {
            r.event = sycl::event{};
            r.status = std::unexpected(ice::Status::from_message(e.what()));
        }
        return r;
    }

    [[nodiscard]] FreeResult deallocate_async() noexcept
    {
        FreeResult r;
        if (!data_ || !queue_) {
            r.event = sycl::event{};
            r.status = {};
            return r;
        }
        try {
            // Free immediately; barrier gives callers an event to wait on.
            sycl::free(data_, *queue_);
            data_ = nullptr;
            bytes_ = 0;
            r.event = queue_->ext_oneapi_submit_barrier();
            r.status = {};
        } catch (const sycl::exception& e) {
            r.event = sycl::event{};
            r.status = std::unexpected(ice::Status::from_message(e.what()));
        }
        return r;
    }

    // ---- async copy operations -------------------------------------------

    [[nodiscard]] CopyResult copy_from_host_async(const void* src, size_t bytes) noexcept
    {
        CopyResult r;
        if (!data_ || !queue_ || bytes > bytes_) {
            r.event = sycl::event{};
            r.status = std::unexpected(ice::Status::from_message("invalid copy bounds"));
            return r;
        }
        try {
            r.event = queue_->memcpy(data_, src, bytes);
            r.status = {};
        } catch (const sycl::exception& e) {
            r.event = sycl::event{};
            r.status = std::unexpected(ice::Status::from_message(e.what()));
        }
        return r;
    }

    [[nodiscard]] CopyResult copy_to_host_async(void* dst, size_t bytes) noexcept
    {
        CopyResult r;
        if (!data_ || !queue_ || bytes > bytes_) {
            r.event = sycl::event{};
            r.status = std::unexpected(ice::Status::from_message("invalid copy bounds"));
            return r;
        }
        try {
            r.event = queue_->memcpy(dst, data_, bytes);
            r.status = {};
        } catch (const sycl::exception& e) {
            r.event = sycl::event{};
            r.status = std::unexpected(ice::Status::from_message(e.what()));
        }
        return r;
    }

    [[nodiscard]] CopyResult copy_from_async(
        const XPU_AsyncBuffer& src,
        size_t bytes,
        size_t src_offset = 0,
        size_t dst_offset = 0
    ) noexcept
    {
        CopyResult r;
        if (!data_ || !src.data_ || !queue_ || src_offset + bytes > src.bytes_ ||
            dst_offset + bytes > bytes_) {
            r.event = sycl::event{};
            r.status = std::unexpected(ice::Status::from_message("invalid copy bounds"));
            return r;
        }
        try {
            r.event = queue_->memcpy(
                static_cast<char*>(data_) + dst_offset,
                static_cast<const char*>(src.data_) + src_offset,
                bytes
            );
            r.status = {};
        } catch (const sycl::exception& e) {
            r.event = sycl::event{};
            r.status = std::unexpected(ice::Status::from_message(e.what()));
        }
        return r;
    }

    [[nodiscard]] CopyResult memset_async(int val, size_t bytes, size_t offset = 0) noexcept
    {
        CopyResult r;
        if (!data_ || !queue_ || offset + bytes > bytes_) {
            r.event = sycl::event{};
            r.status = std::unexpected(ice::Status::from_message("invalid memset bounds"));
            return r;
        }
        try {
            r.event = queue_->memset(static_cast<char*>(data_) + offset, val, bytes);
            r.status = {};
        } catch (const sycl::exception& e) {
            r.event = sycl::event{};
            r.status = std::unexpected(ice::Status::from_message(e.what()));
        }
        return r;
    }

    // ---- synchronization -------------------------------------------------

    [[nodiscard]] sycl::event record_barrier() noexcept
    {
        return queue_ ? queue_->ext_oneapi_submit_barrier() : sycl::event{};
    }

    void synchronize()
    {
        if (queue_) {
            queue_->wait_and_throw();
        }
    }

    // ---- accessors -------------------------------------------------------

    [[nodiscard]] void* data() const noexcept
    {
        return data_;
    }

    [[nodiscard]] size_t size() const noexcept
    {
        return bytes_;
    }

    [[nodiscard]] bool empty() const noexcept
    {
        return data_ == nullptr;
    }

    [[nodiscard]] sycl::queue* queue() const noexcept
    {
        return queue_;
    }

private:
    sycl::queue* queue_ = nullptr;
    void* data_ = nullptr;
    size_t bytes_ = 0;

    void free_usm() noexcept
    {
        if (data_ && queue_) {
            sycl::free(data_, *queue_);
            data_ = nullptr;
            bytes_ = 0;
        }
    }
};

// ---------------------------------------------------------------------------
// XPU_PooledAsyncBuffer — same interface, but uses a labelled allocation pool.
// In SYCL, "pools" are modelled by keeping a sycl::context-level allocator;
// here we mirror the public API shape so users can switch without API change.
// ---------------------------------------------------------------------------
class XPU_PooledAsyncBuffer
{
public:
    struct PoolId
    {
        uint64_t hi = 0;
        uint64_t lo = 0;
    };

    XPU_PooledAsyncBuffer() = default;

    XPU_PooledAsyncBuffer(size_t bytes, PoolId pool_id, sycl::queue& q) :
        pool_id_(pool_id)
    {
        inner_ = XPU_AsyncBuffer(bytes, q);
    }

    // Delegate all operations to XPU_AsyncBuffer
    using AllocResult = XPU_AsyncBuffer::AllocResult;
    using FreeResult = XPU_AsyncBuffer::FreeResult;
    using CopyResult = XPU_AsyncBuffer::CopyResult;

    [[nodiscard]] static AllocResult
    allocate_async(size_t bytes, PoolId /*pool_id*/, sycl::queue& q) noexcept
    {
        return XPU_AsyncBuffer::allocate_async(bytes, q);
    }

    [[nodiscard]] void* data() const noexcept
    {
        return inner_.data();
    }

    [[nodiscard]] size_t size() const noexcept
    {
        return inner_.size();
    }

    [[nodiscard]] bool empty() const noexcept
    {
        return inner_.empty();
    }

    [[nodiscard]] PoolId pool_id() const noexcept
    {
        return pool_id_;
    }

    XPU_PooledAsyncBuffer(XPU_PooledAsyncBuffer&&) = default;
    XPU_PooledAsyncBuffer& operator=(XPU_PooledAsyncBuffer&&) = default;

private:
    XPU_AsyncBuffer inner_;
    PoolId pool_id_;
};

} // namespace ice::builder
