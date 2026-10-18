// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/jobber/options.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/jobber/job.h"
#include "include/c/extern/jobber/options.h"
#include "include/c/intern/registration.h"
#include "include/c/intern/status.h"

export module cc_ice_extern_jobber_builder:options;

import std;
import cc_ice_extern_jobber_sonic;
import cc_ice_intern_sonic;

export namespace ice::builder {

class Options
{
public:
    explicit Options(const ::TF_JobOps* Job_ops, const ::TF_StatusOps* Status_ops) noexcept :
        m_handle{.plugin_data = this}
    {
        m_Job_ops = Job_ops;
        m_Status_ops = Status_ops;
    }

    Options(const Options&) = delete;
    Options& operator=(const Options&) = delete;

    static Options& from_handle(void* ctx) noexcept
    {
        return *static_cast<Options*>(ctx);
    }

    template<typename HandleT>
    static Options& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<Options*>(handle->plugin_data);
    }

    virtual ~Options() = default;
    virtual void destroy() noexcept = 0;
    virtual void get_options(
        const ice::sonic::Job& job,
        ice::sonic::JobOptions* out_options,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void update_options(
        const ice::sonic::Job& job,
        const ice::sonic::JobOptions* new_options,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void set_priority(
        const ice::sonic::Job& job,
        int priority,
        const ice::sonic::Status& out_status
    ) noexcept = 0;

    void get_generic_vtable(void (*create)(::TF_Options*)) noexcept
    {
        m_vtable = ::TF_OptionsOps{
            .struct_size = TF_OFFSET_OF_END(::TF_OptionsOps, set_priority),

            .create = create,
            .destroy =
                [](::TF_Options* handle) noexcept
            {
                auto& self = Options::from_handle(handle);
                self.destroy();
            },
            .get_options =
                [](::TF_Options* options,
                   ::TF_Job* job,
                   ::TFJobOptions* out_options,
                   ::TF_Status* out_status) noexcept
            {
                auto& self = Options::from_handle(options);
                std::optional<ice::sonic::JobOptions> wrapped_out_options;
                if (out_options != nullptr) {
                    wrapped_out_options.emplace(*out_options);
                }
                self.get_options(
                    self.wrap(std::type_identity<ice::sonic::Job>{}, job),
                    wrapped_out_options ? &*wrapped_out_options : nullptr,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
                if (out_options != nullptr) {
                    *out_options = *wrapped_out_options->get_handle();
                }
            },
            .update_options =
                [](::TF_Options* options,
                   ::TF_Job* job,
                   const ::TFJobOptions* new_options,
                   ::TF_Status* out_status) noexcept
            {
                auto& self = Options::from_handle(options);
                std::optional<ice::sonic::JobOptions> wrapped_new_options;
                if (new_options != nullptr) {
                    wrapped_new_options.emplace(*new_options);
                }
                self.update_options(
                    self.wrap(std::type_identity<ice::sonic::Job>{}, job),
                    wrapped_new_options ? &*wrapped_new_options : nullptr,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .set_priority =
                [](::TF_Options* options,
                   ::TF_Job* job,
                   int priority,
                   ::TF_Status* out_status) noexcept
            {
                auto& self = Options::from_handle(options);
                self.set_priority(
                    self.wrap(std::type_identity<ice::sonic::Job>{}, job),
                    priority,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },

        };
    }

    ice::sonic::Job wrap(std::type_identity<ice::sonic::Job>, const ::TF_Job* handle) const noexcept
    {
        return ice::sonic::Job{m_Job_ops, handle};
    }

    ice::sonic::Status
    wrap(std::type_identity<ice::sonic::Status>, const ::TF_Status* handle) const noexcept
    {
        return ice::sonic::Status{m_Status_ops, handle};
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
        const ice::sonic::Registration& registry,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) const noexcept
    {
        registry.register_op(type, provider, const_cast<::TF_OptionsOps*>(&m_vtable));
    }

private:
    ::TF_OptionsOps m_vtable;
    ::TF_Options m_handle;

    const ::TF_JobOps* m_Job_ops{nullptr};

    const ::TF_StatusOps* m_Status_ops{nullptr};
};

} // namespace ice::builder
