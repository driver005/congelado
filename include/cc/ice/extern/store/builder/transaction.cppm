// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/store/transaction.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/store/transaction.h"

export module cc_ice_extern_store_builder:transaction;

import std;

export namespace ice::builder {

class TFStoreTransactionOps
{
public:
    TFStoreTransactionOps() noexcept :
        m_handle{.plugin_data = this}
    {
    }

    TFStoreTransactionOps(const TFStoreTransactionOps&) = delete;
    TFStoreTransactionOps& operator=(const TFStoreTransactionOps&) = delete;

    static TFStoreTransactionOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TFStoreTransactionOps*>(ctx);
    }

    template<typename HandleT>
    static TFStoreTransactionOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TFStoreTransactionOps*>(handle->plugin_data);
    }

    virtual ~TFStoreTransactionOps() = default;
    virtual void destroy() noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status> begin() noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    add_collection(const ice::sonic::TFStoreCollectionOps& collection) noexcept = 0;
    virtual void get_collection(
        const ice::sonic::String& name,
        const ice::sonic::TFStoreCollectionOps& out_collection
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    list_collections(TF_Tensor** out_collections) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    commit(TFStoreAckFn completion, void* user_data) noexcept = 0;
    virtual void rollback() noexcept = 0;

    void get_generic_vtable() noexcept
    {
        m_vtable = ::TFStoreTransactionOps{
            .struct_size = TF_TORETRANSACTION_STRUCT_SIZE,
            .destroy =
                [](TFStoreTransaction* transaction) noexcept
            {
                TFStoreTransactionOps::from_handle(transaction).destroy();
            },
            .begin =
                [](TFStoreTransaction* transaction, TF_Status* out_status) noexcept
            {
                auto res = TFStoreTransactionOps::from_handle(transaction).begin();
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .add_collection =
                [](TFStoreTransaction* transaction,
                   TFStoreCollection* collection,
                   TF_Status* out_status) noexcept
            {
                auto res = TFStoreTransactionOps::from_handle(transaction)
                               .add_collection(ice::sonic::TFStoreCollectionOps::wrap(collection));
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_collection =
                [](TFStoreTransaction* transaction,
                   const TF_String* name,
                   TFStoreCollection* out_collection) noexcept
            {
                TFStoreTransactionOps::from_handle(transaction)
                    .get_collection(
                        ice::sonic::String::wrap(name),
                        ice::sonic::TFStoreCollectionOps::wrap(out_collection)
                    );
            },
            .list_collections =
                [](TFStoreTransaction* transaction,
                   TF_Tensor** out_collections,
                   TF_Status* out_status) noexcept
            {
                auto res = TFStoreTransactionOps::from_handle(transaction)
                               .list_collections(out_collections);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .commit =
                [](TFStoreTransaction* transaction,
                   TFStoreAckFn completion,
                   void* user_data,
                   TF_Status* out_status) noexcept
            {
                auto res =
                    TFStoreTransactionOps::from_handle(transaction).commit(completion, user_data);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .rollback =
                [](TFStoreTransaction* transaction) noexcept
            {
                TFStoreTransactionOps::from_handle(transaction).rollback();
            },

        };
    }

    const ::TFStoreTransactionOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const TFStoreTransaction& get_handle() const noexcept
    {
        return m_handle;
    }


private:
    ::TFStoreTransactionOps m_vtable;
    TFStoreTransaction m_handle;
};

} // namespace ice::builder
