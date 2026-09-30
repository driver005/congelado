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
    virtual void destroy() noexcept = 0;
    virtual void get_name(const ice::sonic::String& out_name) noexcept = 0;

    void get_generic_vtable() noexcept
    {
        m_vtable = ::TF_JobberOps{
            .struct_size = TF_JOBBER_STRUCT_SIZE,
            .destroy =
                [](TF_Jobber* jobber) noexcept
            {
                TF_JobberOps::from_handle(jobber).destroy();
            },
            .get_name =
                [](TF_Jobber* jobber, TF_String* out_name) noexcept
            {
                TF_JobberOps::from_handle(jobber).get_name(ice::sonic::String::wrap(out_name));
            },

        };
    }

    const ::TF_JobberOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const TF_Jobber& get_handle() const noexcept
    {
        return m_handle;
    }


private:
    ::TF_JobberOps m_vtable;
    TF_Jobber m_handle;
};

} // namespace ice::builder
