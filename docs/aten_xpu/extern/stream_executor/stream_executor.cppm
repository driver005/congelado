module;

#include "include/c/extern/stream_executor/stream_executor.h"

export module aten_xpu_extern_stream_executor:stream_executor;

import std;
import cc_ice_intern_sonic;
import cc_ice_extern_stream_executor_builder;
import aten_xpu_intern;

export namespace aten_xpu {

class SyclStreamExecutor : public ice::builder::TF_StreamExecutorOps
{
public:
    explicit SyclStreamExecutor(const SyclOpsTable& ops) noexcept :
        ice::builder::TF_StreamExecutorOps{ops.getStringOps()},
        m_status{ops}
    {
    }

    ~SyclStreamExecutor() override = default;
    SyclStreamExecutor(const SyclStreamExecutor&) = delete;
    SyclStreamExecutor& operator=(const SyclStreamExecutor&) = delete;
    SyclStreamExecutor(SyclStreamExecutor&&) = delete;
    SyclStreamExecutor& operator=(SyclStreamExecutor&&) = delete;

    static void create(::TF_StreamExecutor* handle)
    {
        auto* stream_executor = new SyclStreamExecutor{SyclOpsTable::getInstance()};
        SyclHandle::attach(handle, *stream_executor);
    }

    void destroy() noexcept override
    {
        delete this;
    }

    void get_name(const ice::sonic::String& out_name) noexcept override
    {
        m_status.copy_into(out_name, "XPU");
    }

private:
    SyclStatus m_status;
};

} // namespace aten_xpu
