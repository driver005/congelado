// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/store/transaction.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/store/transaction.h"

export module cc_abi_sonic_store;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;
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

    [[nodiscard]] std::expected<void, ice::Status> begin() noexcept
    {
        ice::Status status;
        m_ops->begin(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    add_collection(const ice::sonic::TFStoreCollectionOps& collection) noexcept
    {
        ice::Status status;
        m_ops->add_collection(get_handle(), collection.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> get_collection(
        const ice::sonic::TF_StringOps& name,
        const ice::sonic::TFStoreCollectionOps& out_collection
    ) noexcept
    {
        ice::Status status;
        m_ops->get_collection(
            get_handle(),
            name.get_handle(),
            out_collection.get_handle(),
            status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    list_collections(TF_Tensor** out_collections) noexcept
    {
        ice::Status status;
        m_ops->list_collections(get_handle(), out_collections, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    commit(TFStoreAckFn completion, void* user_data) noexcept
    {
        ice::Status status;
        m_ops->commit(get_handle(), completion, user_data, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> rollback() noexcept
    {
        ice::Status status;
        m_ops->rollback(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    ice::String get_name() const noexcept
    {
        ice::String result;
        m_ops->get_name(get_handle(), result.get_handle());
        return result;
    }
};

} // namespace ice::sonic
