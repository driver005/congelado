module;

#include <oneapi/dnnl/dnnl.hpp>
#include <oneapi/dnnl/dnnl_sycl.hpp>
#include <sycl/sycl.hpp>

export module aten_xpu_extern_kernel:onednn_primitive_executor;

import std;
import :onednn_engine_cache;

export namespace aten_xpu {

class SyclPrimitiveExecutor
{
public:
    explicit SyclPrimitiveExecutor(sycl::queue& queue) :
        m_queue{queue},
        m_engine{SyclEngineCache::getInstance().getEngine(queue)},
        m_stream{SyclEngineCache::getInstance().getStream(queue)}
    {
    }

    void addArgument(int argument, const dnnl::memory::desc& desc, const void* data)
    {

        m_arguments.insert_or_assign(argument, dnnl::memory{desc, m_engine, const_cast<void*>(data)});

    }

    dnnl::engine& getEngine() const noexcept { return m_engine; }

    std::unordered_map<int, dnnl::memory>& getArguments() noexcept { return m_arguments; }

    template<typename Primitive, typename PrimitiveDesc>
    sycl::event execute(const Primitive& primitive, const PrimitiveDesc& primitive_desc)
    {

        const auto scratchpad_desc = primitive_desc.scratchpad_desc();
        if (scratchpad_desc.get_size() > 0) {
            m_arguments.insert_or_assign(
                DNNL_ARG_SCRATCHPAD,
                dnnl::memory{scratchpad_desc, m_engine, acquire_scratchpad(scratchpad_desc.get_size())}
            );
        }
        return dnnl::sycl_interop::execute(primitive, m_stream, m_arguments);

    }

    static dnnl::primitive_attr user_scratchpad_attributes()
    {

        dnnl::primitive_attr attributes;
        attributes.set_scratchpad_mode(dnnl::scratchpad_mode::user);
        return attributes;

    }

private:
    void* acquire_scratchpad(std::size_t size)
    {

        auto& [pointer, capacity] = s_scratchpads[&m_queue];
        if (capacity < size) {
            if (pointer != nullptr) {
                m_queue.wait();
                sycl::free(pointer, m_queue);
            }
            pointer = sycl::malloc_device(size, m_queue);
            capacity = size;
        }
        return pointer;

    }

    sycl::queue& m_queue;
    dnnl::engine& m_engine;
    dnnl::stream& m_stream;
    std::unordered_map<int, dnnl::memory> m_arguments;

    static thread_local std::map<const sycl::queue*, std::pair<void*, std::size_t>> s_scratchpads;
};

inline thread_local std::map<const sycl::queue*, std::pair<void*, std::size_t>>
    SyclPrimitiveExecutor::s_scratchpads;

} // namespace aten_xpu
