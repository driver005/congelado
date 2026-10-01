// SYCL reference plugin — per-device dnnl::engine / per-(device, TF_Stream*) dnnl::stream cache.
//
// Not built by Bazel (docs/ only). Replaces mkldnn/detail/oneDNNContext.h's GpuEngineManager and
// GpuStreamManager singletons. Kept as the same kind of Meyer's singleton the original two
// classes already are — this is oneDNN-object-construction plumbing (constructing a dnnl::engine
// is not cheap), not ATen device/stream state, so it does not conflict with the rest of this
// plugin's "no globals" rule: every other file's state lives in an object the host creates and
// destroys through a vtable slot, and this cache does not shadow any of that — it is keyed by
// (device index, TF_Stream*) and never outlives what those already mean.
//
// Deliberately NOT thread_local, matching this plugin's own convention: it is guarded by a
// mutex instead, and dnnl::stream construction depends only on the queue, not on the calling
// thread.

module;

#include "include/c/extern/kernel/context.h"

#include <oneapi/dnnl/dnnl.hpp>
#include <oneapi/dnnl/dnnl_sycl.hpp>

export module sycl_backend:engine_cache;

import std;

export namespace sycl_backend {

class EngineCache
{
public:
    static EngineCache& instance()
    {
        static EngineCache cache;
        return cache;
    }

    EngineCache(const EngineCache&) = delete;
    EngineCache& operator=(const EngineCache&) = delete;
    EngineCache(EngineCache&&) = delete;
    EngineCache& operator=(EngineCache&&) = delete;

    dnnl::engine& get_engine(const sycl::device& device, const sycl::context& context)
    {
        std::lock_guard<std::mutex> lock{m_mutex};

        auto found = m_engines.find(&device);
        if (found != m_engines.end()) {
            return *found->second;
        }

        auto engine =
            std::make_shared<dnnl::engine>(dnnl::sycl_interop::make_engine(device, context));
        return *m_engines.emplace(&device, std::move(engine)).first->second;
    }

    dnnl::stream& get_stream(dnnl::engine& engine, sycl::queue& queue)
    {
        std::lock_guard<std::mutex> lock{m_mutex};

        auto found = m_streams.find(&queue);
        if (found != m_streams.end()) {
            return *found->second;
        }

        auto stream =
            std::make_shared<dnnl::stream>(dnnl::sycl_interop::make_stream(engine, queue));
        return *m_streams.emplace(&queue, std::move(stream)).first->second;
    }

private:
    EngineCache() = default;
    ~EngineCache() = default;

    std::mutex m_mutex;
    std::map<const sycl::device*, std::shared_ptr<dnnl::engine>> m_engines;
    std::map<const sycl::queue*, std::shared_ptr<dnnl::stream>> m_streams;
};

} // namespace sycl_backend
