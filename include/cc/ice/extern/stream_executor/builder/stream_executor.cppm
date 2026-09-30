// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/stream_executor/stream_executor.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/stream_executor/stream_executor.h"

export module cc_ice_extern_stream_executor_builder:stream_executor;

import std;

export namespace ice::builder {

class TF_StreamExecutorOps
{
public:
    TF_StreamExecutorOps() noexcept :
        m_handle{.plugin_data = this}
    {
    }

    TF_StreamExecutorOps(const TF_StreamExecutorOps&) = delete;
    TF_StreamExecutorOps& operator=(const TF_StreamExecutorOps&) = delete;

    static TF_StreamExecutorOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TF_StreamExecutorOps*>(ctx);
    }

    template<typename HandleT>
    static TF_StreamExecutorOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TF_StreamExecutorOps*>(handle->plugin_data);
    }

    virtual ~TF_StreamExecutorOps() = default;

    void get_generic_vtable() noexcept
    {
        m_vtable = ::TF_StreamExecutorOps{
            .struct_size = TF_STREAMEXECUTOR_STRUCT_SIZE,

            .destroy =
                [](void* plugin_context) noexcept
            {
                std::unique_ptr<TF_StreamExecutorOps>{
                    &TF_StreamExecutorOps::from_handle(plugin_context)
                };
            },

            .get_name =
                [](void* plugin_context, TF_String* out) noexcept
            {
                auto result = TF_StreamExecutorOps::from_handle(plugin_context).get_name();
                result.to_c(out);
            },

        };
    }

    const ::TF_StreamExecutorOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    virtual builder::String get_name() const noexcept = 0;

    const TF_StreamExecutor& get_handle() const noexcept
    {
        return m_handle;
    }


private:
    ::TF_StreamExecutorOps m_vtable;
    TF_StreamExecutor m_handle;
};

} // namespace ice::builder
