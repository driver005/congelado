module;

#include "c/extern/cache/cache.h"

export module cc_abi_sonic_cache;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;
import cc_abi_sonic_registration;

export namespace ice::sonic {

class Cache : public ice::sonic::Runtime<Cache, TF_Cache>
{
public:
    explicit Cache(TF_Cache* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "cache";

    [[nodiscard]] std::expected<void, ice::Status>
    get(const ice::String& key, TF_Cache_CompletionFn completion, void* cb_user_data) noexcept
    {
        ice::Status status;
        m_ops->get(get_handle(), key.get_handle(), completion, cb_user_data, status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    set(const ice::String& key,
        const ice::String& value,
        TF_Cache_CompletionFn completion,
        void* cb_user_data) noexcept
    {
        ice::Status status;
        m_ops->set(
            get_handle(),
            key.get_handle(),
            value.get_handle(),
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
    remove(const ice::String& key, TF_Cache_CompletionFn completion, void* cb_user_data) noexcept
    {
        ice::Status status;
        m_ops
            ->remove(get_handle(), key.get_handle(), completion, cb_user_data, status.get_handle());
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
