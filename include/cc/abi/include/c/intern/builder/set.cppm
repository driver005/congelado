// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/intern/set.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/intern/set.h"

export module cc_abi_builder_intern;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;

export namespace ice::builder {

class TF_SetOps
{
public:
    static TF_SetOps* create(void* ctx) noexcept
    {
        return static_cast<TF_SetOps*>(ctx);
    }

    template<typename HandleT>
    static TF_SetOps* create(HandleT* handle) noexcept
    {
        return static_cast<TF_SetOps*>(handle->plugin_data);
    }

    virtual ~TF_SetOps() = default;
    [[nodiscard]] std::expected<void, ice::Status> insert(const void* key) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    find(const void* key, const void** out_value) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> erase(const void* key) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    contains(const void* key, int* out_found) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> size(size_t* out_size) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    for_each(TF_SetVisitor visitor, void* capture) noexcept = 0;
    virtual ice::String get_name() const noexcept = 0;

    static TF_SetOps* get_generic_vtable()
    {
        static TF_SetOps vtable = {
            .struct_size = TF_SET_STRUCT_SIZE,
            .insert =
                [](TF_Set* set, const void* key, TF_Status* out_status) noexcept
            {
                auto* self = TF_SetOps::create(set);
                auto res = self->insert(key);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .find =
                [](const TF_Set* set,
                   const void* key,
                   const void** out_value,
                   TF_Status* out_status) noexcept
            {
                auto* self = TF_SetOps::create(set);
                auto res = self->find(key, out_value);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .erase =
                [](TF_Set* set, const void* key, TF_Status* out_status) noexcept
            {
                auto* self = TF_SetOps::create(set);
                auto res = self->erase(key);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .contains =
                [](const TF_Set* set,
                   const void* key,
                   int* out_found,
                   TF_Status* out_status) noexcept
            {
                auto* self = TF_SetOps::create(set);
                auto res = self->contains(key, out_found);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .size =
                [](const TF_Set* set, size_t* out_size) noexcept
            {
                auto* self = TF_SetOps::create(set);
                auto res = self->size(out_size);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .for_each =
                [](const TF_Set* set, TF_SetVisitor visitor, void* capture) noexcept
            {
                auto* self = TF_SetOps::create(set);
                auto res = self->for_each(visitor, capture);
                if (!res) {
                    res.error().to_c(status);
                }
            },

            .destroy =
                [](void* plugin_context) noexcept
            {
                delete TF_SetOps::create(plugin_context);
            },

        };

        return &vtable;
    }
};

} // namespace ice::builder
