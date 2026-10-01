// SYCL Tensor Backend — ice::builder::TF_TensorOps implementation
// Implements the ice tensor interface using AdaptiveCpp SYCL USM device memory.
// NO ATen / c10 / PyTorch dependencies.

module;

// SYCL must live in the global module fragment (not inside the module itself).
#include "include/c/intern/tensor.h"

#include <sycl/sycl.hpp>

export module cc_ice_sycl_backend:tensor;

import std;
import cc_ice_builder_intern:tensor;
import cc_ice_builder_intern:status;
import cc_ice_builder_intern:datatype;

export namespace ice::builder {

// ---------------------------------------------------------------------------
// dtype helpers — map TFDataTypeEnum ↔ byte-width (no c10 required)
// ---------------------------------------------------------------------------
namespace detail {

    [[nodiscard]] constexpr size_t dtype_element_size(TFDataTypeEnum dt) noexcept
    {
        switch (dt) {
            case TF_FLOAT:
                return 4;
            case TF_DOUBLE:
                return 8;
            case TF_INT32:
                return 4;
            case TF_UINT32:
                return 4;
            case TF_INT64:
                return 8;
            case TF_UINT64:
                return 8;
            case TF_INT16:
                return 2;
            case TF_UINT16:
                return 2;
            case TF_INT8:
                return 1;
            case TF_UINT8:
                return 1;
            case TF_HALF:
                return 2;
            case TF_BFLOAT16:
                return 2;
            case TF_BOOL:
                return 1;
            case TF_COMPLEX64:
                return 8;
            case TF_COMPLEX128:
                return 16;
            case TF_QINT8:
                return 1;
            case TF_QUINT8:
                return 1;
            case TF_QINT16:
                return 2;
            case TF_QUINT16:
                return 2;
            case TF_QINT32:
                return 4;
            default:
                return 4; // conservative fallback
        }
    }

} // namespace detail

// ---------------------------------------------------------------------------
// AsyncResult — lightweight future: sycl::event + ice::Status
// ---------------------------------------------------------------------------
struct AsyncResult
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

// ---------------------------------------------------------------------------
// SyclTensorImpl — owns USM device memory, shape, and dtype
// ---------------------------------------------------------------------------
class SyclTensorImpl
{
public:
    SyclTensorImpl() = default;

    explicit SyclTensorImpl(sycl::queue& q) :
        queue_(&q)
    {
    }

    ~SyclTensorImpl()
    {
        free_device_memory();
    }

    // Non-copyable; movable
    SyclTensorImpl(const SyclTensorImpl&) = delete;
    SyclTensorImpl& operator=(const SyclTensorImpl&) = delete;

    SyclTensorImpl(SyclTensorImpl&& o) noexcept :
        queue_(o.queue_),
        data_(o.data_),
        dims_(std::move(o.dims_)),
        dtype_(o.dtype_)
    {
        o.data_ = nullptr;
        o.queue_ = nullptr;
    }

    SyclTensorImpl& operator=(SyclTensorImpl&& o) noexcept
    {
        if (this != &o) {
            free_device_memory();
            queue_ = o.queue_;
            data_ = o.data_;
            dims_ = std::move(o.dims_);
            dtype_ = o.dtype_;
            o.data_ = nullptr;
            o.queue_ = nullptr;
        }
        return *this;
    }

    // ---- mutation ---------------------------------------------------------

    [[nodiscard]] std::expected<void, ice::Status> set_dtype(TFDataTypeEnum dt) noexcept
    {
        dtype_ = dt;
        return reallocate();
    }

    [[nodiscard]] std::expected<void, ice::Status> set_dims(const int64_t* d, int n) noexcept
    {
        dims_.assign(d, d + n);
        return reallocate();
    }

    [[nodiscard]] std::expected<void, ice::Status> set_byte_size(size_t bytes) noexcept
    {
        free_device_memory();
        dims_ = {static_cast<int64_t>(bytes)};
        dtype_ = TF_UINT8; // treat as raw byte buffer
        return alloc_bytes(bytes);
    }

    [[nodiscard]] std::expected<void, ice::Status> delete_data() noexcept
    {
        free_device_memory();
        dims_.clear();
        return {};
    }

    // ---- query ------------------------------------------------------------

    [[nodiscard]] TFDataTypeEnum dtype() const noexcept
    {
        return dtype_;
    }

    [[nodiscard]] int ndim() const noexcept
    {
        return static_cast<int>(dims_.size());
    }

    [[nodiscard]] int64_t dim(int i) const noexcept
    {
        return dims_[i];
    }

    [[nodiscard]] void* data() const noexcept
    {
        return data_;
    }

    [[nodiscard]] int64_t numel() const noexcept
    {
        int64_t n = 1;
        for (auto d: dims_) {
            n *= d;
        }
        return n;
    }

