// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/filesystem/read_only_memory_region.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/filesystem/read_only_memory_region.h"

export module cc_abi_builder_filesystem;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;

export namespace ice::builder {

class TF_ReadOnlyMemoryRegionOps
{
public:
    static TF_ReadOnlyMemoryRegionOps* create(void* ctx) noexcept
    {
        return static_cast<TF_ReadOnlyMemoryRegionOps*>(ctx);
    }

    template<typename HandleT>
    static TF_ReadOnlyMemoryRegionOps* create(HandleT* handle) noexcept
    {
        return static_cast<TF_ReadOnlyMemoryRegionOps*>(handle->plugin_data);
    }

    virtual ~TF_ReadOnlyMemoryRegionOps() = default;
    [[nodiscard]] std::expected<void, ice::Status> data(const void** out_data) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> length(uint64_t* out_length) noexcept = 0;
    virtual ice::String get_name() const noexcept = 0;

    static TF_ReadOnlyMemoryRegionOps* get_generic_vtable()
    {
        static TF_ReadOnlyMemoryRegionOps vtable = {
            .struct_size = TF_READONLYMEMORYREGION_STRUCT_SIZE,

            .destroy =
                [](void* plugin_context) noexcept
            {
                delete TF_ReadOnlyMemoryRegionOps::create(plugin_context);
            },

            .get_name =
                [](void* plugin_context, TF_String* out) noexcept
            {
                auto* self = TF_ReadOnlyMemoryRegionOps::create(plugin_context);
                auto result = self->get_name();
                result.to_c(out);
            },
            .data =
                [](TF_ReadOnlyMemoryRegion* region, const void** out_data) noexcept
            {
                auto* self = TF_ReadOnlyMemoryRegionOps::create(region);
                auto res = self->data(out_data);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .length =
                [](TF_ReadOnlyMemoryRegion* region, uint64_t* out_length) noexcept
            {
                auto* self = TF_ReadOnlyMemoryRegionOps::create(region);
                auto res = self->length(out_length);
                if (!res) {
                    res.error().to_c(status);
                }
            },

        };

        return &vtable;
    }
};

} // namespace ice::builder
