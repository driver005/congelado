// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/ops/ops.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/ops/ops.h"
#include "include/c/extern/registration/registration.h"
#include "include/c/intern/tstring.h"

export module cc_ice_extern_ops_builder:ops;

import std;
import cc_ice_intern_sonic;

export namespace ice::builder {

class TF_OpsOps
{
public:
    explicit TF_OpsOps(const ::TF_StringOps* String_ops) noexcept :
        m_handle{.plugin_data = this}
    {
        m_String_ops = String_ops;
    }

    TF_OpsOps(const TF_OpsOps&) = delete;
    TF_OpsOps& operator=(const TF_OpsOps&) = delete;

    static TF_OpsOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TF_OpsOps*>(ctx);
    }

    template<typename HandleT>
    static TF_OpsOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TF_OpsOps*>(handle->plugin_data);
    }

    virtual ~TF_OpsOps() = default;
    virtual void destroy() noexcept = 0;
    virtual void get_name(const ice::sonic::String& out_name) noexcept = 0;

    void get_generic_vtable(void (*create)(::TF_Ops*)) noexcept
    {
        m_vtable = ::TF_OpsOps{
            .struct_size = TF_OFFSET_OF_END(::TF_OpsOps, get_name),

            .create = create,
            .destroy =
                [](TF_Ops* handle) noexcept
            {
                auto& self = TF_OpsOps::from_handle(handle);
                self.destroy();
            },
            .get_name =
                [](TF_Ops* ops_facade, TF_String* out_name) noexcept
            {
                auto& self = TF_OpsOps::from_handle(ops_facade);
                self.get_name(self.wrap(std::type_identity<ice::sonic::String>{}, out_name));
            },

        };
    }

    ice::sonic::String
    wrap(std::type_identity<ice::sonic::String>, const ::TF_String* handle) const noexcept
    {
        return ice::sonic::String{m_String_ops, const_cast<::TF_String*>(handle)};
    }

    const ::TF_OpsOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const ::TF_Ops& get_handle() const noexcept
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
            const_cast<::TF_OpsOps*>(&m_vtable)
        );
    }

private:
    ::TF_OpsOps m_vtable;
    ::TF_Ops m_handle;

    const ::TF_StringOps* m_String_ops{nullptr};
};

} // namespace ice::builder
