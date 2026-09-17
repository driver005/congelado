// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/store/collection.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/store/collection.h"

export module cc_abi_builder_store;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;

export namespace ice::builder {

class TFStoreCollectionOps
{
public:
    static TFStoreCollectionOps* create(void* ctx) noexcept
    {
        return static_cast<TFStoreCollectionOps*>(ctx);
    }

    template<typename HandleT>
    static TFStoreCollectionOps* create(HandleT* handle) noexcept
    {
        return static_cast<TFStoreCollectionOps*>(handle->plugin_data);
    }

    virtual ~TFStoreCollectionOps() = default;
    [[nodiscard]] std::expected<void, ice::Status> close() noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    list(const ice::sonic::TF_VectorOps& out_names) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    drop(const ice::sonic::TF_StringOps& name) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    get_stats(const ice::sonic::TF_MapOps& out_stats) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    get(const ice::sonic::TF_StringOps& key,
        TFStoreGetCompletionFn completion,
        void* user_data) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> multi_get(
        const ice::sonic::TF_VectorOps& keys,
        TFStoreMultiGetCompletionFn completion,
        void* user_data
    ) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    set(const ice::sonic::TF_StringOps& key,
        const ice::sonic::TF_StringOps& value,
        int64_t ttl_seconds,
        TFStoreSetCompletionFn completion,
        void* user_data) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> multi_set(
        const ice::sonic::TF_MapOps& entries,
        int64_t ttl_seconds,
        TFStoreAckFn completion,
        void* user_data
    ) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    erase(const ice::sonic::TF_StringOps& key, TFStoreAckFn completion, void* user_data) noexcept =
        0;
    [[nodiscard]] std::expected<void, ice::Status> multi_erase(
        const ice::sonic::TF_VectorOps& keys,
        TFStoreAckFn completion,
        void* user_data
    ) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> exists(
        const ice::sonic::TF_StringOps& key,
        TFStoreExistsFn completion,
        void* user_data
    ) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> rename(
        const ice::sonic::TF_StringOps& old_key,
        const ice::sonic::TF_StringOps& new_key,
        TFStoreAckFn completion,
        void* user_data
    ) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    clear(TFStoreAckFn completion, void* user_data) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> increment(
        const ice::sonic::TF_StringOps& key,
        int64_t delta,
        TFStoreIntFn completion,
        void* user_data
    ) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> compare_and_swap(
        const ice::sonic::TF_StringOps& key,
        const ice::sonic::TF_StringOps& expected_value,
        const ice::sonic::TF_StringOps& new_value,
        TFStoreBoolFn completion,
        void* user_data
    ) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> expire(
        const ice::sonic::TF_StringOps& key,
        int64_t ttl_seconds,
        TFStoreAckFn completion,
        void* user_data
    ) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> get_ttl(
        const ice::sonic::TF_StringOps& key,
        TFStoreIntFn completion,
        void* user_data
    ) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> persist(
        const ice::sonic::TF_StringOps& key,
        TFStoreAckFn completion,
        void* user_data
    ) noexcept = 0;
    virtual ice::String get_name() const noexcept = 0;

