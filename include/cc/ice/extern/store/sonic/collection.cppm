// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/store/collection.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/store/collection.h"

export module cc_ice_extern_store_sonic:collection;

import std;
import cc_abi_sonic_registration;

export namespace ice::sonic {

class TFStoreCollectionOps : public ice::sonic::Runtime<TFStoreCollectionOps, TFStoreCollectionOps>
{
public:
    explicit TFStoreCollectionOps(TFStoreCollectionOps* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "store";

    void destroy() noexcept
    {
        m_ops->destroy(get_handle());
    }

    void close() noexcept
    {
        m_ops->close(get_handle());
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
    get_stats(const ice::sonic::TF_MapOps& out_stats) noexcept
    {
        ice::sonic::Status status;
        m_ops->get_stats(get_handle(), out_stats.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    get(const ice::sonic::String& key, TFStoreGetCompletionFn completion, void* user_data) noexcept
    {
        ice::sonic::Status status;
        m_ops->get(get_handle(), key.get_handle(), completion, user_data, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status> multi_get(
        const ice::sonic::TF_VectorOps& keys,
        TFStoreMultiGetCompletionFn completion,
        void* user_data
    ) noexcept
    {
        ice::sonic::Status status;
        m_ops->multi_get(
            get_handle(),
            keys.get_handle(),
            completion,
            user_data,
            status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    set(const ice::sonic::String& key,
        const ice::sonic::String& value,
        int64_t ttl_seconds,
        TFStoreSetCompletionFn completion,
        void* user_data) noexcept
    {
        ice::sonic::Status status;
        m_ops->set(
            get_handle(),
            key.get_handle(),
            value.get_handle(),
            ttl_seconds,
            completion,
            user_data,
            status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status> multi_set(
        const ice::sonic::TF_MapOps& entries,
        int64_t ttl_seconds,
        TFStoreAckFn completion,
        void* user_data
    ) noexcept
    {
        ice::sonic::Status status;
        m_ops->multi_set(
            get_handle(),
            entries.get_handle(),
            ttl_seconds,
            completion,
            user_data,
            status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    erase(const ice::sonic::String& key, TFStoreAckFn completion, void* user_data) noexcept
    {
        ice::sonic::Status status;
        m_ops->erase(get_handle(), key.get_handle(), completion, user_data, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status> multi_erase(
        const ice::sonic::TF_VectorOps& keys,
        TFStoreAckFn completion,
        void* user_data
    ) noexcept
    {
        ice::sonic::Status status;
        m_ops->multi_erase(
            get_handle(),
            keys.get_handle(),
            completion,
            user_data,
            status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    exists(const ice::sonic::String& key, TFStoreExistsFn completion, void* user_data) noexcept
    {
        ice::sonic::Status status;
        m_ops->exists(get_handle(), key.get_handle(), completion, user_data, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status> rename(
        const ice::sonic::String& old_key,
        const ice::sonic::String& new_key,
        TFStoreAckFn completion,
        void* user_data
    ) noexcept
    {
        ice::sonic::Status status;
        m_ops->rename(
            get_handle(),
            old_key.get_handle(),
            new_key.get_handle(),
            completion,
            user_data,
            status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    clear(TFStoreAckFn completion, void* user_data) noexcept
    {
        ice::sonic::Status status;
        m_ops->clear(get_handle(), completion, user_data, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status> increment(
        const ice::sonic::String& key,
        int64_t delta,
        TFStoreIntFn completion,
        void* user_data
    ) noexcept
    {
        ice::sonic::Status status;
        m_ops->increment(
            get_handle(),
            key.get_handle(),
            delta,
            completion,
            user_data,
            status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status> compare_and_swap(
        const ice::sonic::String& key,
        const ice::sonic::String& expected_value,
        const ice::sonic::String& new_value,
        TFStoreBoolFn completion,
        void* user_data
    ) noexcept
    {
        ice::sonic::Status status;
        m_ops->compare_and_swap(
            get_handle(),
            key.get_handle(),
            expected_value.get_handle(),
            new_value.get_handle(),
            completion,
            user_data,
            status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status> expire(
        const ice::sonic::String& key,
        int64_t ttl_seconds,
        TFStoreAckFn completion,
        void* user_data
    ) noexcept
    {
        ice::sonic::Status status;
        m_ops->expire(
            get_handle(),
            key.get_handle(),
            ttl_seconds,
            completion,
            user_data,
            status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    get_ttl(const ice::sonic::String& key, TFStoreIntFn completion, void* user_data) noexcept
    {
        ice::sonic::Status status;
        m_ops->get_ttl(get_handle(), key.get_handle(), completion, user_data, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    persist(const ice::sonic::String& key, TFStoreAckFn completion, void* user_data) noexcept
    {
        ice::sonic::Status status;
        m_ops->persist(get_handle(), key.get_handle(), completion, user_data, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }
};

} // namespace ice::sonic
