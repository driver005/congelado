// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from c/extern/store/store.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "c/extern/store/store.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"

export module cc_abi_builder_store;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;

export namespace ice::builder {

class Store
{
public:
    static Store* create(void* ctx) noexcept
    {
        return static_cast<Store*>(ctx);
    }

    template<typename HandleT>
    static Store* create(HandleT* handle) noexcept
    {
        return reinterpret_cast<Store*>(handle);
    }

    virtual ~Store() = default;
    [[nodiscard]] std::expected<void, ice::Status> is_connected() noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    open_collection(const ice::sonic::String& name) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> close_collection() noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    list_collections(const ice::sonic::Vector& out_names) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    drop_collection(const ice::sonic::String& name) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    get_collection_stats(const ice::sonic::Map& out_stats) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    get(const ice::sonic::String& key,
        TF_Store_GetCompletionFn completion,
        void* user_data) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> multi_get(
        const ice::sonic::Vector& keys,
        TF_Store_MultiGetCompletionFn completion,
        void* user_data
    ) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    set(const ice::sonic::String& key,
        const ice::sonic::String& value,
        int64_t ttl_seconds,
        TF_Store_SetCompletionFn completion,
        void* user_data) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> multi_set(
        const ice::sonic::Map& entries,
        int64_t ttl_seconds,
        TF_Store_AckFn completion,
        void* user_data
    ) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    remove(const ice::sonic::String& key, TF_Store_AckFn completion, void* user_data) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> multi_remove(
        const ice::sonic::Vector& keys,
        TF_Store_AckFn completion,
        void* user_data
    ) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    exists(const ice::sonic::String& key, TF_Store_ExistsFn completion, void* user_data) noexcept =
        0;
    [[nodiscard]] std::expected<void, ice::Status> rename(
        const ice::sonic::String& old_key,
        const ice::sonic::String& new_key,
        TF_Store_AckFn completion,
        void* user_data
    ) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    clear(TF_Store_AckFn completion, void* user_data) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> increment(
        const ice::sonic::String& key,
        int64_t delta,
        TF_Store_IntFn completion,
        void* user_data
    ) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> compare_and_swap(
        const ice::sonic::String& key,
        const ice::sonic::String& expected_value,
        const ice::sonic::String& new_value,
        TF_Store_BoolFn completion,
        void* user_data
    ) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> expire(
        const ice::sonic::String& key,
        int64_t ttl_seconds,
        TF_Store_AckFn completion,
        void* user_data
    ) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    get_ttl(const ice::sonic::String& key, TF_Store_IntFn completion, void* user_data) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    persist(const ice::sonic::String& key, TF_Store_AckFn completion, void* user_data) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> begin_transaction() noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    commit_transaction(TF_Store_AckFn completion, void* user_data) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> rollback_transaction() noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    create_index(const ice::sonic::String& name, const ice::sonic::Map& field_config) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    drop_index(const ice::sonic::String& name) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    list_indexes(const ice::sonic::Vector& out_names) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> watch(
        const ice::sonic::String& key_prefix,
        TF_Store_WatchFn handler,
        void* user_data
    ) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> unwatch() noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> query(
        const ice::sonic::Map& filters,
        const ice::sonic::String& free_text,
        const ice::sonic::String& sort,
        size_t offset,
        size_t limit,
        TF_Store_QueryFn completion,
        void* user_data
    ) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> backup(
        const ice::sonic::String& destination,
        TF_Store_AckFn completion,
        void* user_data
    ) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    restore(const ice::sonic::String& source, TF_Store_AckFn completion, void* user_data) noexcept =
        0;
    virtual ice::String get_name() const noexcept = 0;

