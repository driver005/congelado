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
    TF_MemoryOps() noexcept :
        m_handle{.plugin_data = this}
    {
    }

    TF_MemoryOps(const TF_MemoryOps&) = delete;
    TF_MemoryOps& operator=(const TF_MemoryOps&) = delete;

    static TF_MemoryOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TF_MemoryOps*>(ctx);
    }

    template<typename HandleT>
    static TF_MemoryOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TF_MemoryOps*>(handle->plugin_data);
    }

    virtual ~TF_MemoryOps() = default;

    void get_generic_vtable() noexcept
    {
        m_vtable = ::TF_MemoryOps{
            .struct_size = TF_MEMORY_STRUCT_SIZE,

            .destroy =
                [](void* plugin_context) noexcept
            {
                std::unique_ptr<TF_MemoryOps>{&TF_MemoryOps::from_handle(plugin_context)};
            },

            .get_name =
                [](void* plugin_context, TF_String* out) noexcept
            {
                auto result = TF_MemoryOps::from_handle(plugin_context).get_name();
                result.to_c(out);
            },

        };
    }

    const ::TF_MemoryOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    virtual builder::String get_name() const noexcept = 0;

    const TF_Memory& get_handle() const noexcept
    {
        return m_handle;
    }


private:
    ::TF_MemoryOps m_vtable;
    TF_Memory m_handle;
};

} // namespace ice::builder