    [[nodiscard]] size_t nbytes() const noexcept
    {
        return static_cast<size_t>(numel()) * detail::dtype_element_size(dtype_);
    }

    [[nodiscard]] sycl::queue* queue() const noexcept
    {
        return queue_;
    }

    // ---- async ops --------------------------------------------------------

    [[nodiscard]] AsyncResult copy_to_host_async(void* dst, size_t bytes) noexcept
    {
        if (!data_ || !queue_) {
            return {sycl::event{}, std::unexpected(ice::Status::from_message("no allocation"))};
        }
        try {
            auto ev = queue_->memcpy(dst, data_, bytes);
            return {ev, {}};
        } catch (const sycl::exception& e) {
            return {sycl::event{}, std::unexpected(ice::Status::from_message(e.what()))};
        }
    }

    [[nodiscard]] AsyncResult copy_from_host_async(const void* src, size_t bytes) noexcept
    {
        if (!data_ || !queue_) {
            return {sycl::event{}, std::unexpected(ice::Status::from_message("no allocation"))};
        }
        try {
            auto ev = queue_->memcpy(data_, src, bytes);
            return {ev, {}};
        } catch (const sycl::exception& e) {
            return {sycl::event{}, std::unexpected(ice::Status::from_message(e.what()))};
        }
    }

    [[nodiscard]] AsyncResult
    copy_from_device_async(const SyclTensorImpl& src, size_t bytes) noexcept
    {
        if (!data_ || !src.data_ || !queue_) {
            return {sycl::event{}, std::unexpected(ice::Status::from_message("no allocation"))};
        }
        try {
            auto ev = queue_->memcpy(data_, src.data_, bytes);
            return {ev, {}};
        } catch (const sycl::exception& e) {
            return {sycl::event{}, std::unexpected(ice::Status::from_message(e.what()))};
        }
    }

    [[nodiscard]] AsyncResult memset_async(int val, size_t bytes) noexcept
    {
        if (!data_ || !queue_) {
            return {sycl::event{}, std::unexpected(ice::Status::from_message("no allocation"))};
        }
        try {
            auto ev = queue_->memset(data_, val, bytes);
            return {ev, {}};
        } catch (const sycl::exception& e) {
            return {sycl::event{}, std::unexpected(ice::Status::from_message(e.what()))};
        }
    }

    [[nodiscard]] sycl::event record_barrier() noexcept
    {
        return queue_->ext_oneapi_submit_barrier();
    }

    void synchronize()
    {
        queue_->wait_and_throw();
    }

private:
    sycl::queue* queue_ = nullptr;
    void* data_ = nullptr;
    std::vector<int64_t> dims_;
    TFDataTypeEnum dtype_ = TF_FLOAT;

    void free_device_memory() noexcept
    {
        if (data_ && queue_) {
            sycl::free(data_, *queue_);
            data_ = nullptr;
        }
    }

    [[nodiscard]] std::expected<void, ice::Status> reallocate() noexcept
    {
        free_device_memory();
        if (dims_.empty() || !queue_) {
            return {};
        }
        return alloc_bytes(nbytes());
    }

    [[nodiscard]] std::expected<void, ice::Status> alloc_bytes(size_t bytes) noexcept
    {
        if (bytes == 0) {
            return {};
        }
        data_ = sycl::malloc_device(bytes, *queue_);
        if (!data_) {
            return std::unexpected(ice::Status::from_message("sycl::malloc_device OOM"));
        }
        return {};
    }
};

// ---------------------------------------------------------------------------
// XPU_AsyncTensorOps — implements ice::builder::TF_TensorOps via SYCL
// ---------------------------------------------------------------------------
class XPU_AsyncTensorOps : public TF_TensorOps
{
public:
    // Construct from an existing queue (borrowed; caller keeps queue alive)
    explicit XPU_AsyncTensorOps(sycl::queue& q) :
        impl_(q)
    {
    }

    // Default-constructible (queue set later via set_queue)
    XPU_AsyncTensorOps() = default;

    ~XPU_AsyncTensorOps() override = default;

    // Non-copyable; movable
    XPU_AsyncTensorOps(const XPU_AsyncTensorOps&) = delete;
    XPU_AsyncTensorOps& operator=(const XPU_AsyncTensorOps&) = delete;
    XPU_AsyncTensorOps(XPU_AsyncTensorOps&&) = default;
    XPU_AsyncTensorOps& operator=(XPU_AsyncTensorOps&&) = default;

    // ---- ice::builder::TF_TensorOps interface ----------------------------

    [[nodiscard]] std::expected<void, ice::Status> set_dtype(TFDataTypeEnum dt) noexcept override
    {
        return impl_.set_dtype(dt);
    }

    [[nodiscard]] std::expected<void, ice::Status>
    set_dims(const int64_t* dims, int n) noexcept override
    {
        return impl_.set_dims(dims, n);
    }

    [[nodiscard]] std::expected<void, ice::Status> set_byte_size(size_t len) noexcept override
    {
        return impl_.set_byte_size(len);
    }

