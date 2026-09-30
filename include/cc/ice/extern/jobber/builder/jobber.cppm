// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/jobber/jobber.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/jobber/jobber.h"

export module cc_ice_extern_jobber_builder:jobber;

import std;

export namespace ice::builder {

class TF_JobberOps
{
public:
    TF_JobberOps() noexcept :
        m_handle{.plugin_data = this}
    {
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

    void get_generic_vtable() noexcept
    {
        m_vtable = ::TF_JobberOps{
            .struct_size = TF_JOBBER_STRUCT_SIZE,

            .destroy =
                [](void* plugin_context) noexcept
            {
                std::unique_ptr<TF_JobberOps>{&TF_JobberOps::from_handle(plugin_context)};
            },

            .get_name =
                [](void* plugin_context, TF_String* out) noexcept
            {
                auto result = TF_JobberOps::from_handle(plugin_context).get_name();
                result.to_c(out);
            },

        };
    }

    const ::TF_JobberOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    virtual builder::String get_name() const noexcept = 0;

    const TF_Jobber& get_handle() const noexcept
    {
        return m_handle;
    }


private:
    ::TF_JobberOps m_vtable;
    TF_Jobber m_handle;
};

} // namespace ice::builder
