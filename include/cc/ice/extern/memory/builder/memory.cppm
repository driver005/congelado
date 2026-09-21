// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/memory/memory.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/memory/memory.h"

export module cc_ice_extern_memory_builder:memory;

import std;

export namespace ice::builder {

class TF_MemoryOps
{
public:
    static TF_MemoryOps* create(void* ctx) noexcept
    {
        return static_cast<TF_MemoryOps*>(ctx);
    }

    template<typename HandleT>
    static TF_MemoryOps* create(HandleT* handle) noexcept
    {
        return static_cast<TF_MemoryOps*>(handle->plugin_data);
    }

    virtual ~TF_MemoryOps() = default;

    static TF_MemoryOps* get_generic_vtable()
    {
        static TF_MemoryOps vtable = {
            .struct_size = TF_MEMORY_STRUCT_SIZE,

            .destroy =
                [](void* plugin_context) noexcept
            {
                delete TF_MemoryOps::create(plugin_context);
            },

            .get_name =
                [](void* plugin_context, TF_String* out) noexcept
            {
                auto* self = TF_MemoryOps::create(plugin_context);
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
