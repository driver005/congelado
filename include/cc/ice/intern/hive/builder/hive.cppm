// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/intern/hive/hive.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/intern/hive/hive.h"

export module cc_ice_intern_hive_builder:hive;

import std;

export namespace ice::builder {

class TF_HiveOps
{
public:
    TF_HiveOps() noexcept :
        m_handle{.plugin_data = this}
    {
    }

    TF_HiveOps(const TF_HiveOps&) = delete;
    TF_HiveOps& operator=(const TF_HiveOps&) = delete;

    static TF_HiveOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TF_HiveOps*>(ctx);
    }

    template<typename HandleT>
    static TF_HiveOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TF_HiveOps*>(handle->plugin_data);
    }

    virtual ~TF_HiveOps() = default;
    virtual void set_element_size(size_t element_size) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    insert(const void* value, TFHiveSlot* out_slot) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    erase(TFHiveSlot* slot) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    get(const TFHiveSlot* slot, const void** out_value) noexcept = 0;
    virtual void for_each(TF_HiveVisitor visitor, void* capture) noexcept = 0;
    virtual void size(size_t* out_size) noexcept = 0;
    virtual void destroy() noexcept = 0;

    void get_generic_vtable() noexcept
    {
        m_vtable = ::TF_HiveOps{
            .struct_size = TF_HIVE_STRUCT_SIZE,
            .set_element_size =
                [](TF_Hive* hive, size_t element_size) noexcept
            {
                TF_HiveOps::from_handle(hive).set_element_size(element_size);
            },
            .insert =
                [](TF_Hive* hive,
                   const void* value,
                   TFHiveSlot* out_slot,
                   TF_Status* out_status) noexcept
            {
                auto res = TF_HiveOps::from_handle(hive).insert(value, out_slot);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .erase =
                [](TF_Hive* hive, TFHiveSlot* slot, TF_Status* out_status) noexcept
            {
                auto res = TF_HiveOps::from_handle(hive).erase(slot);
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
                auto res = TF_HiveOps::from_handle(hive).get(slot, out_value);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .for_each =
                [](const TF_Hive* hive, TF_HiveVisitor visitor, void* capture) noexcept
            {
                TF_HiveOps::from_handle(hive).for_each(visitor, capture);
            },
            .size =
                [](const TF_Hive* hive, size_t* out_size) noexcept
            {
                TF_HiveOps::from_handle(hive).size(out_size);
            },
            .destroy =
                [](TF_Hive* hive) noexcept
            {
                TF_HiveOps::from_handle(hive).destroy();
            },

        };
    }

    const ::TF_HiveOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const TF_Hive& get_handle() const noexcept
    {
        return m_handle;
    }


private:
    ::TF_HiveOps m_vtable;
    TF_Hive m_handle;
};

} // namespace ice::builder
