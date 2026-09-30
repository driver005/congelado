// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/store/index.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/store/index.h"

export module cc_ice_extern_store_sonic:index;

import std;
import cc_abi_sonic_registration;

export namespace ice::sonic {

class TFStoreIndexOps : public ice::sonic::Runtime<TFStoreIndexOps, TFStoreIndexOps>
{
public:
    explicit TFStoreIndexOps(TFStoreIndexOps* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "store";

    void destroy() noexcept
    {
        m_ops->destroy(get_handle());
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    create(const ice::sonic::String& name, const ice::sonic::TF_MapOps& field_config) noexcept
    {
        ice::sonic::Status status;
        m_ops->create(
            get_handle(),
            name.get_handle(),
            field_config.get_handle(),
            status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    drop(const ice::sonic::String& name) noexcept
    {
        ice::sonic::Status status;
        m_ops->drop(get_handle(), name.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    list(const ice::sonic::TF_VectorOps& out_names) noexcept
    {
        ice::sonic::Status status;
        m_ops->list(get_handle(), out_names.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }
};

} // namespace ice::sonic
