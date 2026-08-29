module;

#include "c/extern/logger/logger.h"

export module cc_abi_sonic_logger;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;
import cc_abi_sonic_registration;

export namespace ice::sonic {

// Runtime — the mainframe-facing logger handle. Same in-process/cross-plugin duality as
// ice::sonic::Cache and ice::sonic::Generator.
class Logger : public ice::sonic::Runtime<Logger, TF_Logger>
{
public:
    explicit Logger(TF_Logger* ops, void* plugin_context) :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "logger";

    [[nodiscard]] std::expected<void, ice::Status> debug(const ice::String& message)
    {
        ice::Status status;
        m_ops->debug(get_handle(), message.get_handle(), status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> info(const ice::String& message)
    {
        ice::Status status;
        m_ops->info(get_handle(), message.get_handle(), status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> important(const ice::String& message)
    {
        ice::Status status;
        m_ops->important(get_handle(), message.get_handle(), status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> warning(const ice::String& message)
    {
        ice::Status status;
        m_ops->warning(get_handle(), message.get_handle(), status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> error(const ice::String& message)
    {
        ice::Status status;
        m_ops->error(get_handle(), message.get_handle(), status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> fatal(const ice::String& message)
    {
        ice::Status status;
        m_ops->fatal(get_handle(), message.get_handle(), status.get_handle());
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
