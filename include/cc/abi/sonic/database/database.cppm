module;

#include "c/extern/database/database.h"

export module cc_abi_sonic_database;

import std;
import cc_abi_sonic_intern;
import cc_abi_primitives;
import cc_abi_sonic_registration;

namespace ice::sonic::detail {
inline TF_Database_CompletionFn to_c(ice::builder::CompletionFn fn) noexcept {
    static_assert(sizeof(ice::builder::CompletionFn) == sizeof(TF_Database_CompletionFn));
    return std::bit_cast<TF_Database_CompletionFn>(fn);
}
} // namespace ice::sonic::detail
export namespace ice::sonic {

// Runtime — the mainframe-facing database handle. Same in-process/cross-plugin duality as
// ice::sonic::Cache and ice::sonic::Generator.
class Database : public ice::sonic::Runtime<Database, TF_Database, /*PassNameToFactory=*/true>
{
public:
    static constexpr std::string_view domain_name = "database";

    std::expected<bool, ice::Status> is_connected()
    {


        ice::Status status;
        bool result = this->m_ops->is_connected(this->get_handle(), status.get_handle()) != 0;
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return result;
    }

    std::expected<void, ice::Status> query(
        const ice::String& payload,
        ice::builder::CompletionFn completion,
        void* cb_user_data
    )
    {
        ice::Status status;
        this->m_ops->query(this->get_handle(), payload.get_handle(), detail::to_c(completion), cb_user_data, status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    std::expected<void, ice::Status> insert(
        const ice::String& payload,
        ice::builder::CompletionFn completion,
        void* cb_user_data
    )
    {
        ice::Status status;
        this->m_ops->insert(this->get_handle(), payload.get_handle(), detail::to_c(completion), cb_user_data, status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    std::expected<void, ice::Status> update(
        const ice::String& payload,
        ice::builder::CompletionFn completion,
        void* cb_user_data
    )
    {
        ice::Status status;
        this->m_ops->update(this->get_handle(), payload.get_handle(), detail::to_c(completion), cb_user_data, status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    std::expected<void, ice::Status> remove(
        const ice::String& payload,
        ice::builder::CompletionFn completion,
        void* cb_user_data
    )
    {
        ice::Status status;
        this->m_ops->remove(this->get_handle(), payload.get_handle(), detail::to_c(completion), cb_user_data, status.get_handle());
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
    explicit Database(TF_Database* ops, void* plugin_context) : Runtime(ops, plugin_context) {}
};

} // namespace ice::sonic
