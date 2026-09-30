// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/filesystem/read_only_memory_region.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/filesystem/read_only_memory_region.h"

export module cc_ice_extern_filesystem_builder:read_only_memory_region;

import std;

export namespace ice::builder {

class TF_ReadOnlyMemoryRegionOps
{
public:
    TF_ReadOnlyMemoryRegionOps() noexcept :
        m_handle{.plugin_data = this}
    {
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

    void get_generic_vtable() noexcept
    {
        m_vtable = ::TF_ReadOnlyMemoryRegionOps{
            .struct_size = TF_READONLYMEMORYREGION_STRUCT_SIZE,
            .destroy =
                [](TF_ReadOnlyMemoryRegion* region) noexcept
            {
                TF_ReadOnlyMemoryRegionOps::from_handle(region).destroy();
            },
            .get_name =
                [](TF_ReadOnlyMemoryRegion* region, TF_String* out_name) noexcept
            {
                TF_ReadOnlyMemoryRegionOps::from_handle(region).get_name(
                    ice::sonic::String::wrap(out_name)
                );
            },
            .data =
                [](TF_ReadOnlyMemoryRegion* region, const void** out_data) noexcept
            {
                TF_ReadOnlyMemoryRegionOps::from_handle(region).data(out_data);
            },
            .length =
                [](TF_ReadOnlyMemoryRegion* region, uint64_t* out_length) noexcept
            {
                TF_ReadOnlyMemoryRegionOps::from_handle(region).length(out_length);
            },

        };
    }

    const ::TF_ReadOnlyMemoryRegionOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const TF_ReadOnlyMemoryRegion& get_handle() const noexcept
    {
        return m_handle;
    }


private:
    ::TF_ReadOnlyMemoryRegionOps m_vtable;
    TF_ReadOnlyMemoryRegion m_handle;
};

} // namespace ice::builder
