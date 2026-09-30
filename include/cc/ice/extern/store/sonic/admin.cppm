// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/store/admin.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/store/admin.h"

export module cc_ice_extern_store_sonic:admin;

import std;
import cc_abi_sonic_registration;

export namespace ice::sonic {

class TFStoreAdminOps : public ice::sonic::Runtime<TFStoreAdminOps, TFStoreAdminOps>
{
public:
    explicit TFStoreAdminOps(TFStoreAdminOps* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "store";

    [[nodiscard]] std::expected<void, ice::Status> is_connected(int* out_connected) noexcept
    {
        ice::Status status;
        m_ops->is_connected(get_handle(), out_connected, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    backup(const ice::sonic::String& destination, TFStoreAckFn completion, void* user_data) noexcept
    {
        ice::Status status;
        m_ops->backup(
            get_handle(),
            destination.get_handle(),
            completion,
            user_data,
            status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    restore(const ice::sonic::String& source, TFStoreAckFn completion, void* user_data) noexcept
    {
        ice::Status status;
        m_ops->restore(
            get_handle(),
            source.get_handle(),
            completion,
            user_data,
            status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    virtual ice::String get_name() const noexcept = 0;

    sonic::String get_name() const noexcept
    {
        sonic::String result;
        m_ops->get_name(get_handle(), result.get_handle());
        return result;
    }
};

} // namespace ice::sonic
