// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/store/transaction.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/registration/registration.h"
#include "include/c/extern/store/collection.h"
#include "include/c/extern/store/transaction.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"

export module cc_ice_extern_store_builder:transaction;

import std;
import cc_ice_extern_store_sonic;
import cc_ice_intern_sonic;

export namespace ice::builder {

class TFStoreTransactionOps
{
public:
    explicit TFStoreTransactionOps(
        const ::TFStoreCollectionOps* TFStoreCollectionOps_ops,
        const ::TF_StatusOps* Status_ops,
        const ::TF_StringOps* String_ops
    ) noexcept :
        m_handle{.plugin_data = this}
    {
        m_TFStoreCollectionOps_ops = TFStoreCollectionOps_ops;
        m_Status_ops = Status_ops;
        m_String_ops = String_ops;
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
    virtual void begin(const ice::sonic::Status& out_status) noexcept = 0;
    virtual void add_collection(
        const ice::sonic::TFStoreCollectionOps& collection,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void get_collection(
        const ice::sonic::String& name,
        const ice::sonic::TFStoreCollectionOps& out_collection
    ) noexcept = 0;
    virtual void list_collections(
        TF_Tensor** out_collections,
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
                [](TFStoreTransaction* handle) noexcept
            {
                auto& self = TFStoreTransactionOps::from_handle(handle);
                self.destroy();
            },
            .begin =
                [](TFStoreTransaction* transaction, TF_Status* out_status) noexcept
            {
                auto& self = TFStoreTransactionOps::from_handle(transaction);
                self.begin(self.wrap(std::type_identity<ice::sonic::Status>{}, out_status));
            },
            .add_collection =
                [](TFStoreTransaction* transaction,
                   TFStoreCollection* collection,
                   TF_Status* out_status) noexcept
            {
                auto& self = TFStoreTransactionOps::from_handle(transaction);
                self.add_collection(
                    self.wrap(std::type_identity<ice::sonic::TFStoreCollectionOps>{}, collection),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_collection =
                [](TFStoreTransaction* transaction,
                   const TF_String* name,
                   TFStoreCollection* out_collection) noexcept
            {
                auto& self = TFStoreTransactionOps::from_handle(transaction);
                self.get_collection(
                    self.wrap(std::type_identity<ice::sonic::String>{}, name),
                    self.wrap(
                        std::type_identity<ice::sonic::TFStoreCollectionOps>{},
                        out_collection
                    )
                );
            },
            .list_collections =
                [](TFStoreTransaction* transaction,
                   TF_Tensor** out_collections,
                   TF_Status* out_status) noexcept
            {
                auto& self = TFStoreTransactionOps::from_handle(transaction);
                self.list_collections(
                    out_collections,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .commit =
                [](TFStoreTransaction* transaction,
                   TFStoreAckFn completion,
                   void* user_data,
                   TF_Status* out_status) noexcept
            {
                auto& self = TFStoreTransactionOps::from_handle(transaction);
                self.commit(
                    completion,
                    user_data,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .rollback =
                [](TFStoreTransaction* transaction) noexcept
            {
                auto& self = TFStoreTransactionOps::from_handle(transaction);
                self.rollback();
            },

        };
    }

    ice::sonic::TFStoreCollectionOps wrap(
        std::type_identity<ice::sonic::TFStoreCollectionOps>,
        const ::TFStoreCollection* handle
    ) const noexcept
    {
        return ice::sonic::TFStoreCollectionOps{
            m_TFStoreCollectionOps_ops,
            const_cast<::TFStoreCollection*>(handle)
        };
    }

    ice::sonic::Status
    wrap(std::type_identity<ice::sonic::Status>, const ::TF_Status* handle) const noexcept
    {
        return ice::sonic::Status{m_Status_ops, const_cast<::TF_Status*>(handle)};
    }

    ice::sonic::String
    wrap(std::type_identity<ice::sonic::String>, const ::TF_String* handle) const noexcept
    {
        return ice::sonic::String{m_String_ops, const_cast<::TF_String*>(handle)};
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
        const ::TF_RegistrationOps& registry_ops,
        ::TF_Registration* registry_handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) const noexcept
    {
        registry_ops.register_op(
            registry_handle,
            type.get_handle(),
            provider.get_handle(),
            const_cast<::TFStoreTransactionOps*>(&m_vtable)
        );
    }

private:
    ::TFStoreTransactionOps m_vtable;
    ::TFStoreTransaction m_handle;

    const ::TFStoreCollectionOps* m_TFStoreCollectionOps_ops{nullptr};

    const ::TF_StatusOps* m_Status_ops{nullptr};

    const ::TF_StringOps* m_String_ops{nullptr};
};

} // namespace ice::builder
