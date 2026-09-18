// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/intern/map.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/intern/map.h"

export module cc_ice_builder_intern:map;

import std;

export namespace ice::builder {

class TF_MapOps
{
public:
    static TF_MapOps* create(void* ctx) noexcept
    {
        return static_cast<TF_MapOps*>(ctx);
    }

    template<typename HandleT>
    static TF_MapOps* create(HandleT* handle) noexcept
    {
        return static_cast<TF_MapOps*>(handle->plugin_data);
    }

    virtual ~TF_MapOps() = default;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    insert(const void* key, const void* value) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    find(const void* key, const void** out_value) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> erase(const void* key) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    contains(const void* key, int* out_found) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> size(size_t* out_size) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    for_each(TF_MapVisitor visitor, void* capture) noexcept = 0;

    static TF_MapOps* get_generic_vtable()
    {
        static TF_MapOps vtable = {
            .struct_size = TF_MAP_STRUCT_SIZE,
            .insert =
                [](TF_Map* map, const void* key, const void* value, TF_Status* out_status) noexcept
            {
                auto* self = TF_MapOps::create(map);
                auto res = self->insert(key, value);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .find =
                [](const TF_Map* map,
                   const void* key,
                   const void** out_value,
                   TF_Status* out_status) noexcept
            {
                auto* self = TF_MapOps::create(map);
                auto res = self->find(key, out_value);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .erase =
                [](TF_Map* map, const void* key, TF_Status* out_status) noexcept
            {
                auto* self = TF_MapOps::create(map);
                auto res = self->erase(key);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .contains =
                [](TF_Map* map, const void* key, int* out_found, TF_Status* out_status) noexcept
            {
                auto* self = TF_MapOps::create(map);
                auto res = self->contains(key, out_found);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .size =
                [](const TF_Map* map, size_t* out_size) noexcept
            {
                auto* self = TF_MapOps::create(map);
                auto res = self->size(out_size);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .for_each =
                [](const TF_Map* map, TF_MapVisitor visitor, void* capture) noexcept
            {
                auto* self = TF_MapOps::create(map);
                auto res = self->for_each(visitor, capture);
                if (!res) {
                    res.error().to_c(status);
                }
            },

            .destroy =
                [](void* plugin_context) noexcept
            {
                delete TF_MapOps::create(plugin_context);
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
