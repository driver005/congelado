// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/jobber/options.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/jobber/options.h"

export module cc_abi_builder_jobber;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;

export namespace ice::builder {

class TF_OptionsOps
{
public:
    static TF_OptionsOps* create(void* ctx) noexcept
    {
        return static_cast<TF_OptionsOps*>(ctx);
    }

    template<typename HandleT>
    static TF_OptionsOps* create(HandleT* handle) noexcept
    {
        return static_cast<TF_OptionsOps*>(handle->plugin_data);
    }

    virtual ~TF_OptionsOps() = default;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_options(const ice::sonic::TF_JobOps& job, TFJobOptions* out_options) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    update_options(const ice::sonic::TF_JobOps& job, const TFJobOptions* new_options) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    set_priority(const ice::sonic::TF_JobOps& job, int priority) noexcept = 0;

    static TF_OptionsOps* get_generic_vtable()
    {
        static TF_OptionsOps vtable = {
            .struct_size = TF_OPTIONS_STRUCT_SIZE,

            .destroy =
                [](void* plugin_context) noexcept
            {
                delete TF_OptionsOps::create(plugin_context);
            },
            .get_options =
                [](TF_Options* options,
                   TF_Job* job,
                   TFJobOptions* out_options,
                   TF_Status* out_status) noexcept
            {
                auto* self = TF_OptionsOps::create(options);
                auto res = self->get_options(ice::sonic::TF_JobOps::wrap(job), out_options);
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
                auto* self = TF_OptionsOps::create(options);
                auto res = self->update_options(ice::sonic::TF_JobOps::wrap(job), new_options);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .set_priority =
                [](TF_Options* options, TF_Job* job, int priority, TF_Status* out_status) noexcept
            {
                auto* self = TF_OptionsOps::create(options);
                auto res = self->set_priority(ice::sonic::TF_JobOps::wrap(job), priority);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },

        };

        return &vtable;
    }
};

} // namespace ice::builder
