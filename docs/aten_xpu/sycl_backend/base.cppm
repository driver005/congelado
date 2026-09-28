// SYCL reference plugin — module aggregator. Not built by Bazel (docs/ only); this just gives
// plugin.cc (and anything else importing `sycl_backend`) one import that reaches every partition.

export module sycl_backend;

export import :device;
export import :platform;
export import :stream;
export import :event;
export import :timer;
export import :executor;
export import :mem_pool;
export import :allocator;
export import :memory;
export import :tensor;
export import :random_generator;
export import :device_graph;
export import :optimizer;
export import :engine_cache;
export import :kernels_matmul;
export import :kernels_conv;
export import :kernels_deconv;
export import :kernels_linear;
export import :kernels_sdpa;
export import :kernels_sdpa_backward;
export import :kernels_int8_conv;
export import :kernels_int8_matmul;
export import :kernels_woq_matmul;
export import :kernels_scaled_mm;
export import :kernels_rnn_lstm;
