// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/store/collection.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/registration/registration.h"
#include "include/c/extern/store/collection.h"
#include "include/c/intern/map.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"
#include "include/c/intern/vector.h"

export module cc_ice_extern_store_builder:collection;

import std;
import cc_ice_intern_sonic;

export namespace ice::builder {

class TFStoreCollectionOps
{
public:
    explicit TFStoreCollectionOps(
        const ::TF_MapOps* TF_MapOps_ops,
        const ::TF_StatusOps* Status_ops,
        const ::TF_StringOps* String_ops,
        const ::TF_VectorOps* TF_VectorOps_ops
    ) noexcept :
        m_handle{.plugin_data = this}
    {
        m_TF_MapOps_ops = TF_MapOps_ops;
        m_Status_ops = Status_ops;
        m_String_ops = String_ops;
        m_TF_VectorOps_ops = TF_VectorOps_ops;
    }

    TFStoreCollectionOps(const TFStoreCollectionOps&) = delete;
    TFStoreCollectionOps& operator=(const TFStoreCollectionOps&) = delete;

    static TFStoreCollectionOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TFStoreCollectionOps*>(ctx);
    }

    template<typename HandleT>
    static TFStoreCollectionOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TFStoreCollectionOps*>(handle->plugin_data);
    }

    virtual ~TFStoreCollectionOps() = default;
    virtual void destroy() noexcept = 0;
    virtual void close() noexcept = 0;
    virtual void list(
        const ice::sonic::TF_VectorOps& out_names,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void
    drop(const ice::sonic::String& name, const ice::sonic::Status& out_status) noexcept = 0;
    virtual void get_stats(
        const ice::sonic::TF_MapOps& out_stats,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void
    get(const ice::sonic::String& key,
        TFStoreGetCompletionFn completion,
        void* user_data,
        const ice::sonic::Status& out_status) noexcept = 0;
    virtual void multi_get(
        const ice::sonic::TF_VectorOps& keys,
        TFStoreMultiGetCompletionFn completion,
        void* user_data,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void
    set(const ice::sonic::String& key,
        const ice::sonic::String& value,
        int64_t ttl_seconds,
        TFStoreSetCompletionFn completion,
        void* user_data,
        const ice::sonic::Status& out_status) noexcept = 0;
    virtual void multi_set(
        const ice::sonic::TF_MapOps& entries,
        int64_t ttl_seconds,
        TFStoreAckFn completion,
        void* user_data,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void erase(
        const ice::sonic::String& key,
        TFStoreAckFn completion,
        void* user_data,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void multi_erase(
        const ice::sonic::TF_VectorOps& keys,
        TFStoreAckFn completion,
        void* user_data,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void exists(
        const ice::sonic::String& key,
        TFStoreExistsFn completion,
        void* user_data,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void rename(
        const ice::sonic::String& old_key,
        const ice::sonic::String& new_key,
        TFStoreAckFn completion,
        void* user_data,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void clear(
        TFStoreAckFn completion,
        void* user_data,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void increment(
        const ice::sonic::String& key,
        int64_t delta,
        TFStoreIntFn completion,
        void* user_data,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void compare_and_swap(
        const ice::sonic::String& key,
        const ice::sonic::String& expected_value,
        const ice::sonic::String& new_value,
        TFStoreBoolFn completion,
        void* user_data,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void expire(
        const ice::sonic::String& key,
        int64_t ttl_seconds,
        TFStoreAckFn completion,
        void* user_data,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void get_ttl(
        const ice::sonic::String& key,
        TFStoreIntFn completion,
        void* user_data,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void persist(
        const ice::sonic::String& key,
        TFStoreAckFn completion,
        void* user_data,
        const ice::sonic::Status& out_status
    ) noexcept = 0;

    void get_generic_vtable(void (*create)(::TFStoreCollection*)) noexcept
    {
        m_vtable = ::TFStoreCollectionOps{
            .struct_size = TF_OFFSET_OF_END(::TFStoreCollectionOps, persist),

            .create = create,
            .destroy =
                [](TFStoreCollection* handle) noexcept
            {
                auto& self = TFStoreCollectionOps::from_handle(handle);
                self.destroy();
            },
            .close =
                [](TFStoreCollection* collection) noexcept
            {
                auto& self = TFStoreCollectionOps::from_handle(collection);
                self.close();
            },
            .list =
                [](TFStoreCollection* store, TF_Vector* out_names, TF_Status* out_status) noexcept
            {
                auto& self = TFStoreCollectionOps::from_handle(store);
                self.list(
                    self.wrap(std::type_identity<ice::sonic::TF_VectorOps>{}, out_names),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .drop =
                [](TFStoreCollection* store, const TF_String* name, TF_Status* out_status) noexcept
            {
                auto& self = TFStoreCollectionOps::from_handle(store);
                self.drop(
                    self.wrap(std::type_identity<ice::sonic::String>{}, name),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_stats =
                [](TFStoreCollection* collection, TF_Map* out_stats, TF_Status* out_status) noexcept
            {
                auto& self = TFStoreCollectionOps::from_handle(collection);
                self.get_stats(
                    self.wrap(std::type_identity<ice::sonic::TF_MapOps>{}, out_stats),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get =
                [](TFStoreCollection* collection,
                   const TF_String* key,
                   TFStoreGetCompletionFn completion,
                   void* user_data,
                   TF_Status* out_status) noexcept
            {
                auto& self = TFStoreCollectionOps::from_handle(collection);
                self.get(
                    self.wrap(std::type_identity<ice::sonic::String>{}, key),
                    completion,
                    user_data,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .multi_get =
                [](TFStoreCollection* collection,
                   const TF_Vector* keys,
                   TFStoreMultiGetCompletionFn completion,
                   void* user_data,
                   TF_Status* out_status) noexcept
            {
                auto& self = TFStoreCollectionOps::from_handle(collection);
                self.multi_get(
                    self.wrap(std::type_identity<ice::sonic::TF_VectorOps>{}, keys),
                    completion,
                    user_data,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .set =
                [](TFStoreCollection* collection,
                   const TF_String* key,
                   const TF_String* value,
                   int64_t ttl_seconds,
                   TFStoreSetCompletionFn completion,
                   void* user_data,
                   TF_Status* out_status) noexcept
            {
                auto& self = TFStoreCollectionOps::from_handle(collection);
                self.set(
                    self.wrap(std::type_identity<ice::sonic::String>{}, key),
                    self.wrap(std::type_identity<ice::sonic::String>{}, value),
                    ttl_seconds,
                    completion,
                    user_data,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .multi_set =
                [](TFStoreCollection* collection,
                   const TF_Map* entries,
                   int64_t ttl_seconds,
                   TFStoreAckFn completion,
                   void* user_data,
                   TF_Status* out_status) noexcept
            {
                auto& self = TFStoreCollectionOps::from_handle(collection);
                self.multi_set(
                    self.wrap(std::type_identity<ice::sonic::TF_MapOps>{}, entries),
                    ttl_seconds,
                    completion,
                    user_data,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .erase =
                [](TFStoreCollection* collection,
                   const TF_String* key,
                   TFStoreAckFn completion,
                   void* user_data,
                   TF_Status* out_status) noexcept
            {
                auto& self = TFStoreCollectionOps::from_handle(collection);
                self.erase(
                    self.wrap(std::type_identity<ice::sonic::String>{}, key),
                    completion,
                    user_data,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .multi_erase =
                [](TFStoreCollection* collection,
                   const TF_Vector* keys,
                   TFStoreAckFn completion,
                   void* user_data,
                   TF_Status* out_status) noexcept
            {
                auto& self = TFStoreCollectionOps::from_handle(collection);
                self.multi_erase(
                    self.wrap(std::type_identity<ice::sonic::TF_VectorOps>{}, keys),
                    completion,
                    user_data,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .exists =
                [](TFStoreCollection* collection,
                   const TF_String* key,
                   TFStoreExistsFn completion,
                   void* user_data,
                   TF_Status* out_status) noexcept
            {
                auto& self = TFStoreCollectionOps::from_handle(collection);
                self.exists(
                    self.wrap(std::type_identity<ice::sonic::String>{}, key),
                    completion,
                    user_data,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .rename =
                [](TFStoreCollection* collection,
                   const TF_String* old_key,
                   const TF_String* new_key,
                   TFStoreAckFn completion,
                   void* user_data,
                   TF_Status* out_status) noexcept
            {
                auto& self = TFStoreCollectionOps::from_handle(collection);
                self.rename(
                    self.wrap(std::type_identity<ice::sonic::String>{}, old_key),
                    self.wrap(std::type_identity<ice::sonic::String>{}, new_key),
                    completion,
                    user_data,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .clear =
                [](TFStoreCollection* collection,
                   TFStoreAckFn completion,
                   void* user_data,
                   TF_Status* out_status) noexcept
            {
                auto& self = TFStoreCollectionOps::from_handle(collection);
                self.clear(
                    completion,
                    user_data,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .increment =
                [](TFStoreCollection* collection,
                   const TF_String* key,
                   int64_t delta,
                   TFStoreIntFn completion,
                   void* user_data,
                   TF_Status* out_status) noexcept
            {
                auto& self = TFStoreCollectionOps::from_handle(collection);
                self.increment(
                    self.wrap(std::type_identity<ice::sonic::String>{}, key),
                    delta,
                    completion,
                    user_data,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .compare_and_swap =
                [](TFStoreCollection* collection,
                   const TF_String* key,
                   const TF_String* expected_value,
                   const TF_String* new_value,
                   TFStoreBoolFn completion,
                   void* user_data,
                   TF_Status* out_status) noexcept
            {
                auto& self = TFStoreCollectionOps::from_handle(collection);
                self.compare_and_swap(
                    self.wrap(std::type_identity<ice::sonic::String>{}, key),
                    self.wrap(std::type_identity<ice::sonic::String>{}, expected_value),
                    self.wrap(std::type_identity<ice::sonic::String>{}, new_value),
                    completion,
                    user_data,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .expire =
                [](TFStoreCollection* collection,
                   const TF_String* key,
                   int64_t ttl_seconds,
                   TFStoreAckFn completion,
                   void* user_data,
                   TF_Status* out_status) noexcept
            {
                auto& self = TFStoreCollectionOps::from_handle(collection);
                self.expire(
                    self.wrap(std::type_identity<ice::sonic::String>{}, key),
                    ttl_seconds,
                    completion,
                    user_data,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_ttl =
                [](TFStoreCollection* collection,
                   const TF_String* key,
                   TFStoreIntFn completion,
                   void* user_data,
                   TF_Status* out_status) noexcept
            {
                auto& self = TFStoreCollectionOps::from_handle(collection);
                self.get_ttl(
                    self.wrap(std::type_identity<ice::sonic::String>{}, key),
                    completion,
                    user_data,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .persist =
                [](TFStoreCollection* collection,
                   const TF_String* key,
                   TFStoreAckFn completion,
                   void* user_data,
                   TF_Status* out_status) noexcept
            {
                auto& self = TFStoreCollectionOps::from_handle(collection);
                self.persist(
                    self.wrap(std::type_identity<ice::sonic::String>{}, key),
                    completion,
                    user_data,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },

        };
    }

    ice::sonic::TF_MapOps
    wrap(std::type_identity<ice::sonic::TF_MapOps>, const ::TF_Map* handle) const noexcept
    {
        return ice::sonic::TF_MapOps{m_TF_MapOps_ops, const_cast<::TF_Map*>(handle)};
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

    ice::sonic::TF_VectorOps
    wrap(std::type_identity<ice::sonic::TF_VectorOps>, const ::TF_Vector* handle) const noexcept
    {
        return ice::sonic::TF_VectorOps{m_TF_VectorOps_ops, const_cast<::TF_Vector*>(handle)};
    }

    const ::TFStoreCollectionOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const ::TFStoreCollection& get_handle() const noexcept
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
            const_cast<::TFStoreCollectionOps*>(&m_vtable)
        );
    }

private:
    ::TFStoreCollectionOps m_vtable;
    ::TFStoreCollection m_handle;

    const ::TF_MapOps* m_TF_MapOps_ops{nullptr};

    const ::TF_StatusOps* m_Status_ops{nullptr};

    const ::TF_StringOps* m_String_ops{nullptr};

    const ::TF_VectorOps* m_TF_VectorOps_ops{nullptr};
};

} // namespace ice::builder