    static TFStoreCollectionOps* get_generic_vtable()
    {
        static TFStoreCollectionOps vtable = {
            .struct_size = TF_TORECOLLECTION_STRUCT_SIZE,

            .destroy =
                [](void* plugin_context) noexcept
            {
                delete TFStoreCollectionOps::create(plugin_context);
            },
            .close =
                [](TFStoreCollection* collection) noexcept
            {
                auto* self = TFStoreCollectionOps::create(collection);
                auto res = self->close();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .list =
                [](TFStoreCollection* store, TF_Vector* out_names, TF_Status* out_status) noexcept
            {
                auto* self = TFStoreCollectionOps::create(store);
                auto res = self->list(ice::sonic::TF_VectorOps::wrap(out_names));
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .drop =
                [](TFStoreCollection* store, const TF_String* name, TF_Status* out_status) noexcept
            {
                auto* self = TFStoreCollectionOps::create(store);
                auto res = self->drop(ice::sonic::TF_StringOps::wrap(name));
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_stats =
                [](TFStoreCollection* collection, TF_Map* out_stats, TF_Status* out_status) noexcept
            {
                auto* self = TFStoreCollectionOps::create(collection);
                auto res = self->get_stats(ice::sonic::TF_MapOps::wrap(out_stats));
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get =
                [](TFStoreCollection* collection,
                   const TF_String* key,
                   TFStoreGetCompletionFn completion,
                   void* user_data,
                   TF_Status* out_status) noexcept
            {
                auto* self = TFStoreCollectionOps::create(collection);
                auto res = self->get(ice::sonic::TF_StringOps::wrap(key), completion, user_data);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .multi_get =
                [](TFStoreCollection* collection,
                   const TF_Vector* keys,
                   TFStoreMultiGetCompletionFn completion,
                   void* user_data,
                   TF_Status* out_status) noexcept
            {
                auto* self = TFStoreCollectionOps::create(collection);
                auto res =
                    self->multi_get(ice::sonic::TF_VectorOps::wrap(keys), completion, user_data);
                if (!res) {
                    res.error().to_c(out_status);
                }
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
                auto* self = TFStoreCollectionOps::create(collection);
                auto res = self->set(
                    ice::sonic::TF_StringOps::wrap(key),
                    ice::sonic::TF_StringOps::wrap(value),
                    ttl_seconds,
                    completion,
                    user_data
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .multi_set =
                [](TFStoreCollection* collection,
                   const TF_Map* entries,
                   int64_t ttl_seconds,
                   TFStoreAckFn completion,
                   void* user_data,
                   TF_Status* out_status) noexcept
            {
                auto* self = TFStoreCollectionOps::create(collection);
                auto res = self->multi_set(
                    ice::sonic::TF_MapOps::wrap(entries),
                    ttl_seconds,
                    completion,
                    user_data
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .erase =
                [](TFStoreCollection* collection,
                   const TF_String* key,
                   TFStoreAckFn completion,
                   void* user_data,
                   TF_Status* out_status) noexcept
            {
                auto* self = TFStoreCollectionOps::create(collection);
                auto res = self->erase(ice::sonic::TF_StringOps::wrap(key), completion, user_data);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .multi_erase =
                [](TFStoreCollection* collection,
                   const TF_Vector* keys,
                   TFStoreAckFn completion,
                   void* user_data,
                   TF_Status* out_status) noexcept
            {
                auto* self = TFStoreCollectionOps::create(collection);
                auto res =
                    self->multi_erase(ice::sonic::TF_VectorOps::wrap(keys), completion, user_data);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .exists =
                [](TFStoreCollection* collection,
                   const TF_String* key,
                   TFStoreExistsFn completion,
                   void* user_data,
                   TF_Status* out_status) noexcept
            {
                auto* self = TFStoreCollectionOps::create(collection);
                auto res = self->exists(ice::sonic::TF_StringOps::wrap(key), completion, user_data);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .rename =
                [](TFStoreCollection* collection,
                   const TF_String* old_key,
                   const TF_String* new_key,
                   TFStoreAckFn completion,
                   void* user_data,
                   TF_Status* out_status) noexcept
            {
                auto* self = TFStoreCollectionOps::create(collection);
                auto res = self->rename(
                    ice::sonic::TF_StringOps::wrap(old_key),
                    ice::sonic::TF_StringOps::wrap(new_key),
                    completion,
                    user_data
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .clear =
                [](TFStoreCollection* collection,
                   TFStoreAckFn completion,
                   void* user_data,
                   TF_Status* out_status) noexcept
            {
                auto* self = TFStoreCollectionOps::create(collection);
                auto res = self->clear(completion, user_data);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .increment =
                [](TFStoreCollection* collection,
                   const TF_String* key,
                   int64_t delta,
                   TFStoreIntFn completion,
                   void* user_data,
                   TF_Status* out_status) noexcept
            {
                auto* self = TFStoreCollectionOps::create(collection);
                auto res = self->increment(
                    ice::sonic::TF_StringOps::wrap(key),
                    delta,
                    completion,
                    user_data
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
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
                auto* self = TFStoreCollectionOps::create(collection);
                auto res = self->compare_and_swap(
                    ice::sonic::TF_StringOps::wrap(key),
                    ice::sonic::TF_StringOps::wrap(expected_value),
                    ice::sonic::TF_StringOps::wrap(new_value),
                    completion,
                    user_data
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .expire =
                [](TFStoreCollection* collection,
                   const TF_String* key,
                   int64_t ttl_seconds,
                   TFStoreAckFn completion,
                   void* user_data,
                   TF_Status* out_status) noexcept
            {
                auto* self = TFStoreCollectionOps::create(collection);
                auto res = self->expire(
                    ice::sonic::TF_StringOps::wrap(key),
                    ttl_seconds,
                    completion,
                    user_data
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_ttl =
                [](TFStoreCollection* collection,
                   const TF_String* key,
                   TFStoreIntFn completion,
                   void* user_data,
                   TF_Status* out_status) noexcept
            {
                auto* self = TFStoreCollectionOps::create(collection);
                auto res =
                    self->get_ttl(ice::sonic::TF_StringOps::wrap(key), completion, user_data);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .persist =
                [](TFStoreCollection* collection,
                   const TF_String* key,
                   TFStoreAckFn completion,
                   void* user_data,
                   TF_Status* out_status) noexcept
            {
                auto* self = TFStoreCollectionOps::create(collection);
                auto res =
                    self->persist(ice::sonic::TF_StringOps::wrap(key), completion, user_data);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },

        };

        return &vtable;
    }
};

} // namespace ice::builder
