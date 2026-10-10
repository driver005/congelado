// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/store/transaction.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/registration/registration.h"
#include "include/c/extern/store/transaction.h"

export module cc_ice_extern_store_sonic:transaction;

import std;
import :collection;
import cc_ice_intern_sonic;

export namespace ice::sonic {

class TFStoreTransactionOps :
    public ice::sonic::Runtime<::TFStoreTransactionOps, ::TFStoreTransaction>
{
public:
    TFStoreTransactionOps(
        const ::TF_RegistrationOps& registry_ops,
        ::TF_Registration* registry_handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry_ops, registry_handle, type.get_handle(), provider.get_handle())
    {
    }

    TFStoreTransactionOps(
        const ::TF_RegistrationOps& registry_ops,
        ::TF_Registration* registry_handle,
        ::TFStoreTransaction* handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry_ops, registry_handle, handle, type.get_handle(), provider.get_handle())
    {
    }

    explicit TFStoreTransactionOps(const ::TFStoreTransactionOps* ops) noexcept :
        Runtime(ops)
    {
    }

    TFStoreTransactionOps(const ::TFStoreTransactionOps* ops, ::TFStoreTransaction* handle) noexcept
        :
        Runtime(ops, handle)
    {
    }

    void create() const noexcept
    {
        m_ops->create(get_handle());
    }

    void destroy() const noexcept
    {
        m_ops->destroy(get_handle());
    }

    void begin(const ice::sonic::Status& out_status) const noexcept
    {
        m_ops->begin(get_handle(), out_status.get_handle());
    }

    void add_collection(
        const ice::sonic::TFStoreCollectionOps& collection,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->add_collection(get_handle(), collection.get_handle(), out_status.get_handle());
    }

    void get_collection(
        const ice::sonic::String& name,
        const ice::sonic::TFStoreCollectionOps& out_collection
    ) const noexcept
    {
        m_ops->get_collection(get_handle(), name.get_handle(), out_collection.get_handle());
    }

    void list_collections(
        TF_Tensor** out_collections,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->list_collections(get_handle(), out_collections, out_status.get_handle());
    }

    void commit(
        TFStoreAckFn completion,
        void* user_data,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->commit(get_handle(), completion, user_data, out_status.get_handle());
    }

    void rollback() const noexcept
    {
        m_ops->rollback(get_handle());
    }
};

} // namespace ice::sonic
