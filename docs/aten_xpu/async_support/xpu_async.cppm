// Unified XPU Async API for ice::builder
// Single header to include for all async XPU operations

module;

export module cc_ice_builder_intern:xpu_async;

import cc_ice_builder_intern:xpu_async_tensor;
import cc_ice_builder_intern:xpu_async_buffer;
import cc_ice_builder_intern:xpu_async_kernels;

export namespace ice::builder {

// Convenience aliases
using XPUAsyncTensor = XPU_AsyncTensorOps;
using XPUAsyncBuffer = XPU_AsyncBuffer;
using XPUPooledAsyncBuffer = XPU_PooledAsyncBuffer;
using XPUAsyncEvent = XPU_AsyncEvent;
using XPUAsyncPipeline = XPU_AsyncPipeline;

using XPUAsyncAttention = XPU_AsyncAttention;
using XPUAsyncConv = XPU_AsyncConv;
using XPUAsyncLinear = XPU_AsyncLinear;
using XPUAsyncRNN = XPU_AsyncRNN;

using AttentionResult = XPU_AsyncAttention::AttentionResult;
using AttentionBackwardResult = XPU_AsyncAttention::AttentionBackwardResult;
using ConvResult = XPU_AsyncConv::ConvResult;
using LinearResult = XPU_AsyncLinear::LinearResult;
using RNNResult = XPU_AsyncRNN::RNNResult;

// Async result base
using AsyncResult = XPU_AsyncTensorOps::AsyncResult;

// Factory functions
[[nodiscard]] inline XPUAsyncTensor make_xpu_tensor(
    const TensorOptions& options = {},
    c10::xpu::XPUStream stream = c10::xpu::getCurrentXPUStream()) {
    return XPUAsyncTensor(options, stream);
}

[[nodiscard]] inline XPUAsyncTensor make_xpu_tensor(
    at::Tensor tensor,
    c10::xpu::XPUStream stream = c10::xpu::getCurrentXPUStream()) {
    return XPUAsyncTensor(std::move(tensor), stream);
}

[[nodiscard]] inline XPUAsyncBuffer make_xpu_buffer(
    size_t bytes,
    c10::xpu::XPUStream stream = c10::xpu::getCurrentXPUStream()) {
    return XPUAsyncBuffer(bytes, stream);
}

[[nodiscard]] inline XPUPooledAsyncBuffer make_xpu_pooled_buffer(
    size_t bytes,
    c10::MempoolId_t pool_id,
    c10::xpu::XPUStream stream = c10::xpu::getCurrentXPUStream()) {
    return XPUPooledAsyncBuffer(bytes, pool_id, stream);
}

[[nodiscard]] inline XPUAsyncAttention make_xpu_attention(
    c10::xpu::XPUStream stream = c10::xpu::getCurrentXPUStream()) {
    return XPUAsyncAttention(stream);
}

[[nodiscard]] inline XPUAsyncConv make_xpu_conv(
    c10::xpu::XPUStream stream = c10::xpu::getCurrentXPUStream()) {
    return XPUAsyncConv(stream);
}

[[nodiscard]] inline XPUAsyncLinear make_xpu_linear(
    c10::xpu::XPUStream stream = c10::xpu::getCurrentXPUStream()) {
    return XPUAsyncLinear(stream);
}

[[nodiscard]] inline XPUAsyncRNN make_xpu_rnn(
    c10::xpu::XPUStream stream = c10::xpu::getCurrentXPUStream()) {
    return XPUAsyncRNN(stream);
}

[[nodiscard]] inline XPUAsyncPipeline make_xpu_pipeline(
    c10::xpu::XPUStream stream = c10::xpu::getCurrentXPUStream()) {
    return XPUAsyncPipeline(stream);
}

// Stream management
inline c10::xpu::XPUStream get_current_stream() noexcept {
    return c10::xpu::getCurrentXPUStream();
}

inline void set_current_stream(c10::xpu::XPUStream stream) noexcept {
    c10::xpu::set_stream(stream);
}

inline c10::xpu::XPUStream create_stream(c10::DeviceIndex device = -1) {
    return c10::xpu::XPUStream(device >= 0 ? device : c10::xpu::current_device());
}

inline void synchronize_stream(c10::xpu::XPUStream stream = c10::xpu::getCurrentXPUStream()) {
    c10::xpu::getRawStream(stream)->wait_and_throw();
}

// Device management
inline c10::DeviceIndex device_count() noexcept {
    return c10::xpu::device_count();
}

inline c10::DeviceIndex current_device() noexcept {
    return c10::xpu::current_device();
}

inline void set_device(c10::DeviceIndex device) noexcept {
    c10::xpu::set_device(device);
}

inline void device_synchronize(c10::DeviceIndex device = -1) {
    c10::xpu::device_synchronize(device);
}

// Memory management
inline void empty_cache(c10::MempoolId_t mempool_id = {0, 0}) {
    c10::xpu::XPUCachingAllocator::emptyCache(mempool_id);
}

inline c10::xpu::XPUCachingAllocator::DeviceStats get_device_stats(c10::DeviceIndex device) {
    return c10::xpu::XPUCachingAllocator::getDeviceStats(device);
}

inline void reset_peak_stats(c10::DeviceIndex device) {
    c10::xpu::XPUCachingAllocator::resetPeakStats(device);
}

// Async event recording
inline sycl::event record_event(c10::xpu::XPUStream stream = c10::xpu::getCurrentXPUStream()) {
    return c10::xpu::getRawStream(stream)->ext_oneapi_submit_barrier();
}

inline void wait_event(const sycl::event& event, c10::xpu::XPUStream stream = c10::xpu::getCurrentXPUStream()) {
    c10::xpu::getRawStream(stream)->wait(event);
}

// Type-safe dtype conversion
inline c10::ScalarType to_scalar_type(TFDataTypeEnum dt) {
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

inline TFDataTypeEnum from_scalar_type(c10::ScalarType t) {
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
        default: return TF_FLOAT;
    }
}

} // namespace ice::builder