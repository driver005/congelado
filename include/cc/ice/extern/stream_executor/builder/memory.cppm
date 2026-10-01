// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/stream_executor/memory.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/stream_executor/memory.h"
#include "include/c/intern/tstring.h"

export module cc_ice_extern_stream_executor_builder:memory;

import std;
import cc_ice_intern_sonic;

export namespace ice::builder {

class TF_MemoryOps
{
public:
    explicit TF_MemoryOps(const ::TF_StringOps* String_ops) noexcept :
        m_handle{.plugin_data = this}
    {
        m_String_ops = String_ops;
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
    virtual void destroy() noexcept = 0;
    virtual void get_name(const ice::sonic::String& out_name) noexcept = 0;

    void get_generic_vtable(void (*create)(::TF_Memory*)) noexcept
    {
        m_vtable = ::TF_MemoryOps{
            .struct_size = TF_OFFSET_OF_END(::TF_MemoryOps, get_name),

            .create = create,
            .destroy =
                [](TF_Memory* handle) noexcept
            {
                auto& self = TF_MemoryOps::from_handle(handle);
                self.destroy();
            },
            .get_name =
                [](TF_Memory* memory, TF_String* out_name) noexcept
            {
                auto& self = TF_MemoryOps::from_handle(memory);
                self.get_name(self.wrap(std::type_identity<ice::sonic::String>{}, out_name));
            },

        };
    }

    ice::sonic::String
    wrap(std::type_identity<ice::sonic::String>, const ::TF_String* handle) const noexcept
    {
        return ice::sonic::String{m_String_ops, const_cast<::TF_String*>(handle)};
    }

    const ::TF_MemoryOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const ::TF_Memory& get_handle() const noexcept
    {
        return m_handle;
    }

    template<typename Registry, typename StringType>
    void register_ops(
        Registry& registry,
        const StringType& type,
        const StringType& provider
    ) const noexcept
    {
        registry.register_op(type, provider, const_cast<::TF_MemoryOps*>(&m_vtable));
    }

private:
    ::TF_MemoryOps m_vtable;
    ::TF_Memory m_handle;

    const ::TF_StringOps* m_String_ops{nullptr};
};

} // namespace ice::builder
