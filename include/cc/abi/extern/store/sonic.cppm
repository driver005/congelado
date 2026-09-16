// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from c/extern/store/store.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "c/extern/store/store.h"

export module cc_abi_sonic_store;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;
import cc_abi_sonic_registration;

export namespace ice::sonic {

class Store : public ice::sonic::Runtime<Store, TF_StoreOps>
{
public:
    explicit Store(TF_StoreOps* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "store";

    [[nodiscard]] std::expected<void, ice::Status> is_connected() noexcept
    {
        ice::Status status;
        m_ops->is_connected(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    open_collection(const ice::sonic::String& name) noexcept
    {
        ice::Status status;
        m_ops->open_collection(get_handle(), name.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> close_collection() noexcept
    {
        ice::Status status;
        m_ops->close_collection(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    list_collections(const ice::sonic::Vector& out_names) noexcept
    {
        ice::Status status;
        m_ops->list_collections(get_handle(), out_names.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    drop_collection(const ice::sonic::String& name) noexcept
    {
        ice::Status status;
        m_ops->drop_collection(get_handle(), name.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    get_collection_stats(const ice::sonic::Map& out_stats) noexcept
    {
        ice::Status status;
        m_ops->get_collection_stats(get_handle(), out_stats.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    get(const ice::sonic::String& key,
        TF_Store_GetCompletionFn completion,
        void* user_data) noexcept
    {
        ice::Status status;
        m_ops->get(get_handle(), key.get_handle(), completion, user_data, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> multi_get(
        const ice::sonic::Vector& keys,
        TF_Store_MultiGetCompletionFn completion,
        void* user_data
    ) noexcept
    {
        ice::Status status;
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

    [[nodiscard]] std::expected<void, ice::Status>
    set(const ice::sonic::String& key,
        const ice::sonic::String& value,
        int64_t ttl_seconds,
        TF_Store_SetCompletionFn completion,
        void* user_data) noexcept
    {
        ice::Status status;
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

    [[nodiscard]] std::expected<void, ice::Status> multi_set(
        const ice::sonic::Map& entries,
        int64_t ttl_seconds,
        TF_Store_AckFn completion,
        void* user_data
    ) noexcept
    {
        ice::Status status;
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

    [[nodiscard]] std::expected<void, ice::Status>
    remove(const ice::sonic::String& key, TF_Store_AckFn completion, void* user_data) noexcept
    {
        ice::Status status;
        m_ops->remove(get_handle(), key.get_handle(), completion, user_data, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> multi_remove(
        const ice::sonic::Vector& keys,
        TF_Store_AckFn completion,
        void* user_data
    ) noexcept
    {
        ice::Status status;
        m_ops->multi_remove(
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

    [[nodiscard]] std::expected<void, ice::Status>
    exists(const ice::sonic::String& key, TF_Store_ExistsFn completion, void* user_data) noexcept
    {
        ice::Status status;
        m_ops->exists(get_handle(), key.get_handle(), completion, user_data, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> rename(
        const ice::sonic::String& old_key,
        const ice::sonic::String& new_key,
        TF_Store_AckFn completion,
        void* user_data
    ) noexcept
    {
        ice::Status status;
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

    [[nodiscard]] std::expected<void, ice::Status>
    clear(TF_Store_AckFn completion, void* user_data) noexcept
    {
        ice::Status status;
        m_ops->clear(get_handle(), completion, user_data, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> increment(
        const ice::sonic::String& key,
        int64_t delta,
        TF_Store_IntFn completion,
        void* user_data
    ) noexcept
    {
        ice::Status status;
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

    [[nodiscard]] std::expected<void, ice::Status> compare_and_swap(
        const ice::sonic::String& key,
        const ice::sonic::String& expected_value,
        const ice::sonic::String& new_value,
        TF_Store_BoolFn completion,
        void* user_data
    ) noexcept
    {
        ice::Status status;
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

    [[nodiscard]] std::expected<void, ice::Status> expire(
        const ice::sonic::String& key,
        int64_t ttl_seconds,
        TF_Store_AckFn completion,
        void* user_data
    ) noexcept
    {
        ice::Status status;
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

    [[nodiscard]] std::expected<void, ice::Status>
    get_ttl(const ice::sonic::String& key, TF_Store_IntFn completion, void* user_data) noexcept
    {
        ice::Status status;
        m_ops->get_ttl(get_handle(), key.get_handle(), completion, user_data, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    persist(const ice::sonic::String& key, TF_Store_AckFn completion, void* user_data) noexcept
    {
        ice::Status status;
        m_ops->persist(get_handle(), key.get_handle(), completion, user_data, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> begin_transaction() noexcept
    {
        ice::Status status;
        m_ops->begin_transaction(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    commit_transaction(TF_Store_AckFn completion, void* user_data) noexcept
    {
        ice::Status status;
        m_ops->commit_transaction(get_handle(), completion, user_data, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> rollback_transaction() noexcept
    {
        ice::Status status;
        m_ops->rollback_transaction(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    create_index(const ice::sonic::String& name, const ice::sonic::Map& field_config) noexcept
    {
        ice::Status status;
        m_ops->create_index(
            get_handle(),
            name.get_handle(),
            field_config.get_handle(),
            status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    drop_index(const ice::sonic::String& name) noexcept
    {
        ice::Status status;
        m_ops->drop_index(get_handle(), name.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    list_indexes(const ice::sonic::Vector& out_names) noexcept
    {
        ice::Status status;
        m_ops->list_indexes(get_handle(), out_names.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    watch(const ice::sonic::String& key_prefix, TF_Store_WatchFn handler, void* user_data) noexcept
    {
        ice::Status status;
        m_ops
            ->watch(get_handle(), key_prefix.get_handle(), handler, user_data, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> unwatch() noexcept
    {
        ice::Status status;
        m_ops->unwatch(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> query(
        const ice::sonic::Map& filters,
        const ice::sonic::String& free_text,
        const ice::sonic::String& sort,
        size_t offset,
        size_t limit,
        TF_Store_QueryFn completion,
        void* user_data
    ) noexcept
    {
        ice::Status status;
        m_ops->query(
            get_handle(),
            filters.get_handle(),
            free_text.get_handle(),
            sort.get_handle(),
            offset,
            limit,
            completion,
            user_data,
            status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> backup(
        const ice::sonic::String& destination,
        TF_Store_AckFn completion,
        void* user_data
    ) noexcept
    {
        ice::Status status;
        m_ops->backup(
            get_handle(),
            destination.get_handle(),
            completion,
            user_data,
            status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    restore(const ice::sonic::String& source, TF_Store_AckFn completion, void* user_data) noexcept
    {
        ice::Status status;
        m_ops->restore(
            get_handle(),
            source.get_handle(),
            completion,
            user_data,
            status.get_handle()
        );

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
