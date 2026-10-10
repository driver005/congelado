// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/intern/set.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/registration/registration.h"
#include "include/c/intern/set.h"
#include "include/c/intern/status.h"

export module cc_ice_intern_builder:set;

import std;
import cc_ice_intern_sonic;

export namespace ice::builder {

class TF_SetOps
{
public:
    explicit TF_SetOps(const ::TF_StatusOps* Status_ops) noexcept :
        m_handle{.plugin_data = this}
    {
        m_Status_ops = Status_ops;
    }

    TF_SetOps(const TF_SetOps&) = delete;
    TF_SetOps& operator=(const TF_SetOps&) = delete;

    static TF_SetOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TF_SetOps*>(ctx);
    }

    template<typename HandleT>
    static TF_SetOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TF_SetOps*>(handle->plugin_data);
    }

    virtual ~TF_SetOps() = default;
    virtual void destroy() noexcept = 0;
    virtual void insert(const void* key, const ice::sonic::Status& out_status) noexcept = 0;
    virtual void find(
        const void* key,
        const void** out_value,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void erase(const void* key, const ice::sonic::Status& out_status) noexcept = 0;
    virtual void
    contains(const void* key, int* out_found, const ice::sonic::Status& out_status) noexcept = 0;
    virtual void size(size_t* out_size) noexcept = 0;
    virtual void for_each(TF_SetVisitor visitor, void* capture) noexcept = 0;

    void get_generic_vtable(void (*create)(::TF_Set*)) noexcept
    {
        m_vtable = ::TF_SetOps{
            .struct_size = TF_OFFSET_OF_END(::TF_SetOps, for_each),

            .create = create,
            .destroy =
                [](TF_Set* handle) noexcept
            {
                auto& self = TF_SetOps::from_handle(handle);
                self.destroy();
            },
            .insert =
                [](TF_Set* set, const void* key, TF_Status* out_status) noexcept
            {
                auto& self = TF_SetOps::from_handle(set);
                self.insert(key, self.wrap(std::type_identity<ice::sonic::Status>{}, out_status));
            },
            .find =
                [](const TF_Set* set,
                   const void* key,
                   const void** out_value,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_SetOps::from_handle(set);
                self.find(
                    key,
                    out_value,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .erase =
                [](TF_Set* set, const void* key, TF_Status* out_status) noexcept
            {
                auto& self = TF_SetOps::from_handle(set);
                self.erase(key, self.wrap(std::type_identity<ice::sonic::Status>{}, out_status));
            },
            .contains =
                [](const TF_Set* set,
                   const void* key,
                   int* out_found,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_SetOps::from_handle(set);
                self.contains(
                    key,
                    out_found,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .size =
                [](const TF_Set* set, size_t* out_size) noexcept
            {
                auto& self = TF_SetOps::from_handle(set);
                self.size(out_size);
            },
            .for_each =
                [](const TF_Set* set, TF_SetVisitor visitor, void* capture) noexcept
            {
                auto& self = TF_SetOps::from_handle(set);
                self.for_each(visitor, capture);
            },

        };
    }

    ice::sonic::Status
    wrap(std::type_identity<ice::sonic::Status>, const ::TF_Status* handle) const noexcept
    {
        return ice::sonic::Status{m_Status_ops, const_cast<::TF_Status*>(handle)};
    }

    const ::TF_SetOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const ::TF_Set& get_handle() const noexcept
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
            const_cast<::TF_SetOps*>(&m_vtable)
        );
    }

private:
    ::TF_SetOps m_vtable;
    ::TF_Set m_handle;

    const ::TF_StatusOps* m_Status_ops{nullptr};
};

} // namespace ice::builder
