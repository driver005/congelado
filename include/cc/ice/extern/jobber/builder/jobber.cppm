// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/jobber/jobber.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/jobber/jobber.h"
#include "include/c/extern/registration/registration.h"
#include "include/c/intern/tstring.h"

export module cc_ice_extern_jobber_builder:jobber;

import std;
import cc_ice_intern_sonic;

export namespace ice::builder {

class TF_JobberOps
{
public:
    explicit TF_JobberOps(const ::TF_StringOps* String_ops) noexcept :
        m_handle{.plugin_data = this}
    {
        m_String_ops = String_ops;
    }

    TF_JobberOps(const TF_JobberOps&) = delete;
    TF_JobberOps& operator=(const TF_JobberOps&) = delete;

    static TF_JobberOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TF_JobberOps*>(ctx);
    }

    template<typename HandleT>
    static TF_JobberOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TF_JobberOps*>(handle->plugin_data);
    }

    virtual ~TF_JobberOps() = default;
    virtual void destroy() noexcept = 0;
    virtual void get_name(const ice::sonic::String& out_name) noexcept = 0;

    void get_generic_vtable(void (*create)(::TF_Jobber*)) noexcept
    {
        m_vtable = ::TF_JobberOps{
            .struct_size = TF_OFFSET_OF_END(::TF_JobberOps, get_name),

            .create = create,
            .destroy =
                [](TF_Jobber* handle) noexcept
            {
                auto& self = TF_JobberOps::from_handle(handle);
                self.destroy();
            },
            .get_name =
                [](TF_Jobber* jobber, TF_String* out_name) noexcept
            {
                auto& self = TF_JobberOps::from_handle(jobber);
                self.get_name(self.wrap(std::type_identity<ice::sonic::String>{}, out_name));
            },

        };
    }

    ice::sonic::String
    wrap(std::type_identity<ice::sonic::String>, const ::TF_String* handle) const noexcept
    {
        return ice::sonic::String{m_String_ops, const_cast<::TF_String*>(handle)};
    }

    const ::TF_JobberOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const ::TF_Jobber& get_handle() const noexcept
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
            const_cast<::TF_JobberOps*>(&m_vtable)
        );
    }

private:
    ::TF_JobberOps m_vtable;
    ::TF_Jobber m_handle;

    const ::TF_StringOps* m_String_ops{nullptr};
};

} // namespace ice::builder
