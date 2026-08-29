module;

#include "c/extern/logger/logger.h"

export module cc_abi_sonic_logger;

import std;
import cc_abi_sonic_intern;
import cc_abi_primitives;
import cc_abi_sonic_registration;
export namespace ice::sonic {

// Runtime — the mainframe-facing logger handle. Same in-process/cross-plugin duality as
// ice::sonic::Cache and ice::sonic::Generator.
class Logger : public ice::sonic::Runtime<Logger, TF_Logger, /*PassNameToFactory=*/true>
{
public:
    static constexpr std::string_view domain_name = "logger";

    std::expected<void, ice::Status> debug(const ice::String& message) {
        ice::Status status;
        this->m_ops->debug(this->get_handle(), message.get_handle(), status.get_handle());
        if (!status.ok()) return std::unexpected{status};
        return {};
    }

    std::expected<void, ice::Status> info(const ice::String& message) {
        ice::Status status;
        this->m_ops->info(this->get_handle(), message.get_handle(), status.get_handle());
        if (!status.ok()) return std::unexpected{status};
        return {};
    }

    std::expected<void, ice::Status> important(const ice::String& message) {
        ice::Status status;
        this->m_ops->important(this->get_handle(), message.get_handle(), status.get_handle());
        if (!status.ok()) return std::unexpected{status};
        return {};
    }

    std::expected<void, ice::Status> warning(const ice::String& message) {
        ice::Status status;
        this->m_ops->warning(this->get_handle(), message.get_handle(), status.get_handle());
        if (!status.ok()) return std::unexpected{status};
        return {};
    }

    std::expected<void, ice::Status> error(const ice::String& message) {
        ice::Status status;
        this->m_ops->error(this->get_handle(), message.get_handle(), status.get_handle());
        if (!status.ok()) return std::unexpected{status};
        return {};
    }

    std::expected<void, ice::Status> fatal(const ice::String& message) {
        ice::Status status;
        this->m_ops->fatal(this->get_handle(), message.get_handle(), status.get_handle());
        if (!status.ok()) return std::unexpected{status};
        return {};
    }

    ice::String get_name() const
    {


        ice::String tf_name;
        this->m_ops->get_name(this->get_handle(), tf_name.get_handle());
        return std::move(tf_name);
    }
    

public:
    explicit Logger(TF_Logger* ops, void* plugin_context) : Runtime(ops, plugin_context) {}
};

} // namespace ice::sonic
