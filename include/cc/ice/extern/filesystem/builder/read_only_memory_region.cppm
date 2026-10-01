// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/filesystem/read_only_memory_region.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/filesystem/read_only_memory_region.h"
#include "include/c/intern/tstring.h"

export module cc_ice_extern_filesystem_builder:read_only_memory_region;

import std;
import cc_ice_intern_sonic;

export namespace ice::builder {

class TF_ReadOnlyMemoryRegionOps
{
public:
    explicit TF_ReadOnlyMemoryRegionOps(const ::TF_StringOps* String_ops) noexcept :
        m_handle{.plugin_data = this}
    {
        m_String_ops = String_ops;
    }

    TF_ReadOnlyMemoryRegionOps(const TF_ReadOnlyMemoryRegionOps&) = delete;
    TF_ReadOnlyMemoryRegionOps& operator=(const TF_ReadOnlyMemoryRegionOps&) = delete;

    static TF_ReadOnlyMemoryRegionOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TF_ReadOnlyMemoryRegionOps*>(ctx);
    }

    template<typename HandleT>
    static TF_ReadOnlyMemoryRegionOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TF_ReadOnlyMemoryRegionOps*>(handle->plugin_data);
    }

    virtual ~TF_ReadOnlyMemoryRegionOps() = default;
    virtual void destroy() noexcept = 0;
    virtual void get_name(const ice::sonic::String& out_name) noexcept = 0;
    virtual void data(const void** out_data) noexcept = 0;
    virtual void length(uint64_t* out_length) noexcept = 0;

    void get_generic_vtable(void (*create)(::TF_ReadOnlyMemoryRegion*)) noexcept
    {
        m_vtable = ::TF_ReadOnlyMemoryRegionOps{
            .struct_size = TF_OFFSET_OF_END(::TF_ReadOnlyMemoryRegionOps, length),

            .create = create,
            .destroy =
                [](TF_ReadOnlyMemoryRegion* handle) noexcept
            {
                auto& self = TF_ReadOnlyMemoryRegionOps::from_handle(handle);
                self.destroy();
            },
            .get_name =
                [](TF_ReadOnlyMemoryRegion* region, TF_String* out_name) noexcept
            {
                auto& self = TF_ReadOnlyMemoryRegionOps::from_handle(region);
                self.get_name(self.wrap(std::type_identity<ice::sonic::String>{}, out_name));
            },
            .data =
                [](TF_ReadOnlyMemoryRegion* region, const void** out_data) noexcept
            {
                auto& self = TF_ReadOnlyMemoryRegionOps::from_handle(region);
                self.data(out_data);
            },
            .length =
                [](TF_ReadOnlyMemoryRegion* region, uint64_t* out_length) noexcept
            {
                auto& self = TF_ReadOnlyMemoryRegionOps::from_handle(region);
                self.length(out_length);
            },

        };
    }

    ice::sonic::String
    wrap(std::type_identity<ice::sonic::String>, const ::TF_String* handle) const noexcept
    {
        return ice::sonic::String{m_String_ops, const_cast<::TF_String*>(handle)};
    }

    const ::TF_ReadOnlyMemoryRegionOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const ::TF_ReadOnlyMemoryRegion& get_handle() const noexcept
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
        registry.register_op(type, provider, const_cast<::TF_ReadOnlyMemoryRegionOps*>(&m_vtable));
    }

private:
    ::TF_ReadOnlyMemoryRegionOps m_vtable;
    ::TF_ReadOnlyMemoryRegion m_handle;

    const ::TF_StringOps* m_String_ops{nullptr};
};

} // namespace ice::builder
