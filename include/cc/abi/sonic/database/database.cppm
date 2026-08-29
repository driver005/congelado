module;

#include "c/extern/database/database.h"

export module cc_abi_sonic_database;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;
import cc_abi_sonic_registration;

export namespace ice::sonic {

// Runtime — the mainframe-facing database handle. Same in-process/cross-plugin duality as
// ice::sonic::Cache and ice::sonic::Generator.
class Database : public ice::sonic::Runtime<Database, TF_Database>
{
public:
    explicit Database(TF_Database* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "database";

    [[nodiscard]] std::expected<bool, ice::Status> is_connected() noexcept
    {
        ice::Status status;
        bool result = m_ops->is_connected(get_handle(), status.get_handle()) != 0;
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return result;
    }

    [[nodiscard]] std::expected<void, ice::Status>
    query(const ice::String& payload, TF_Database_CompletionFn completion, void* cb_user_data) noexcept
    {
        ice::Status status;
        m_ops->query(
            get_handle(),
            payload.get_handle(),
            completion,
            cb_user_data,
            status.get_handle()
        );
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    insert(const ice::String& payload, TF_Database_CompletionFn completion, void* cb_user_data) noexcept
    {
        ice::Status status;
        m_ops->insert(
            get_handle(),
            payload.get_handle(),
            completion,
            cb_user_data,
            status.get_handle()
        );
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    update(const ice::String& payload, TF_Database_CompletionFn completion, void* cb_user_data) noexcept
    {
        ice::Status status;
        m_ops->update(
            get_handle(),
            payload.get_handle(),
            completion,
            cb_user_data,
            status.get_handle()
        );
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    remove(const ice::String& payload, TF_Database_CompletionFn completion, void* cb_user_data) noexcept
    {
        ice::Status status;
        m_ops->remove(
            get_handle(),
            payload.get_handle(),
            completion,
            cb_user_data,
            status.get_handle()
        );
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    ice::String get_name() const noexcept
    {
        ice::String out;
        m_ops->get_name(get_handle(), out.get_handle());
        return out;
    }
};

} // namespace ice::sonic
