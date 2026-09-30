// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/jobber/options.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/jobber/options.h"

export module cc_ice_extern_jobber_builder:options;

import std;

export namespace ice::builder {

class TF_OptionsOps
{
public:
    TF_OptionsOps() noexcept :
        m_handle{.plugin_data = this}
    {
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
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    get_options(const ice::sonic::TF_JobOps& job, TFJobOptions* out_options) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    update_options(const ice::sonic::TF_JobOps& job, const TFJobOptions* new_options) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    set_priority(const ice::sonic::TF_JobOps& job, int priority) noexcept = 0;

    void get_generic_vtable() noexcept
    {
        m_vtable = ::TF_OptionsOps{
            .struct_size = TF_OPTIONS_STRUCT_SIZE,
            .destroy =
                [](TF_Options* options) noexcept
            {
                TF_OptionsOps::from_handle(options).destroy();
            },
            .get_options =
                [](TF_Options* options,
                   TF_Job* job,
                   TFJobOptions* out_options,
                   TF_Status* out_status) noexcept
            {
                auto res = TF_OptionsOps::from_handle(options).get_options(
                    ice::sonic::TF_JobOps::wrap(job),
                    out_options
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .update_options =
                [](TF_Options* options,
                   TF_Job* job,
                   const TFJobOptions* new_options,
                   TF_Status* out_status) noexcept
            {
                auto res = TF_OptionsOps::from_handle(options).update_options(
                    ice::sonic::TF_JobOps::wrap(job),
                    new_options
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .set_priority =
                [](TF_Options* options, TF_Job* job, int priority, TF_Status* out_status) noexcept
            {
                auto res = TF_OptionsOps::from_handle(options).set_priority(
                    ice::sonic::TF_JobOps::wrap(job),
                    priority
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },

        };
    }

    const ::TF_OptionsOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const TF_Options& get_handle() const noexcept
    {
        return m_handle;
    }


private:
    ::TF_OptionsOps m_vtable;
    TF_Options m_handle;
};

} // namespace ice::builder
