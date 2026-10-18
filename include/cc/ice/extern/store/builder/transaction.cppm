// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/store/transaction.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/store/collection.h"
#include "include/c/extern/store/transaction.h"
#include "include/c/intern/registration.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"

export module cc_ice_extern_store_builder:transaction;

import std;
import cc_ice_extern_store_sonic;
import cc_ice_intern_sonic;

export namespace ice::builder {

class StoreTransaction
{
public:
    explicit StoreTransaction(
        const ::TFStoreCollectionOps* StoreCollection_ops,
        const ::TF_StatusOps* Status_ops,
        const ::TF_StringOps* String_ops
    ) noexcept :
        m_handle{.plugin_data = this}
    {
        m_StoreCollection_ops = StoreCollection_ops;
        m_Status_ops = Status_ops;
        m_String_ops = String_ops;
    }

    StoreTransaction(const StoreTransaction&) = delete;
    StoreTransaction& operator=(const StoreTransaction&) = delete;

    static StoreTransaction& from_handle(void* ctx) noexcept
    {
        return *static_cast<StoreTransaction*>(ctx);
    }

    template<typename HandleT>
    static StoreTransaction& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<StoreTransaction*>(handle->plugin_data);
    }

    virtual ~StoreTransaction() = default;
    virtual void destroy() noexcept = 0;
    virtual void begin(const ice::sonic::Status& out_status) noexcept = 0;
    virtual void add_collection(
        const ice::sonic::StoreCollection& collection,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void get_collection(
        const ice::sonic::String& name,
        const ice::sonic::StoreCollection& out_collection
    ) noexcept = 0;
    virtual void list_collections(
        ::TF_Tensor** out_collections,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void commit(
        TFStoreAckFn completion,
        void* user_data,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void rollback() noexcept = 0;

    void get_generic_vtable(void (*create)(::TFStoreTransaction*)) noexcept
    {
        m_vtable = ::TFStoreTransactionOps{
            .struct_size = TF_OFFSET_OF_END(::TFStoreTransactionOps, rollback),

            .create = create,
            .destroy =
                [](::TFStoreTransaction* handle) noexcept
            {
                auto& self = StoreTransaction::from_handle(handle);
                self.destroy();
            },
            .begin =
                [](::TFStoreTransaction* transaction, ::TF_Status* out_status) noexcept
            {
                auto& self = StoreTransaction::from_handle(transaction);
                self.begin(self.wrap(std::type_identity<ice::sonic::Status>{}, out_status));
            },
            .add_collection =
                [](::TFStoreTransaction* transaction,
                   ::TFStoreCollection* collection,
                   ::TF_Status* out_status) noexcept
            {
                auto& self = StoreTransaction::from_handle(transaction);
                self.add_collection(
                    self.wrap(std::type_identity<ice::sonic::StoreCollection>{}, collection),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_collection =
                [](::TFStoreTransaction* transaction,
                   const ::TF_String* name,
                   ::TFStoreCollection* out_collection) noexcept
            {
                auto& self = StoreTransaction::from_handle(transaction);
                self.get_collection(
                    self.wrap(std::type_identity<ice::sonic::String>{}, name),
                    self.wrap(std::type_identity<ice::sonic::StoreCollection>{}, out_collection)
                );
            },
            .list_collections =
                [](::TFStoreTransaction* transaction,
                   ::TF_Tensor** out_collections,
                   ::TF_Status* out_status) noexcept
            {
                auto& self = StoreTransaction::from_handle(transaction);
                self.list_collections(
                    out_collections,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .commit =
                [](::TFStoreTransaction* transaction,
                   TFStoreAckFn completion,
                   void* user_data,
                   ::TF_Status* out_status) noexcept
            {
                auto& self = StoreTransaction::from_handle(transaction);
                self.commit(
                    completion,
                    user_data,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .rollback =
                [](::TFStoreTransaction* transaction) noexcept
            {
                auto& self = StoreTransaction::from_handle(transaction);
                self.rollback();
            },

        };
    }

    ice::sonic::StoreCollection wrap(
        std::type_identity<ice::sonic::StoreCollection>,
        const ::TFStoreCollection* handle
    ) const noexcept
    {
        return ice::sonic::StoreCollection{m_StoreCollection_ops, handle};
    }

    ice::sonic::Status
    wrap(std::type_identity<ice::sonic::Status>, const ::TF_Status* handle) const noexcept
    {
        return ice::sonic::Status{m_Status_ops, handle};
    }

    ice::sonic::String
    wrap(std::type_identity<ice::sonic::String>, const ::TF_String* handle) const noexcept
    {
        return ice::sonic::String{m_String_ops, handle};
    }

    const ::TFStoreTransactionOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const ::TFStoreTransaction& get_handle() const noexcept
    {
        return m_handle;
    }

    void register_ops(
        const ice::sonic::Registration& registry,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) const noexcept
    {
        registry.register_op(type, provider, const_cast<::TFStoreTransactionOps*>(&m_vtable));
    }

private:
    ::TFStoreTransactionOps m_vtable;
    ::TFStoreTransaction m_handle;

    const ::TFStoreCollectionOps* m_StoreCollection_ops{nullptr};

    const ::TF_StatusOps* m_Status_ops{nullptr};

    const ::TF_StringOps* m_String_ops{nullptr};
};

} // namespace ice::builder
