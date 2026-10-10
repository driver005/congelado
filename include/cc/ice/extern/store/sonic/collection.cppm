// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/store/collection.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/registration/registration.h"
#include "include/c/extern/store/collection.h"

export module cc_ice_extern_store_sonic:collection;

import std;
import cc_ice_intern_sonic;

export namespace ice::sonic {

class TFStoreCollectionOps : public ice::sonic::Runtime<::TFStoreCollectionOps, ::TFStoreCollection>
{
public:
    TFStoreCollectionOps(
        const ::TF_RegistrationOps& registry_ops,
        ::TF_Registration* registry_handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry_ops, registry_handle, type.get_handle(), provider.get_handle())
    {
    }

    TFStoreCollectionOps(
        const ::TF_RegistrationOps& registry_ops,
        ::TF_Registration* registry_handle,
        ::TFStoreCollection* handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry_ops, registry_handle, handle, type.get_handle(), provider.get_handle())
    {
    }

    explicit TFStoreCollectionOps(const ::TFStoreCollectionOps* ops) noexcept :
        Runtime(ops)
    {
    }

    TFStoreCollectionOps(const ::TFStoreCollectionOps* ops, ::TFStoreCollection* handle) noexcept :
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

    void close() const noexcept
    {
        m_ops->close(get_handle());
    }

    void list(
        const ice::sonic::TF_VectorOps& out_names,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->list(get_handle(), out_names.get_handle(), out_status.get_handle());
    }

    void drop(const ice::sonic::String& name, const ice::sonic::Status& out_status) const noexcept
    {
        m_ops->drop(get_handle(), name.get_handle(), out_status.get_handle());
    }

    void get_stats(
        const ice::sonic::TF_MapOps& out_stats,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->get_stats(get_handle(), out_stats.get_handle(), out_status.get_handle());
    }

    void get(
        const ice::sonic::String& key,
        TFStoreGetCompletionFn completion,
        void* user_data,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->get(get_handle(), key.get_handle(), completion, user_data, out_status.get_handle());
    }

    void multi_get(
        const ice::sonic::TF_VectorOps& keys,
        TFStoreMultiGetCompletionFn completion,
        void* user_data,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->multi_get(
            get_handle(),
            keys.get_handle(),
            completion,
            user_data,
            out_status.get_handle()
        );
    }

    void set(
        const ice::sonic::String& key,
        const ice::sonic::String& value,
        int64_t ttl_seconds,
        TFStoreSetCompletionFn completion,
        void* user_data,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->set(
            get_handle(),
            key.get_handle(),
            value.get_handle(),
            ttl_seconds,
            completion,
            user_data,
            out_status.get_handle()
        );
    }

    void multi_set(
        const ice::sonic::TF_MapOps& entries,
        int64_t ttl_seconds,
        TFStoreAckFn completion,
        void* user_data,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->multi_set(
            get_handle(),
            entries.get_handle(),
            ttl_seconds,
            completion,
            user_data,
            out_status.get_handle()
        );
    }

    void erase(
        const ice::sonic::String& key,
        TFStoreAckFn completion,
        void* user_data,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops
            ->erase(get_handle(), key.get_handle(), completion, user_data, out_status.get_handle());
    }

    void multi_erase(
        const ice::sonic::TF_VectorOps& keys,
        TFStoreAckFn completion,
        void* user_data,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->multi_erase(
            get_handle(),
            keys.get_handle(),
            completion,
            user_data,
            out_status.get_handle()
        );
    }

    void exists(
        const ice::sonic::String& key,
        TFStoreExistsFn completion,
        void* user_data,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->exists(
            get_handle(),
            key.get_handle(),
            completion,
            user_data,
            out_status.get_handle()
        );
    }

    void rename(
        const ice::sonic::String& old_key,
        const ice::sonic::String& new_key,
        TFStoreAckFn completion,
        void* user_data,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->rename(
            get_handle(),
            old_key.get_handle(),
            new_key.get_handle(),
            completion,
            user_data,
            out_status.get_handle()
        );
    }

    void clear(
        TFStoreAckFn completion,
        void* user_data,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->clear(get_handle(), completion, user_data, out_status.get_handle());
    }

    void increment(
        const ice::sonic::String& key,
        int64_t delta,
        TFStoreIntFn completion,
        void* user_data,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->increment(
            get_handle(),
            key.get_handle(),
            delta,
            completion,
            user_data,
            out_status.get_handle()
        );
    }

    void compare_and_swap(
        const ice::sonic::String& key,
        const ice::sonic::String& expected_value,
        const ice::sonic::String& new_value,
        TFStoreBoolFn completion,
        void* user_data,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->compare_and_swap(
            get_handle(),
            key.get_handle(),
            expected_value.get_handle(),
            new_value.get_handle(),
            completion,
            user_data,
            out_status.get_handle()
        );
    }

    void expire(
        const ice::sonic::String& key,
        int64_t ttl_seconds,
        TFStoreAckFn completion,
        void* user_data,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->expire(
            get_handle(),
            key.get_handle(),
            ttl_seconds,
            completion,
            user_data,
            out_status.get_handle()
        );
    }

    void get_ttl(
        const ice::sonic::String& key,
        TFStoreIntFn completion,
        void* user_data,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->get_ttl(
            get_handle(),
            key.get_handle(),
            completion,
            user_data,
            out_status.get_handle()
        );
    }

    void persist(
        const ice::sonic::String& key,
        TFStoreAckFn completion,
        void* user_data,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->persist(
            get_handle(),
            key.get_handle(),
            completion,
            user_data,
            out_status.get_handle()
        );
    }
};

} // namespace ice::sonic
