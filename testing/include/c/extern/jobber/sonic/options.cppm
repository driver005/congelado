// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/jobber/options.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/jobber/options.h"

export module cc_abi_sonic_jobber;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;
import cc_abi_sonic_registration;

export namespace ice::sonic {

class TF_OptionsOps : public ice::sonic::Runtime<TF_OptionsOps, TF_OptionsOps>
{
public:
    explicit TF_OptionsOps(TF_OptionsOps* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "jobber";

    [[nodiscard]] std::expected<void, ice::Status>
    get_options(const ice::sonic::TF_JobOps& job, TFJobOptions* out_options) noexcept
    {
        ice::Status status;
        m_ops->get_options(get_handle(), job.get_handle(), out_options status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    update_options(const ice::sonic::TF_JobOps& job, const TFJobOptions* new_options) noexcept
    {
        ice::Status status;
        m_ops->update_options(get_handle(), job.get_handle(), new_options status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    set_priority(const ice::sonic::TF_JobOps& job, int priority) noexcept
    {
        ice::Status status;
        m_ops->set_priority(get_handle(), job.get_handle(), priority status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    virtual ice::String get_name() const noexcept = 0;

    ice::String get_name() const noexcept
    {
        ice::String result;
        m_ops->get_name(get_handle(), result.get_handle());
        return result;
    }
};

} // namespace ice::sonic
