module;

#include <oneapi/dnnl/dnnl.hpp>
#include <sycl/sycl.hpp>

export module aten_xpu_extern_kernel:onednn_runtime;

import std;
import :onednn_engine_cache;
import :onednn_primitive_executor;

export namespace aten_xpu {

class SyclOnednnRuntime
{
public:
    SyclEngineCache& getEngines() noexcept
    {
        return m_engines;
    }

    SyclPrimitiveExecutor make_executor(sycl::queue& queue)
    {
        return SyclPrimitiveExecutor{queue, m_engines};
    }

    dnnl::primitive_attr user_scratchpad_attributes() const
    {
        dnnl::primitive_attr attributes;
        attributes.set_scratchpad_mode(dnnl::scratchpad_mode::user);
        return attributes;
    }

private:
    SyclEngineCache m_engines;
};

} // namespace aten_xpu
