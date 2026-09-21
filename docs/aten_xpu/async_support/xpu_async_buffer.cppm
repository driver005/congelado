// Async Buffer Operations for XPU/SYCL — ice::builder integration
// Provides async buffer allocation/deallocation using XPUCachingAllocator

module;

#include <sycl/sycl.hpp>
#include <c10/xpu/XPUCachingAllocator.h>
#include <c10/xpu/XPUStream.h>

export module cc_ice_builder_intern:xpu_async_buffer;

import std;
import cc_ice_builder_intern:status;

export namespace ice::builder {

// Async buffer with SYCL event-based synchronization
class XPU_AsyncBuffer {
    c10::DataPtr data_;
    std::shared_ptr<sycl::queue> queue_;
    size_t bytes_ = 0;
    
public:
    XPU_AsyncBuffer() = default;
    
    explicit XPU_AsyncBuffer(size_t bytes, 
                             c10::xpu::XPUStream stream = c10::xpu::getCurrentXPUStream()) {
        queue_ = c10::xpu::getRawStream(stream);
        allocate(bytes);
    }
    
    ~XPU_AsyncBuffer() {
        if (data_) deallocate();
    }
    
    // Non-copyable, movable
    XPU_AsyncBuffer(const XPU_AsyncBuffer&) = delete;
    XPU_AsyncBuffer& operator=(const XPU_AsyncBuffer&) = delete;
    XPU_AsyncBuffer(XPU_AsyncBuffer&& other) noexcept
        : data_(std::move(other.data_)), queue_(std::move(other.queue_)), bytes_(other.bytes_) {
        other.bytes_ = 0;
    }
    XPU_AsyncBuffer& operator=(XPU_AsyncBuffer&& other) noexcept {
        if (this != &other) {
            if (data_) deallocate();
            data_ = std::move(other.data_);
            queue_ = std::move(other.queue_);
            bytes_ = other.bytes_;
            other.bytes_ = 0;
        }
        return *this;
    }
    
    // Sync allocation
    void allocate(size_t bytes) {
        if (data_) deallocate();
        data_ = c10::xpu::XPUCachingAllocator::raw_alloc(bytes);
        bytes_ = data_ ? bytes : 0;
    }
    
    // Sync deallocation
    void deallocate() {
        if (data_) {
            c10::xpu::XPUCachingAllocator::raw_delete(data_.get());
            data_ = nullptr;
            bytes_ = 0;
        }
    }
    
    // Async allocation with event
    struct AllocResult {
        sycl::event event;
        std::expected<void, Status> status;
        XPU_AsyncBuffer buffer;
        
        std::expected<void, Status> wait() {
            try {
                event.wait();
                return status;
            } catch (const sycl::exception& e) {
                return std::unexpected(Status::from_sycl_error(e));
            }
        }
    };
    
    [[nodiscard]] static AllocResult allocate_async(
        size_t bytes, 
        c10::xpu::XPUStream stream = c10::xpu::getCurrentXPUStream()) {
        auto* q = c10::xpu::getRawStream(stream);
        AllocResult result;
        result.buffer.queue_ = std::shared_ptr<sycl::queue>(q, [](auto*){});
        
        try {
            result.event = q->submit([&](sycl::handler& h) {
                h.single_task([]{});
            });
            
            result.buffer.data_ = c10::xpu::XPUCachingAllocator::raw_alloc(bytes);
            result.buffer.bytes_ = result.buffer.data_ ? bytes : 0;
            result.status = result.buffer.data_ ? std::expected<void, Status>{} 
                                                 : std::unexpected(Status::OOM);
        } catch (const sycl::exception& e) {
            result.event = sycl::event{};
            result.status = std::unexpected(Status::from_sycl_error(e));
        }
        
        return result;
    }
    
    // Async deallocation with event
    struct FreeResult {
        sycl::event event;
        std::expected<void, Status> status;
        
        std::expected<void, Status> wait() {
            try {
                event.wait();
                return status;
            } catch (const sycl::exception& e) {
                return std::unexpected(Status::from_sycl_error(e));
            }
        }
    };
    
