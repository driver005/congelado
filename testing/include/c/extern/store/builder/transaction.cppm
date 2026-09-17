// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/store/transaction.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/store/transaction.h"

export module cc_abi_builder_store;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;

export namespace ice::builder {

class TFStoreTransactionOps
{
public:
    static TFStoreTransactionOps* create(void* ctx) noexcept
    {
        return static_cast<TFStoreTransactionOps*>(ctx);
    }

    template<typename HandleT>
    static TFStoreTransactionOps* create(HandleT* handle) noexcept
    {
        return static_cast<TFStoreTransactionOps*>(handle->plugin_data);
    }

    virtual ~TFStoreTransactionOps() = default;
    [[nodiscard]] virtual std::expected<void, ice::Status> begin() noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    add_collection(const ice::sonic::TFStoreCollectionOps& collection) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> get_collection(
        const ice::sonic::TF_StringOps& name,
        const ice::sonic::TFStoreCollectionOps& out_collection
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    list_collections(TF_Tensor** out_collections) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    commit(TFStoreAckFn completion, void* user_data) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> rollback() noexcept = 0;

    static TFStoreTransactionOps* get_generic_vtable()
    {
        static TFStoreTransactionOps vtable = {
            .struct_size = TF_TORETRANSACTION_STRUCT_SIZE,

            .destroy =
                [](void* plugin_context) noexcept
            {
                delete TFStoreTransactionOps::create(plugin_context);
            },
            .begin =
                [](TFStoreTransaction* transaction, TF_Status* out_status) noexcept
            {
                auto* self = TFStoreTransactionOps::create(transaction);
                auto res = self->begin();
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .add_collection =
                [](TFStoreTransaction* transaction,
                   TFStoreCollection* collection,
                   TF_Status* out_status) noexcept
            {
                auto* self = TFStoreTransactionOps::create(transaction);
                auto res = self->add_collection(ice::sonic::TFStoreCollectionOps::wrap(collection));
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_collection =
                [](TFStoreTransaction* transaction,
                   const TF_String* name,
                   TFStoreCollection* out_collection) noexcept
            {
                auto* self = TFStoreTransactionOps::create(transaction);
                auto res = self->get_collection(
                    ice::sonic::TF_StringOps::wrap(name),
                    ice::sonic::TFStoreCollectionOps::wrap(out_collection)
                );
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .list_collections =
                [](TFStoreTransaction* transaction,
                   TF_Tensor** out_collections,
                   TF_Status* out_status) noexcept
            {
                auto* self = TFStoreTransactionOps::create(transaction);
                auto res = self->list_collections(out_collections);
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
                auto* self = TFStoreTransactionOps::create(transaction);
                auto res = self->commit(completion, user_data);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .rollback =
                [](TFStoreTransaction* transaction) noexcept
            {
                auto* self = TFStoreTransactionOps::create(transaction);
                auto res = self->rollback();
                if (!res) {
                    res.error().to_c(status);
                }
            },

        };

        return &vtable;
    }
};

} // namespace ice::builder
