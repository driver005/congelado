// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/intern/hive/hive.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/intern/hive/hive.h"

export module cc_ice_builder_hive:hive;

import std;

export namespace ice::builder {

class TF_HiveOps
{
public:
    static TF_HiveOps* create(void* ctx) noexcept
    {
        return static_cast<TF_HiveOps*>(ctx);
    }

    template<typename HandleT>
    static TF_HiveOps* create(HandleT* handle) noexcept
    {
        return static_cast<TF_HiveOps*>(handle->plugin_data);
    }

    virtual ~TF_HiveOps() = default;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    set_element_size(size_t element_size) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    insert(const void* value, TFHiveSlot* out_slot) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> erase(TFHiveSlot* slot) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get(const TFHiveSlot* slot, const void** out_value) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    for_each(TF_HiveVisitor visitor, void* capture) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> size(size_t* out_size) noexcept = 0;

    static TF_HiveOps* get_generic_vtable()
    {
        static TF_HiveOps vtable = {
            .struct_size = TF_HIVE_STRUCT_SIZE,
            .set_element_size =
                [](TF_Hive* hive, size_t element_size) noexcept
            {
                auto* self = TF_HiveOps::create(hive);
                auto res = self->set_element_size(element_size);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .insert =
                [](TF_Hive* hive,
                   const void* value,
                   TFHiveSlot* out_slot,
                   TF_Status* out_status) noexcept
            {
                auto* self = TF_HiveOps::create(hive);
                auto res = self->insert(value, out_slot);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .erase =
                [](TF_Hive* hive, TFHiveSlot* slot, TF_Status* out_status) noexcept
            {
                auto* self = TF_HiveOps::create(hive);
                auto res = self->erase(slot);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get =
                [](const TF_Hive* hive,
                   const TFHiveSlot* slot,
                   const void** out_value,
                   TF_Status* out_status) noexcept
            {
                auto* self = TF_HiveOps::create(hive);
                auto res = self->get(slot, out_value);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .for_each =
                [](const TF_Hive* hive, TF_HiveVisitor visitor, void* capture) noexcept
            {
                auto* self = TF_HiveOps::create(hive);
                auto res = self->for_each(visitor, capture);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .size =
                [](const TF_Hive* hive, size_t* out_size) noexcept
            {
                auto* self = TF_HiveOps::create(hive);
                auto res = self->size(out_size);
                if (!res) {
                    res.error().to_c(status);
                }
            },

            .destroy =
                [](void* plugin_context) noexcept
            {
                delete TF_HiveOps::create(plugin_context);
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