    [[nodiscard]] FreeResult deallocate_async() {
        FreeResult result;
        if (!data_) {
            result.event = sycl::event{};
            result.status = std::expected<void, Status>{};
            return result;
        }
        
        try {
            result.event = queue_->submit([&](sycl::handler& h) {
                h.single_task([ptr = data_.get()] {
                    c10::xpu::XPUCachingAllocator::raw_delete(ptr);
                });
            });
            data_ = nullptr;
            bytes_ = 0;
            result.status = std::expected<void, Status>{};
        } catch (const sycl::exception& e) {
            result.event = sycl::event{};
            result.status = std::unexpected(Status::from_sycl_error(e));
        }
        
        return result;
    }
    
    // Async copy from host
    struct CopyResult {
        sycl::event event;
        std::expected<void, Status> status;
        
        std::expected<void, Status> wait() {
            try {
                event.wait();
                return status;
            } catch (const sycl::exception& e) {
                return std::unexpected(Status::from_sycl_error(e));
            }
        }
    };
    
    [[nodiscard]] CopyResult copy_from_host_async(const void* host_ptr, size_t bytes) {
        CopyResult result;
        if (!data_ || bytes > bytes_) {
            result.event = sycl::event{};
            result.status = std::unexpected(Status::InvalidArgument);
            return result;
        }
        
        try {
            result.event = queue_->submit([&](sycl::handler& h) {
                h.memcpy(data_.get(), host_ptr, bytes);
            });
            result.status = std::expected<void, Status>{};
        } catch (const sycl::exception& e) {
            result.event = sycl::event{};
            result.status = std::unexpected(Status::from_sycl_error(e));
        }
        
        return result;
    }
    
    // Async copy to host
    [[nodiscard]] CopyResult copy_to_host_async(void* host_ptr, size_t bytes) {
        CopyResult result;
        if (!data_ || bytes > bytes_) {
            result.event = sycl::event{};
            result.status = std::unexpected(Status::InvalidArgument);
            return result;
        }
        
        try {
            result.event = queue_->submit([&](sycl::handler& h) {
                h.memcpy(host_ptr, data_.get(), bytes);
            });
            result.status = std::expected<void, Status>{};
        } catch (const sycl::exception& e) {
            result.event = sycl::event{};
            result.status = std::unexpected(Status::from_sycl_error(e));
        }
        
        return result;
    }
    
    // Async copy between buffers
    [[nodiscard]] CopyResult copy_from_async(const XPU_AsyncBuffer& src, size_t bytes, size_t src_offset = 0, size_t dst_offset = 0) {
        CopyResult result;
        if (!data_ || !src.data_ || src_offset + bytes > src.bytes_ || dst_offset + bytes > bytes_) {
            result.event = sycl::event{};
            result.status = std::unexpected(Status::InvalidArgument);
            return result;
        }
        
        try {
            result.event = queue_->submit([&](sycl::handler& h) {
                h.memcpy(static_cast<char*>(data_.get()) + dst_offset,
                         static_cast<const char*>(src.data_.get()) + src_offset,
                         bytes);
            });
            result.status = std::expected<void, Status>{};
        } catch (const sycl::exception& e) {
            result.event = sycl::event{};
            result.status = std::unexpected(Status::from_sycl_error(e));
        }
        
        return result;
    }
    
    // Async memset
    [[nodiscard]] CopyResult memset_async(int value, size_t bytes, size_t offset = 0) {
        CopyResult result;
        if (!data_ || offset + bytes > bytes_) {
            result.event = sycl::event{};
            result.status = std::unexpected(Status::InvalidArgument);
            return result;
        }
        
        try {
            result.event = queue_->submit([&](sycl::handler& h) {
                h.memset(data_.get(), value, bytes);
            });
            result.status = std::expected<void, Status>{};
        } catch (const sycl::exception& e) {
            result.event = sycl::event{};
            result.status = std::unexpected(Status::from_sycl_error(e));
        }
        
        return result;
    }
    
