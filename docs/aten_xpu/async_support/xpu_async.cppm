// SYCL Backend — unified ice layer entry point
// Single module to import for all SYCL-backed ice::builder operations.
// Replaces the old ATen/c10-dependent xpu_async.cppm facade.
// NO ATen / c10 / PyTorch dependencies.

module;

export module cc_ice_sycl_backend;

// Re-export all sub-partitions of this module.
export import cc_ice_sycl_backend:tensor;
export import cc_ice_sycl_backend:buffer;
export import cc_ice_sycl_backend:executor;

// ---------------------------------------------------------------------------
// Convenience aliases (mirrors the old xpu_async.cppm public surface)
// ---------------------------------------------------------------------------
export namespace ice::builder {

// Tensor backend
using SyclTensor      = XPU_AsyncTensorOps;
using SyclAsyncEvent  = XPU_AsyncEvent;
using SyclAsyncResult = AsyncResult;

// Buffer backends
using SyclBuffer       = XPU_AsyncBuffer;
using SyclPooledBuffer = XPU_PooledAsyncBuffer;

// Executor backend
using SyclExecutor = SyclExecutorImpl;

} // namespace ice::builder