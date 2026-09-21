// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/stream_executor/stream_executor.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/stream_executor/stream_executor.h"

export module cc_ice_builder_stream_executor:stream_executor;

import std;

export namespace ice::builder {

class TF_StreamExecutorOps
{
public:
    static TF_StreamExecutorOps* create(void* ctx) noexcept
    {
        return static_cast<TF_StreamExecutorOps*>(ctx);
    }

    template<typename HandleT>
    static TF_StreamExecutorOps* create(HandleT* handle) noexcept
    {
        return static_cast<TF_StreamExecutorOps*>(handle->plugin_data);
    }

    virtual ~TF_StreamExecutorOps() = default;

    static TF_StreamExecutorOps* get_generic_vtable()
    {
        static TF_StreamExecutorOps vtable = {
            .struct_size = TF_STREAMEXECUTOR_STRUCT_SIZE,

            .destroy =
                [](void* plugin_context) noexcept
            {
                delete TF_StreamExecutorOps::create(plugin_context);
            },

            .get_name =
                [](void* plugin_context, TF_String* out) noexcept
            {
                auto* self = TF_StreamExecutorOps::create(plugin_context);
                auto result = self->get_name();
                result.to_c(out);
            },

        };

        return &vtable;
    }

    builder::String get_name() const noexcept
    {
        builder::String result;
        m_ops->get_name(get_handle(), result.get_handle());
        return result;
    }
};

} // namespace ice::builder