    // Accessors
    void* data() const noexcept { return data_.get(); }
    size_t size() const noexcept { return bytes_; }
    bool empty() const noexcept { return !data_; }
    c10::xpu::XPUStream stream() const noexcept { 
        return queue_ ? c10::xpu::XPUStream(queue_) : c10::xpu::getCurrentXPUStream(); 
    }
    sycl::queue& queue() noexcept { return *queue_; }
    
    // Record event for synchronization
    sycl::event record_event() {
        return queue_->ext_oneapi_submit_barrier();
    }
    
    // Wait for all operations
    void synchronize() {
        queue_->wait_and_throw();
    }
};

// Pool-aware async buffer (uses mempool for faster allocation)
class XPU_PooledAsyncBuffer {
    c10::DataPtr data_;
    std::shared_ptr<sycl::queue> queue_;
    size_t bytes_ = 0;
    c10::MempoolId_t mempool_id_ = {0, 0};
    
public:
    XPU_PooledAsyncBuffer() = default;
    
    XPU_PooledAsyncBuffer(size_t bytes, c10::MempoolId_t pool_id,
                          c10::xpu::XPUStream stream = c10::xpu::getCurrentXPUStream())
        : queue_(c10::xpu::getRawStream(stream)), mempool_id_(pool_id) {
        allocate(bytes);
    }
    
    ~XPU_PooledAsyncBuffer() {
        if (data_) deallocate();
    }
    
    XPU_PooledAsyncBuffer(const XPU_PooledAsyncBuffer&) = delete;
    XPU_PooledAsyncBuffer& operator=(const XPU_PooledAsyncBuffer&) = delete;
    XPU_PooledAsyncBuffer(XPU_PooledAsyncBuffer&&) = default;
    XPU_PooledAsyncBuffer& operator=(XPU_PooledAsyncBuffer&&) = default;
    
    void allocate(size_t bytes) {
        if (data_) deallocate();
        c10::xpu::XPUCachingAllocator::createOrIncrefPool(
            c10::xpu::current_device(), mempool_id_);
        data_ = c10::xpu::XPUCachingAllocator::raw_alloc(bytes);
        bytes_ = data_ ? bytes : 0;
    }
    
    void deallocate() {
        if (data_) {
            c10::xpu::XPUCachingAllocator::raw_delete(data_.get());
            data_ = nullptr;
            bytes_ = 0;
        }
    }
    
    // Same async interface as XPU_AsyncBuffer
    using AllocResult = XPU_AsyncBuffer::AllocResult;
    using FreeResult = XPU_AsyncBuffer::FreeResult;
    using CopyResult = XPU_AsyncBuffer::CopyResult;
    
    [[nodiscard]] static AllocResult allocate_async(
        size_t bytes, c10::MempoolId_t pool_id,
        c10::xpu::XPUStream stream = c10::xpu::getCurrentXPUStream()) {
        auto* q = c10::xpu::getRawStream(stream);
        AllocResult result;
        result.buffer.queue_ = std::shared_ptr<sycl::queue>(q, [](auto*){});
        result.buffer.mempool_id_ = pool_id;
        
        try {
            result.event = q->submit([&](sycl::handler& h) { h.single_task([]{}); });
            c10::xpu::XPUCachingAllocator::createOrIncrefPool(
                c10::xpu::current_device(), pool_id);
            result.buffer.data_ = c10::xpu::XPUCachingAllocator::raw_alloc(bytes);
            result.buffer.bytes_ = result.buffer.data_ ? bytes : 0;
            result.status = result.buffer.data_ ? std::expected<void, Status>{} 
                                                 : std::unexpected(Status::OOM);
        } catch (const sycl::exception& e) {
            result.event = sycl::event{};
            result.status = std::unexpected(Status::from_sycl_error(e));
        }
        
        return result;
    }
    
    // ... (same async methods as XPU_AsyncBuffer)
    void* data() const noexcept { return data_.get(); }
    size_t size() const noexcept { return bytes_; }
    bool empty() const noexcept { return !data_; }
    c10::MempoolId_t pool_id() const noexcept { return mempool_id_; }
};

} // namespace ice::builder