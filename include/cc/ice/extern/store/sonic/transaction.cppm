// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/store/transaction.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/store/transaction.h"

export module cc_ice_extern_store_sonic:transaction;

import std;
import cc_abi_sonic_registration;

export namespace ice::sonic {

class TFStoreTransactionOps :
    public ice::sonic::Runtime<TFStoreTransactionOps, TFStoreTransactionOps>
{
public:
    explicit TFStoreTransactionOps(TFStoreTransactionOps* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "store";

    void destroy() noexcept
    {
        m_ops->destroy(get_handle());
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status> begin() noexcept
    {
        ice::sonic::Status status;
        m_ops->begin(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    add_collection(const ice::sonic::TFStoreCollectionOps& collection) noexcept
    {
        ice::sonic::Status status;
        m_ops->add_collection(get_handle(), collection.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    void get_collection(
        const ice::sonic::String& name,
        const ice::sonic::TFStoreCollectionOps& out_collection
    ) noexcept
    {
        m_ops->get_collection(get_handle(), name.get_handle(), out_collection.get_handle());
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    list_collections(TF_Tensor** out_collections) noexcept
    {
        ice::sonic::Status status;
        m_ops->list_collections(get_handle(), out_collections, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    commit(TFStoreAckFn completion, void* user_data) noexcept
    {
        ice::sonic::Status status;
        m_ops->commit(get_handle(), completion, user_data, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    void rollback() noexcept
    {
        m_ops->rollback(get_handle());
    }
};

} // namespace ice::sonic