    static TF_Store* get_generic_vtable()
    {
        static TF_Store vtable = {
            .struct_size = TF_STORE_STRUCT_SIZE,

            .destroy =
                [](void* plugin_context) noexcept
            {
                delete Store::create(plugin_context);
            },

            .get_name =
                [](void* plugin_context, TF_String* out) noexcept
            {
                auto* self = Store::create(plugin_context);
                auto result = self->get_name();
                result.to_c(out);
            },
            .is_connected =
                [](void* plugin_context) noexcept
            {
                auto* self = Store::create(plugin_context);
                auto res = self->is_connected();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .open_collection =
                [](void* plugin_context,
                   const TF_String_Handle* name,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Store::create(plugin_context);
                auto res = self->open_collection(ice::sonic::String::wrap(name));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .close_collection =
                [](TF_Store_Handle* collection) noexcept
            {
                auto* self = Store::create(collection);
                auto res = self->close_collection();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .list_collections =
                [](void* plugin_context,
                   TF_Vector_Handle* out_names,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Store::create(plugin_context);
                auto res = self->list_collections(ice::sonic::Vector::wrap(out_names));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .drop_collection =
                [](void* plugin_context,
                   const TF_String_Handle* name,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Store::create(plugin_context);
                auto res = self->drop_collection(ice::sonic::String::wrap(name));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_collection_stats =
                [](TF_Store_Handle* collection,
                   TF_Map_Handle* out_stats,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Store::create(collection);
                auto res = self->get_collection_stats(ice::sonic::Map::wrap(out_stats));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get =
                [](TF_Store_Handle* collection,
                   const TF_String_Handle* key,
                   TF_Store_GetCompletionFn completion,
                   void* user_data,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Store::create(collection);
                auto res = self->get(ice::sonic::String::wrap(key), completion, user_data);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .multi_get =
                [](TF_Store_Handle* collection,
                   const TF_Vector_Handle* keys,
                   TF_Store_MultiGetCompletionFn completion,
                   void* user_data,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Store::create(collection);
                auto res = self->multi_get(ice::sonic::Vector::wrap(keys), completion, user_data);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .set =
                [](TF_Store_Handle* collection,
                   const TF_String_Handle* key,
                   const TF_String_Handle* value,
                   int64_t ttl_seconds,
                   TF_Store_SetCompletionFn completion,
                   void* user_data,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Store::create(collection);
                auto res = self->set(
                    ice::sonic::String::wrap(key),
                    ice::sonic::String::wrap(value),
                    ttl_seconds,
                    completion,
                    user_data
                );
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .multi_set =
                [](TF_Store_Handle* collection,
                   const TF_Map_Handle* entries,
                   int64_t ttl_seconds,
                   TF_Store_AckFn completion,
                   void* user_data,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Store::create(collection);
                auto res = self->multi_set(
                    ice::sonic::Map::wrap(entries),
                    ttl_seconds,
                    completion,
                    user_data
                );
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .remove =
                [](TF_Store_Handle* collection,
                   const TF_String_Handle* key,
                   TF_Store_AckFn completion,
                   void* user_data,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Store::create(collection);
                auto res = self->remove(ice::sonic::String::wrap(key), completion, user_data);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .multi_remove =
                [](TF_Store_Handle* collection,
                   const TF_Vector_Handle* keys,
                   TF_Store_AckFn completion,
                   void* user_data,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Store::create(collection);
                auto res =
                    self->multi_remove(ice::sonic::Vector::wrap(keys), completion, user_data);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .exists =
                [](TF_Store_Handle* collection,
                   const TF_String_Handle* key,
                   TF_Store_ExistsFn completion,
                   void* user_data,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Store::create(collection);
                auto res = self->exists(ice::sonic::String::wrap(key), completion, user_data);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .rename =
                [](TF_Store_Handle* collection,
                   const TF_String_Handle* old_key,
                   const TF_String_Handle* new_key,
                   TF_Store_AckFn completion,
                   void* user_data,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Store::create(collection);
                auto res = self->rename(
                    ice::sonic::String::wrap(old_key),
                    ice::sonic::String::wrap(new_key),
                    completion,
                    user_data
                );
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .clear =
                [](TF_Store_Handle* collection,
                   TF_Store_AckFn completion,
                   void* user_data,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Store::create(collection);
                auto res = self->clear(completion, user_data);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .increment =
                [](TF_Store_Handle* collection,
                   const TF_String_Handle* key,
                   int64_t delta,
                   TF_Store_IntFn completion,
                   void* user_data,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Store::create(collection);
                auto res =
                    self->increment(ice::sonic::String::wrap(key), delta, completion, user_data);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .compare_and_swap =
                [](TF_Store_Handle* collection,
                   const TF_String_Handle* key,
                   const TF_String_Handle* expected_value,
                   const TF_String_Handle* new_value,
                   TF_Store_BoolFn completion,
                   void* user_data,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Store::create(collection);
                auto res = self->compare_and_swap(
                    ice::sonic::String::wrap(key),
                    ice::sonic::String::wrap(expected_value),
                    ice::sonic::String::wrap(new_value),
                    completion,
                    user_data
                );
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .expire =
                [](TF_Store_Handle* collection,
                   const TF_String_Handle* key,
                   int64_t ttl_seconds,
                   TF_Store_AckFn completion,
                   void* user_data,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Store::create(collection);
                auto res =
                    self->expire(ice::sonic::String::wrap(key), ttl_seconds, completion, user_data);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_ttl =
                [](TF_Store_Handle* collection,
                   const TF_String_Handle* key,
                   TF_Store_IntFn completion,
                   void* user_data,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Store::create(collection);
                auto res = self->get_ttl(ice::sonic::String::wrap(key), completion, user_data);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .persist =
                [](TF_Store_Handle* collection,
                   const TF_String_Handle* key,
                   TF_Store_AckFn completion,
                   void* user_data,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Store::create(collection);
                auto res = self->persist(ice::sonic::String::wrap(key), completion, user_data);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .begin_transaction =
                [](void* plugin_context, TF_Status_Handle* status) noexcept
            {
                auto* self = Store::create(plugin_context);
                auto res = self->begin_transaction();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .commit_transaction =
                [](TF_Store_Transaction* transaction,
                   TF_Store_AckFn completion,
                   void* user_data,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Store::create(transaction);
                auto res = self->commit_transaction(completion, user_data);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .rollback_transaction =
                [](TF_Store_Transaction* transaction) noexcept
            {
                auto* self = Store::create(transaction);
                auto res = self->rollback_transaction();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .create_index =
                [](TF_Store_Handle* collection,
                   const TF_String_Handle* name,
                   const TF_Map_Handle* field_config,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Store::create(collection);
                auto res = self->create_index(
                    ice::sonic::String::wrap(name),
                    ice::sonic::Map::wrap(field_config)
                );
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .drop_index =
                [](TF_Store_Handle* collection,
                   const TF_String_Handle* name,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Store::create(collection);
                auto res = self->drop_index(ice::sonic::String::wrap(name));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .list_indexes =
                [](TF_Store_Handle* collection,
                   TF_Vector_Handle* out_names,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Store::create(collection);
                auto res = self->list_indexes(ice::sonic::Vector::wrap(out_names));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .watch =
                [](TF_Store_Handle* collection,
                   const TF_String_Handle* key_prefix,
                   TF_Store_WatchFn handler,
                   void* user_data,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Store::create(collection);
                auto res = self->watch(ice::sonic::String::wrap(key_prefix), handler, user_data);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .unwatch =
                [](TF_Store_Watch* watch) noexcept
            {
                auto* self = Store::create(watch);
                auto res = self->unwatch();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .query =
                [](TF_Store_Handle* collection,
                   const TF_Map_Handle* filters,
                   const TF_String_Handle* free_text,
                   const TF_String_Handle* sort,
                   size_t offset,
                   size_t limit,
                   TF_Store_QueryFn completion,
                   void* user_data,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Store::create(collection);
                auto res = self->query(
                    ice::sonic::Map::wrap(filters),
                    ice::sonic::String::wrap(free_text),
                    ice::sonic::String::wrap(sort),
                    offset,
                    limit,
                    completion,
                    user_data
                );
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .backup =
                [](void* plugin_context,
                   const TF_String_Handle* destination,
                   TF_Store_AckFn completion,
                   void* user_data,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Store::create(plugin_context);
                auto res =
                    self->backup(ice::sonic::String::wrap(destination), completion, user_data);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .restore =
                [](void* plugin_context,
                   const TF_String_Handle* source,
                   TF_Store_AckFn completion,
                   void* user_data,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Store::create(plugin_context);
                auto res = self->restore(ice::sonic::String::wrap(source), completion, user_data);
                if (!res) {
                    res.error().to_c(status);
                }
            },

        };

        return &vtable;
    }
};

} // namespace ice::builder
