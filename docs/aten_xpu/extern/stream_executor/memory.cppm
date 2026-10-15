module;

#include "include/c/extern/stream_executor/memory.h"

export module aten_xpu_extern_stream_executor:memory;

import std;
import cc_ice_intern_sonic;
import cc_ice_extern_stream_executor_builder;
import aten_xpu_intern;

export namespace aten_xpu {

class SyclMemory : public ice::builder::Memory
{
public:
    explicit SyclMemory(const SyclOpsTable& ops) noexcept :
        ice::builder::Memory{ops.getStringOps()},
        m_status{ops}
    {
    }

    ~SyclMemory() override = default;
    SyclMemory(const SyclMemory&) = delete;
    SyclMemory& operator=(const SyclMemory&) = delete;
    SyclMemory(SyclMemory&&) = delete;
    SyclMemory& operator=(SyclMemory&&) = delete;

    void destroy() noexcept override
    {
        delete this;
    }

    void get_name(const ice::sonic::String& out_name) noexcept override
    {
        m_status.copy_into(out_name, "xpu_caching_allocator");
    }

private:
    SyclStatus m_status;
};

} // namespace aten_xpu
