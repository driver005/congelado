// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/jobber/options.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/jobber/job.h"
#include "include/c/extern/jobber/options.h"
#include "include/c/extern/registration/registration.h"
#include "include/c/intern/status.h"

export module cc_ice_extern_jobber_builder:options;

import std;
import cc_ice_extern_jobber_sonic;
import cc_ice_intern_sonic;

export namespace ice::builder {

class TF_OptionsOps
{
public:
    explicit TF_OptionsOps(
        const ::TF_JobOps* TF_JobOps_ops,
        const ::TF_StatusOps* Status_ops
    ) noexcept :
        m_handle{.plugin_data = this}
    {
        m_TF_JobOps_ops = TF_JobOps_ops;
        m_Status_ops = Status_ops;
    }

    TF_OptionsOps(const TF_OptionsOps&) = delete;
    TF_OptionsOps& operator=(const TF_OptionsOps&) = delete;

    static TF_OptionsOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TF_OptionsOps*>(ctx);
    }

    template<typename HandleT>
    static TF_OptionsOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TF_OptionsOps*>(handle->plugin_data);
    }

    virtual ~TF_OptionsOps() = default;
    virtual void destroy() noexcept = 0;
    virtual void get_options(
        const ice::sonic::TF_JobOps& job,
        TFJobOptions* out_options,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void update_options(
        const ice::sonic::TF_JobOps& job,
        const TFJobOptions* new_options,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void set_priority(
        const ice::sonic::TF_JobOps& job,
        int priority,
        const ice::sonic::Status& out_status
    ) noexcept = 0;

    void get_generic_vtable(void (*create)(::TF_Options*)) noexcept
    {
        m_vtable = ::TF_OptionsOps{
            .struct_size = TF_OFFSET_OF_END(::TF_OptionsOps, set_priority),

            .create = create,
            .destroy =
                [](TF_Options* handle) noexcept
            {
                auto& self = TF_OptionsOps::from_handle(handle);
                self.destroy();
            },
            .get_options =
                [](TF_Options* options,
                   TF_Job* job,
                   TFJobOptions* out_options,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_OptionsOps::from_handle(options);
                self.get_options(
                    self.wrap(std::type_identity<ice::sonic::TF_JobOps>{}, job),
                    out_options,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .update_options =
                [](TF_Options* options,
                   TF_Job* job,
                   const TFJobOptions* new_options,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_OptionsOps::from_handle(options);
                self.update_options(
                    self.wrap(std::type_identity<ice::sonic::TF_JobOps>{}, job),
                    new_options,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .set_priority =
                [](TF_Options* options, TF_Job* job, int priority, TF_Status* out_status) noexcept
            {
                auto& self = TF_OptionsOps::from_handle(options);
                self.set_priority(
                    self.wrap(std::type_identity<ice::sonic::TF_JobOps>{}, job),
                    priority,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },

        };
    }

    ice::sonic::TF_JobOps
    wrap(std::type_identity<ice::sonic::TF_JobOps>, const ::TF_Job* handle) const noexcept
    {
        return ice::sonic::TF_JobOps{m_TF_JobOps_ops, const_cast<::TF_Job*>(handle)};
    }

    ice::sonic::Status
    wrap(std::type_identity<ice::sonic::Status>, const ::TF_Status* handle) const noexcept
    {
        return ice::sonic::Status{m_Status_ops, const_cast<::TF_Status*>(handle)};
    }

    const ::TF_OptionsOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const ::TF_Options& get_handle() const noexcept
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
            const_cast<::TF_OptionsOps*>(&m_vtable)
        );
    }

private:
    ::TF_OptionsOps m_vtable;
    ::TF_Options m_handle;

    const ::TF_JobOps* m_TF_JobOps_ops{nullptr};

    const ::TF_StatusOps* m_Status_ops{nullptr};
};

} // namespace ice::builder
