// Async Tensor Operations for XPU/SYCL — ice::builder integration
// Provides async tensor operations using SYCL events and queues
// Reference implementation for ice::builder::TF_TensorOps with async support

module;

#include <sycl/sycl.hpp>
#include <ATen/xpu/XPUEvent.h>
#include <ATen/xpu/XPUStream.h>
#include <c10/xpu/XPUFunctions.h>
#include <c10/xpu/XPUCachingAllocator.h>

export module cc_ice_builder_intern:xpu_async_tensor;

import std;
import cc_ice_builder_intern:tensor;
import cc_ice_builder_intern:status;
import cc_ice_builder_intern:datatype;

export namespace ice::builder {

// Forward declarations
class XPU_AsyncTensorOps;
class XPU_AsyncEvent;

// Async operation result with SYCL event
struct AsyncResult {
    sycl::event event;
    std::expected<void, Status> status;
    
    // Wait for completion and return status
    std::expected<void, Status> wait() noexcept {
        try {
            event.wait();
            return status;
        } catch (const sycl::exception& e) {
            return std::unexpected(Status::from_sycl_error(e));
        }
    }
    
    // Non-blocking check
    bool is_ready() const noexcept {
        return event.get_info<sycl::info::event::command_execution_status>() 
               == sycl::info::event_command_status::complete;
    }
    
    // Chain continuation
    template<typename F>
    auto then(F&& f) noexcept -> decltype(f(std::declval<AsyncResult>())) {
        return event.then([f = std::forward<F>(f)](sycl::event e) mutable {
            AsyncResult r{std::move(e), std::move(status)};
            return f(std::move(r));
        });
    }
};

// Async tensor operations vtable with full async support
class XPU_AsyncTensorOps final : public TF_TensorOps {
    at::Tensor tensor_;
    c10::xpu::XPUStream stream_;
    std::shared_ptr<sycl::queue> queue_;
    
public:
    // Constructor with optional stream/queue for async ops
    explicit XPU_AsyncTensorOps(at::Tensor tensor = at::empty_xpu({}, at::kFloat),
                                 c10::xpu::XPUStream stream = c10::xpu::getCurrentXPUStream())
        : tensor_(std::move(tensor)), stream_(stream), 
          queue_(c10::xpu::getRawStream(stream_)) {}
    
    explicit XPU_AsyncTensorOps(const TensorOptions& options,
                                 c10::xpu::XPUStream stream = c10::xpu::getCurrentXPUStream())
        : tensor_(at::empty_xpu({}, options)), stream_(stream),
          queue_(c10::xpu::getRawStream(stream_)) {}

    // --- Stream/Queue access ---
    c10::xpu::XPUStream stream() const noexcept { return stream_; }
    sycl::queue& queue() noexcept { return *queue_; }
    const sycl::queue& queue() const noexcept { return *queue_; }
    
    void set_stream(c10::xpu::XPUStream s) noexcept {
        stream_ = s;
        queue_ = c10::xpu::getRawStream(s);
    }

    // --- Sync wrappers (required vtable) ---
    std::expected<void, Status> set_dtype(TFDataTypeEnum dt) override {
        auto st = from_ice_dtype(dt);
        tensor_ = tensor_.to(st);
        return {};
    }
    
    std::expected<void, Status> set_dims(const int64_t* dims, int n) override {
        tensor_ = at::empty_xpu({dims, dims + n}, tensor_.options());
        return {};
    }
    
    std::expected<void, Status> set_byte_size(size_t len) override {
        auto* alloc = c10::GetAllocator(c10::kXPU);
        auto storage = c10::Storage::create(len, alloc, true);
        tensor_ = at::Tensor(std::move(storage)).view({-1});
        return {};
    }
    
    std::expected<void, Status> delete_tensor() override {
        tensor_ = at::Tensor();
        return {};
    }
    
    std::expected<void, Status> tensor_type(TFDataTypeEnum* out) override {
        *out = to_ice_dtype(tensor_.scalar_type());
        return {};
    }
    
    std::expected<void, Status> num_dims(int* out) override {
        *out = tensor_.dim();
        return {};
    }
    
    std::expected<void, Status> dim(int idx, int64_t* out) override {
        *out = tensor_.size(idx);
        return {};
    }
    
    std::expected<void, Status> tensor_element_count(int64_t* out) override {
        *out = tensor_.numel();
        return {};
    }
    
    std::expected<void, Status> tensor_byte_size(size_t* out) override {
        *out = tensor_.nbytes();
        return {};
    }
    
    std::expected<void, Status> tensor_data(void** out) override {
        *out = tensor_.data_ptr();
        return {};
    }
    
    std::expected<void, Status> tensor_bitcast_from(TFDataTypeEnum dt, TF_Tensor** out) override {
        auto view = tensor_.view(from_ice_dtype(dt));
        *out = wrap_in_tensor_ops(std::move(view));
        return {};
    }
    
    std::expected<void, Status> tensor_bitcast_to(TFDataTypeEnum dt, TF_Tensor** out) override {
        return tensor_bitcast_from(dt, out);
    }
    
