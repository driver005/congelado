module;

#include "c/extern/cron/cron.h"

export module cc_abi_sonic_cron;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;
import cc_abi_sonic_registration;

export namespace ice::sonic {

// Runtime — the mainframe-facing cron handle. Same in-process/cross-plugin duality as
// ice::sonic::Cache and ice::sonic::Generator.
class Cron : public ice::sonic::Runtime<Cron, TF_Cron>
{
public:
    explicit Cron(TF_Cron* ops, void* plugin_context) :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "cron";

    [[nodiscard]] std::expected<bool, ice::Status> validate(const ice::String& expression)
    {
        ice::Status status;
        bool result =
            m_ops->validate(get_handle(), expression.get_handle(), status.get_handle()) != 0;
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return result;
    }

    [[nodiscard]] std::expected<bool, ice::Status>
    next_after(const ice::String& expression, std::int64_t base_time_ms, std::int64_t* out_time_ms)
    {
        ice::Status status;
        bool result = m_ops->next_after(
                          get_handle(),
                          expression.get_handle(),
                          base_time_ms,
                          out_time_ms,
                          status.get_handle()
                      ) != 0;
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return result;
    }

    [[nodiscard]] std::expected<void, ice::Status>
    upsert_job(const ice::String& name, const ice::String& expression)
    {
        ice::Status status;
        m_ops->upsert_job(
            get_handle(),
            name.get_handle(),
            expression.get_handle(),
            status.get_handle()
        );
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> remove_job(const ice::String& name)
    {
        ice::Status status;
        m_ops->remove_job(get_handle(), name.get_handle(), status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    ice::String get_name() const
    {
        ice::String out;
        m_ops->get_name(get_handle(), out.get_handle());
        return out;
    }
};

} // namespace ice::sonic
