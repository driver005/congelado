// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/store/index.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/store/index.h"

export module cc_abi_builder_store;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;

export namespace ice::builder {

class TFStoreIndexOps
{
public:
    static TFStoreIndexOps* create(void* ctx) noexcept
    {
        return static_cast<TFStoreIndexOps*>(ctx);
    }

    template<typename HandleT>
    static TFStoreIndexOps* create(HandleT* handle) noexcept
    {
        return static_cast<TFStoreIndexOps*>(handle->plugin_data);
    }

    virtual ~TFStoreIndexOps() = default;
    [[nodiscard]] std::expected<void, ice::Status> create(
        const ice::sonic::TF_StringOps& name,
        const ice::sonic::TF_MapOps& field_config
    ) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    drop(const ice::sonic::TF_StringOps& name) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    list(const ice::sonic::TF_VectorOps& out_names) noexcept = 0;
    virtual ice::String get_name() const noexcept = 0;

    static TFStoreIndexOps* get_generic_vtable()
    {
        static TFStoreIndexOps vtable = {
            .struct_size = TF_TOREINDEX_STRUCT_SIZE,

            .destroy =
                [](void* plugin_context) noexcept
            {
                delete TFStoreIndexOps::create(plugin_context);
            },
            .create =
                [](TFStoreIndex* index,
                   const TF_String* name,
                   const TF_Map* field_config,
                   TF_Status* out_status) noexcept
            {
                auto* self = TFStoreIndexOps::create(index);
                auto res = self->create(
                    ice::sonic::TF_StringOps::wrap(name),
                    ice::sonic::TF_MapOps::wrap(field_config)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .drop =
                [](TFStoreIndex* index, const TF_String* name, TF_Status* out_status) noexcept
            {
                auto* self = TFStoreIndexOps::create(index);
                auto res = self->drop(ice::sonic::TF_StringOps::wrap(name));
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .list =
                [](TFStoreIndex* index, TF_Vector* out_names, TF_Status* out_status) noexcept
            {
                auto* self = TFStoreIndexOps::create(index);
                auto res = self->list(ice::sonic::TF_VectorOps::wrap(out_names));
                if (!res) {
                    res.error().to_c(out_status);
                }
            },

        };

        return &vtable;
    }
};

} // namespace ice::builder