    std::expected<void, Status> tensor_copy(const TF_TensorOps& dst) override {
        auto* xpu_dst = dynamic_cast<const XPU_AsyncTensorOps*>(&dst);
        if (!xpu_dst) return std::unexpected(Status::InvalidArgument);
        
        // Sync copy on current stream
        tensor_.copy_(xpu_dst->tensor_);
        return {};
    }

    // ==================== ASYNC OPERATIONS ====================
    
    // Async copy with event return
    [[nodiscard]] AsyncResult copy_async(const XPU_AsyncTensorOps& dst) noexcept {
        try {
            auto event = queue_->submit([&](sycl::handler& h) {
                // Use SYCL memcpy for async copy
                h.memcpy(dst.tensor_.mutable_data_ptr(), tensor_.data_ptr(), tensor_.nbytes());
            });
            return AsyncResult{event, {}};
        } catch (const sycl::exception& e) {
            return AsyncResult{sycl::event{}, std::unexpected(Status::from_sycl_error(e))};
        }
    }
    
    // Async copy to host
    [[nodiscard]] AsyncResult copy_to_host_async(void* host_ptr, size_t bytes) noexcept {
        try {
            auto event = queue_->submit([&](sycl::handler& h) {
                h.memcpy(host_ptr, tensor_.data_ptr(), bytes);
            });
            return AsyncResult{event, {}};
        } catch (const sycl::exception& e) {
            return AsyncResult{sycl::event{}, std::unexpected(Status::from_sycl_error(e))};
        }
    }
    
    // Async copy from host
    [[nodiscard]] AsyncResult copy_from_host_async(const void* host_ptr, size_t bytes) noexcept {
        try {
            auto event = queue_->submit([&](sycl::handler& h) {
                h.memcpy(tensor_.mutable_data_ptr(), host_ptr, bytes);
            });
            return AsyncResult{event, {}};
        } catch (const sycl::exception& e) {
            return AsyncResult{sycl::event{}, std::unexpected(Status::from_sycl_error(e))};
        }
    }
    
    // Async fill
    [[nodiscard]] AsyncResult fill_async(const Scalar& value) noexcept {
        try {
            auto event = queue_->submit([&](sycl::handler& h) {
                // Use parallel_for for fill
                h.parallel_for(sycl::range<1>(tensor_.numel()), [=](sycl::id<1> idx) {
                    // Type-specific fill would need template instantiation
                    // This is a simplified version
                });
            });
            return AsyncResult{event, {}};
        } catch (const sycl::exception& e) {
            return AsyncResult{sycl::event{}, std::unexpected(Status::from_sycl_error(e))};
        }
    }
    
    // Async kernel launch (generic)
    template<typename KernelFn>
    [[nodiscard]] AsyncResult launch_kernel_async(KernelFn&& kernel, sycl::range<3> global, sycl::range<3> local = {}) noexcept {
        try {
            auto event = queue_->submit([&](sycl::handler& h) {
                if (local.size() > 0) {
                    h.parallel_for(sycl::nd_range<3>(global, local), std::forward<KernelFn>(kernel));
                } else {
                    h.parallel_for(global, std::forward<KernelFn>(kernel));
                }
            });
            return AsyncResult{event, {}};
        } catch (const sycl::exception& e) {
            return AsyncResult{sycl::event{}, std::unexpected(Status::from_sycl_error(e))};
        }
    }
    
    // Record event for synchronization
    [[nodiscard]] sycl::event record_event() noexcept {
        return queue_->ext_oneapi_submit_barrier();
    }
    
    // Wait for all operations on this stream
    void synchronize() noexcept {
        queue_->wait_and_throw();
    }
    
    // Factory
    static TF_TensorOps* create() {
        return new XPU_AsyncTensorOps();
    }
    
    static void destroy(TF_TensorOps* ops) {
        delete static_cast<XPU_AsyncTensorOps*>(ops);
    }

private:
    // Helper to wrap tensor in new ops instance
    static TF_Tensor* wrap_in_tensor_ops(at::Tensor t) {
        auto* ops = new XPU_AsyncTensorOps(std::move(t));
        auto* tensor = new TF_Tensor{ops};
        return tensor;
    }
    
    // Dtype conversion helpers
    static TFDataTypeEnum to_ice_dtype(c10::ScalarType t) {
        switch (t) {
            case c10::kFloat: return TF_FLOAT;
            case c10::kDouble: return TF_DOUBLE;
            case c10::kInt: return TF_INT32;
            case c10::kLong: return TF_INT64;
            case c10::kHalf: return TF_HALF;
            case c10::kBFloat16: return TF_BFLOAT16;
            case c10::kBool: return TF_BOOL;
            case c10::kByte: return TF_UINT8;
            case c10::kChar: return TF_INT8;
            case c10::kShort: return TF_INT16;
            case c10::kComplexHalf: return TF_COMPLEX64;
            case c10::kComplexFloat: return TF_COMPLEX128;
            case c10::kQInt8: return TF_QINT8;
            case c10::kQUInt8: return TF_QUINT8;
            case c10::kQInt32: return TF_QINT32;
            case c10::kQUInt4x2: return TF_QUINT16;
            case c10::kQUInt2x4: return TF_QUINT8;
            default: return TF_FLOAT;
        }
    }
    
