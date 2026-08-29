module;

#include "c/extern/cache/cache.h"

export module cc_abi_sonic_cache;

import std;
import cc_abi_sonic_intern;
import cc_abi_primitives;
import cc_abi_sonic_registration;

namespace ice::sonic::detail {
// completion_to_c: bridges ice::builder::CompletionFn → TF_Cache_CompletionFn.
// Both function pointer types carry identical register-level signatures
// (const TF_TString*, void*) — ice::String is standard-layout with TF_TString as its only
// member, so String::create() can alias through it. std::bit_cast is size-checked at
// compile time and avoids the UB of a raw reinterpret_cast.
inline TF_Cache_CompletionFn to_c(ice::builder::CompletionFn fn) noexcept {
    static_assert(sizeof(ice::builder::CompletionFn) == sizeof(TF_Cache_CompletionFn));
    return std::bit_cast<TF_Cache_CompletionFn>(fn);
}
} // namespace ice::sonic::detail
export namespace ice::sonic {

// Runtime — cross-plugin C ABI handle. Genuine third-party plugins are independently built
// (possibly by a different compiler/toolchain), so this always crosses the opaque-handle C ABI
// to avoid C++ ABI incompatibility (no in-process virtual method sharing).
class Cache : public ice::sonic::Runtime<Cache, TF_Cache, /*PassNameToFactory=*/true>
{
public:
    static constexpr std::string_view domain_name = "cache";

    std::expected<void, ice::Status> get(
        const ice::String& key,
        ice::builder::CompletionFn completion,
        void* cb_user_data
    )
    {
        ice::Status status;
        this->m_ops->get(this->get_handle(), key.get_handle(),
            detail::to_c(completion), cb_user_data, status.get_handle()
        );
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    std::expected<void, ice::Status> set(
        const ice::String& key,
        const ice::String& value,
        ice::builder::CompletionFn completion,
        void* cb_user_data
    )
    {
        ice::Status status;
        this->m_ops->set(this->get_handle(), key.get_handle(), value.get_handle(),
            detail::to_c(completion), cb_user_data, status.get_handle()
        );
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    std::expected<void, ice::Status> remove(
        const ice::String& key,
        ice::builder::CompletionFn completion,
        void* cb_user_data
    )
    {
        ice::Status status;
        this->m_ops->remove(this->get_handle(), key.get_handle(),
            detail::to_c(completion), cb_user_data, status.get_handle()
        );
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
    explicit Cache(TF_Cache* ops, void* plugin_context) : Runtime(ops, plugin_context) {}
};

} // namespace ice::sonic
