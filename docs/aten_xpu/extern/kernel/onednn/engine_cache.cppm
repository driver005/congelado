module;

#include <oneapi/dnnl/dnnl.hpp>
#include <oneapi/dnnl/dnnl_sycl.hpp>
#include <sycl/sycl.hpp>

export module aten_xpu_extern_kernel:onednn_engine_cache;

import std;

export namespace aten_xpu {

class SyclEngineCache
{
public:
    SyclEngineCache(const SyclEngineCache&) = delete;
    SyclEngineCache& operator=(const SyclEngineCache&) = delete;
    SyclEngineCache(SyclEngineCache&&) = delete;
    SyclEngineCache& operator=(SyclEngineCache&&) = delete;

    static SyclEngineCache& getInstance()
    {

        static SyclEngineCache cache;
        return cache;

    }

    dnnl::engine& getEngine(const sycl::queue& queue)
    {

        const std::scoped_lock lock{m_mutex};
        const auto device = queue.get_device();
        for (auto& [candidate, engine]: m_engines) {
            if (candidate == device) {
                return engine;
            }
        }
        return m_engines.emplace_back(device, dnnl::sycl_interop::make_engine(device, queue.get_context())).second;

    }

    dnnl::stream& getStream(sycl::queue& queue)
    {

        auto& engine = getEngine(queue);
        const std::scoped_lock lock{m_mutex};
        auto found = m_streams.find(&queue);
        if (found == m_streams.end()) {
            found = m_streams.emplace(&queue, dnnl::sycl_interop::make_stream(engine, queue)).first;
        }
        return found->second;

    }

private:
    SyclEngineCache() = default;
    ~SyclEngineCache() = default;

    std::mutex m_mutex;
    std::deque<std::pair<sycl::device, dnnl::engine>> m_engines;
    std::map<const sycl::queue*, dnnl::stream> m_streams;
};

} // namespace aten_xpu