    static c10::ScalarType from_ice_dtype(TFDataTypeEnum dt) {
        switch (dt) {
            case TF_FLOAT: return c10::kFloat;
            case TF_DOUBLE: return c10::kDouble;
            case TF_INT32: return c10::kInt;
            case TF_INT64: return c10::kLong;
            case TF_HALF: return c10::kHalf;
            case TF_BFLOAT16: return c10::kBFloat16;
            case TF_BOOL: return c10::kBool;
            case TF_UINT8: return c10::kByte;
            case TF_INT8: return c10::kChar;
            case TF_INT16: return c10::kShort;
            case TF_COMPLEX64: return c10::kComplexHalf;
            case TF_COMPLEX128: return c10::kComplexFloat;
            case TF_QINT8: return c10::kQInt8;
            case TF_QUINT8: return c10::kQUInt8;
            case TF_QINT32: return c10::kQInt32;
            default: return c10::kFloat;
        }
    }
};

// Async event wrapper for cross-operation synchronization
class XPU_AsyncEvent {
    sycl::event event_;
    std::shared_ptr<sycl::queue> queue_;
    
public:
    XPU_AsyncEvent() = default;
    explicit XPU_AsyncEvent(sycl::event e, std::shared_ptr<sycl::queue> q = nullptr)
        : event_(std::move(e)), queue_(std::move(q)) {}
    
    void wait() const { event_.wait(); }
    
    bool is_ready() const {
        return event_.get_info<sycl::info::event::command_execution_status>() 
               == sycl::info::event_command_status::complete;
    }
    
    // Create event from current stream
    static XPU_AsyncEvent record(c10::xpu::XPUStream stream = c10::xpu::getCurrentXPUStream()) {
        auto* q = c10::xpu::getRawStream(stream);
        auto event = q->ext_oneapi_submit_barrier();
        return XPU_AsyncEvent(event, std::shared_ptr<sycl::queue>(q, [](auto*){}));
    }
    
    // Wait for this event on another stream
    void wait_on(c10::xpu::XPUStream stream) {
        auto* q = c10::xpu::getRawStream(stream);
        q->wait(event_);
    }
    
    // Chain: wait for this event, then execute on stream
    template<typename F>
    void then_on(c10::xpu::XPUStream stream, F&& f) {
        auto* q = c10::xpu::getRawStream(stream);
        q->submit([&](sycl::handler& h) {
            h.depends_on(event_);
            h.single_task(std::forward<F>(f));
        });
    }
};

// Async buffer operations for ice::builder::Buffer
class XPU_AsyncBufferOps {
    c10::DataPtr data_;
    std::shared_ptr<sycl::queue> queue_;
    
public:
    XPU_AsyncBufferOps() = default;
    explicit XPU_AsyncBufferOps(size_t bytes, c10::xpu::XPUStream stream = c10::xpu::getCurrentXPUStream()) {
        queue_ = c10::xpu::getRawStream(stream);
        data_ = c10::xpu::XPUCachingAllocator::raw_alloc(bytes);
    }
    
    // Async allocate
    [[nodiscard]] static AsyncResult allocate_async(size_t bytes, 
                                                     c10::xpu::XPUStream stream = c10::xpu::getCurrentXPUStream()) {
        auto* q = c10::xpu::getRawStream(stream);
        try {
            auto event = q->submit([&](sycl::handler& h) {
                // Allocation is sync, but we return event for chaining
                h.single_task([]{});
            });
            auto ptr = c10::xpu::XPUCachingAllocator::raw_alloc(bytes);
            return AsyncResult{event, ptr ? std::expected<void, Status>{} 
                                          : std::unexpected(Status::OOM)};
        } catch (const sycl::exception& e) {
            return AsyncResult{sycl::event{}, std::unexpected(Status::from_sycl_error(e))};
        }
    }
    
    // Async deallocate
    [[nodiscard]] AsyncResult deallocate_async() noexcept {
        if (!data_) return AsyncResult{sycl::event{}, {}};
        try {
            auto event = queue_->submit([&](sycl::handler& h) {
                h.single_task([ptr = data_.get()] { 
                    c10::xpu::XPUCachingAllocator::raw_delete(ptr); 
                });
            });
            data_ = nullptr;
            return AsyncResult{event, {}};
        } catch (const sycl::exception& e) {
            return AsyncResult{sycl::event{}, std::unexpected(Status::from_sycl_error(e))};
        }
    }
    
    void* data() const noexcept { return data_.get(); }
    size_t size() const noexcept { return data_ ? data_->size() : 0; }
    bool empty() const noexcept { return !data_; }
    
    // Move support
    XPU_AsyncBufferOps(XPU_AsyncBufferOps&&) = default;
    XPU_AsyncBufferOps& operator=(XPU_AsyncBufferOps&&) = default;
};

} // namespace ice::builder