    [[nodiscard]] std::expected<void, ice::Status> delete_tensor() noexcept override
    {
        return impl_.delete_data();
    }

    [[nodiscard]] std::expected<void, ice::Status>
    tensor_type(TFDataTypeEnum* out) noexcept override
    {
        *out = impl_.dtype();
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> num_dims(int* out) noexcept override
    {
        *out = impl_.ndim();
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> dim(int idx, int64_t* out) noexcept override
    {
        if (idx < 0 || idx >= impl_.ndim()) {
            return std::unexpected(ice::Status::from_message("dim index out of range"));
        }
        *out = impl_.dim(idx);
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    tensor_element_count(int64_t* out) noexcept override
    {
        *out = impl_.numel();
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> tensor_byte_size(size_t* out) noexcept override
    {
        *out = impl_.nbytes();
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> tensor_data(void** out) noexcept override
    {
        *out = impl_.data();
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    tensor_bitcast_from(TFDataTypeEnum dt, TF_Tensor** out) noexcept override
    {
        // Bitcast: wrap same device memory under a new dtype view.
        // Caller is responsible for ensuring size compatibility.
        auto* view = new XPU_AsyncTensorOps();
        view->impl_ = SyclTensorImpl(*impl_.queue()); // same queue
        // Reuse raw pointer without ownership transfer — shallow view.
        // In a full impl this would share a refcounted USM buffer.
        (void)dt;
        (void)out;
        delete view;
        return std::unexpected(
            ice::Status::from_message("bitcast not yet implemented in SYCL backend")
        );
    }

    [[nodiscard]] std::expected<void, ice::Status>
    tensor_bitcast_to(TFDataTypeEnum dt, TF_Tensor** out) noexcept override
    {
        return tensor_bitcast_from(dt, out);
    }

    [[nodiscard]] std::expected<void, ice::Status>
    tensor_copy(const ice::sonic::TF_TensorOps& dst_sonic) noexcept override
    {
        // Synchronous device-to-device copy via SYCL queue.
        (void)dst_sonic;
        // Full impl: extract plugin_data from dst_sonic, cast to XPU_AsyncTensorOps,
        // then call impl_.copy_from_device_async(src.impl_, bytes).wait()
        return std::unexpected(
            ice::Status::from_message("tensor_copy: cross-plugin dst not yet bridged")
        );
    }

    // ---- SYCL-specific async extensions ----------------------------------

    [[nodiscard]] AsyncResult copy_to_host_async(void* dst, size_t bytes) noexcept
    {
        return impl_.copy_to_host_async(dst, bytes);
    }

    [[nodiscard]] AsyncResult copy_from_host_async(const void* src, size_t bytes) noexcept
    {
        return impl_.copy_from_host_async(src, bytes);
    }

    [[nodiscard]] AsyncResult
    copy_from_device_async(const XPU_AsyncTensorOps& src, size_t bytes) noexcept
    {
        return impl_.copy_from_device_async(src.impl_, bytes);
    }

    [[nodiscard]] AsyncResult memset_async(int val, size_t bytes) noexcept
    {
        return impl_.memset_async(val, bytes);
    }

    [[nodiscard]] sycl::event record_barrier() noexcept
    {
        return impl_.record_barrier();
    }

    void synchronize()
    {
        impl_.synchronize();
    }

    [[nodiscard]] sycl::queue* queue() const noexcept
    {
        return impl_.queue();
    }

    // ---- factory ----------------------------------------------------------

    static XPU_AsyncTensorOps* make(sycl::queue& q)
    {
        return new XPU_AsyncTensorOps(q);
    }

private:
    SyclTensorImpl impl_;
};

// ---------------------------------------------------------------------------
// Async event helper (no c10 dependency)
// ---------------------------------------------------------------------------
class XPU_AsyncEvent
{
    sycl::event event_;

public:
    XPU_AsyncEvent() = default;

    explicit XPU_AsyncEvent(sycl::event e) :
        event_(std::move(e))
    {
    }

    void wait() const
    {
        event_.wait();
    }

    [[nodiscard]] bool is_ready() const
    {
        return event_.get_info<sycl::info::event::command_execution_status>() ==
               sycl::info::event_command_status::complete;
    }

    [[nodiscard]] static XPU_AsyncEvent record(sycl::queue& q)
    {
        return XPU_AsyncEvent(q.ext_oneapi_submit_barrier());
    }

    void wait_on(sycl::queue& q)
    {
        q.wait(event_);
    }

    template<typename F>
    void then_on(sycl::queue& q, F&& f)
    {
        q.submit(
            [&](sycl::handler& h)
            {
                h.depends_on(event_);
                h.single_task(std::forward<F>(f));
            }
        );
    }

    [[nodiscard]] sycl::event native() const noexcept
    {
        return event_;
    }
};

} // namespace ice::builder
