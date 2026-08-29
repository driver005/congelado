module;

#include "c/extern/cron/cron.h"

export module cc_abi_sonic_cron;

import std;
import cc_abi_sonic_intern;
import cc_abi_primitives;
import cc_abi_sonic_registration;
export namespace ice::sonic {

// Runtime — the mainframe-facing cron handle. Same in-process/cross-plugin duality as
// ice::sonic::Cache and ice::sonic::Generator.
class Cron : public ice::sonic::Runtime<Cron, TF_Cron, /*PassNameToFactory=*/true>
{
public:
    static constexpr std::string_view domain_name = "cron";

    std::expected<bool, ice::Status>
    validate(const ice::String& expression)
    {


        ice::Status status;
        bool result =
            this->m_ops->validate(this->get_handle(), expression.get_handle(), status.get_handle()) != 0;
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return result;
    }

    std::expected<bool, ice::Status> next_after(
        const ice::String& expression, std::int64_t base_time_ms,
        std::int64_t* out_time_ms
    )
    {


        ice::Status status;
        bool result = this->m_ops->next_after(this->get_handle(), expression.get_handle(), base_time_ms, out_time_ms,
                           status.get_handle()
                       )
                       != 0;
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return result;
    }

    std::expected<void, ice::Status> upsert_job(
        const ice::String& name, const ice::String& expression
    )
    {


        ice::Status status;
        this->m_ops->upsert_job(this->get_handle(), name.get_handle(), expression.get_handle(), status.get_handle()
        );
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    std::expected<void, ice::Status>
    remove_job(const ice::String& name)
    {


        ice::Status status;
        this->m_ops->remove_job(this->get_handle(), name.get_handle(), status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    ice::String get_name() const
    {


        ice::String tf_name;
        this->m_ops->get_name(this->get_handle(), tf_name.get_handle());
        return std::move(tf_name);
    }

public:
    explicit Cron(TF_Cron* ops, void* plugin_context) : Runtime(ops, plugin_context) {}
};

} // namespace ice::sonic
