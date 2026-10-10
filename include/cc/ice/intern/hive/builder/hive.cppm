// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/intern/hive/hive.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/registration/registration.h"
#include "include/c/intern/hive/hive.h"
#include "include/c/intern/status.h"

export module cc_ice_intern_hive_builder:hive;

import std;
import cc_ice_intern_sonic;

export namespace ice::builder {

class TF_HiveOps
{
public:
    explicit TF_HiveOps(const ::TF_StatusOps* Status_ops) noexcept :
        m_handle{.plugin_data = this}
    {
        m_Status_ops = Status_ops;
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
    virtual void destroy() noexcept = 0;
    virtual void set_element_size(size_t element_size) noexcept = 0;
    virtual void insert(
        const void* value,
        TFHiveSlot* out_slot,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void erase(TFHiveSlot* slot, const ice::sonic::Status& out_status) noexcept = 0;
    virtual void
    get(const TFHiveSlot* slot,
        const void** out_value,
        const ice::sonic::Status& out_status) noexcept = 0;
    virtual void for_each(TF_HiveVisitor visitor, void* capture) noexcept = 0;
    virtual void size(size_t* out_size) noexcept = 0;

    void get_generic_vtable(void (*create)(::TF_Hive*)) noexcept
    {
        m_vtable = ::TF_HiveOps{
            .struct_size = TF_OFFSET_OF_END(::TF_HiveOps, size),

            .create = create,
            .destroy =
                [](TF_Hive* handle) noexcept
            {
                auto& self = TF_HiveOps::from_handle(handle);
                self.destroy();
            },
            .set_element_size =
                [](TF_Hive* hive, size_t element_size) noexcept
            {
                auto& self = TF_HiveOps::from_handle(hive);
                self.set_element_size(element_size);
            },
            .insert =
                [](TF_Hive* hive,
                   const void* value,
                   TFHiveSlot* out_slot,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_HiveOps::from_handle(hive);
                self.insert(
                    value,
                    out_slot,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .erase =
                [](TF_Hive* hive, TFHiveSlot* slot, TF_Status* out_status) noexcept
            {
                auto& self = TF_HiveOps::from_handle(hive);
                self.erase(slot, self.wrap(std::type_identity<ice::sonic::Status>{}, out_status));
            },
            .get =
                [](const TF_Hive* hive,
                   const TFHiveSlot* slot,
                   const void** out_value,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_HiveOps::from_handle(hive);
                self.get(
                    slot,
                    out_value,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .for_each =
                [](const TF_Hive* hive, TF_HiveVisitor visitor, void* capture) noexcept
            {
                auto& self = TF_HiveOps::from_handle(hive);
                self.for_each(visitor, capture);
            },
            .size =
                [](const TF_Hive* hive, size_t* out_size) noexcept
            {
                auto& self = TF_HiveOps::from_handle(hive);
                self.size(out_size);
            },

        };
    }

    ice::sonic::Status
    wrap(std::type_identity<ice::sonic::Status>, const ::TF_Status* handle) const noexcept
    {
        return ice::sonic::Status{m_Status_ops, const_cast<::TF_Status*>(handle)};
    }

    const ::TF_HiveOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const ::TF_Hive& get_handle() const noexcept
    {
        return m_handle;
    }

    void register_ops(
        const ::TF_RegistrationOps& registry_ops,
        ::TF_Registration* registry_handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) const noexcept
    {
        registry_ops.register_op(
            registry_handle,
            type.get_handle(),
            provider.get_handle(),
            const_cast<::TF_HiveOps*>(&m_vtable)
        );
    }

private:
    ::TF_HiveOps m_vtable;
    ::TF_Hive m_handle;

    const ::TF_StatusOps* m_Status_ops{nullptr};
};

} // namespace ice::builder
