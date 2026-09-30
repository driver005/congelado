// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/store/collection.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/store/collection.h"

export module cc_ice_extern_store_builder:collection;

import std;

export namespace ice::builder {

class TFStoreCollectionOps
{
public:
    TFStoreCollectionOps() noexcept :
        m_handle{.plugin_data = this}
    {
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
    [[nodiscard]] virtual std::expected<void, ice::Status> close() noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    list(const ice::sonic::TF_VectorOps& out_names) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    drop(const ice::sonic::String& name) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_stats(const ice::sonic::TF_MapOps& out_stats) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get(const ice::sonic::String& key,
        TFStoreGetCompletionFn completion,
        void* user_data) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> multi_get(
        const ice::sonic::TF_VectorOps& keys,
        TFStoreMultiGetCompletionFn completion,
        void* user_data
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    set(const ice::sonic::String& key,
        const ice::sonic::String& value,
        int64_t ttl_seconds,
        TFStoreSetCompletionFn completion,
        void* user_data) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> multi_set(
        const ice::sonic::TF_MapOps& entries,
        int64_t ttl_seconds,
        TFStoreAckFn completion,
        void* user_data
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    erase(const ice::sonic::String& key, TFStoreAckFn completion, void* user_data) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> multi_erase(
        const ice::sonic::TF_VectorOps& keys,
        TFStoreAckFn completion,
        void* user_data
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    exists(const ice::sonic::String& key, TFStoreExistsFn completion, void* user_data) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> rename(
        const ice::sonic::String& old_key,
        const ice::sonic::String& new_key,
        TFStoreAckFn completion,
        void* user_data
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    clear(TFStoreAckFn completion, void* user_data) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> increment(
        const ice::sonic::String& key,
        int64_t delta,
        TFStoreIntFn completion,
        void* user_data
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> compare_and_swap(
        const ice::sonic::String& key,
        const ice::sonic::String& expected_value,
        const ice::sonic::String& new_value,
        TFStoreBoolFn completion,
        void* user_data
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> expire(
        const ice::sonic::String& key,
        int64_t ttl_seconds,
        TFStoreAckFn completion,
        void* user_data
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_ttl(const ice::sonic::String& key, TFStoreIntFn completion, void* user_data) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    persist(const ice::sonic::String& key, TFStoreAckFn completion, void* user_data) noexcept = 0;

    void get_generic_vtable() noexcept
    {
        m_vtable = ::TFStoreCollectionOps{
            .struct_size = TF_TORECOLLECTION_STRUCT_SIZE,

            .destroy =
                [](void* plugin_context) noexcept
            {
                std::unique_ptr<TFStoreCollectionOps>{
                    &TFStoreCollectionOps::from_handle(plugin_context)
                };
            },
            .close =
                [](TFStoreCollection* collection) noexcept
            {
                auto res = TFStoreCollectionOps::from_handle(collection).close();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .list =
                [](TFStoreCollection* store, TF_Vector* out_names, TF_Status* out_status) noexcept
            {
                auto res = TFStoreCollectionOps::from_handle(store).list(
                    ice::sonic::TF_VectorOps::wrap(out_names)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .drop =
                [](TFStoreCollection* store, const TF_String* name, TF_Status* out_status) noexcept
            {
                auto res =
                    TFStoreCollectionOps::from_handle(store).drop(ice::sonic::String::wrap(name));
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_stats =
                [](TFStoreCollection* collection, TF_Map* out_stats, TF_Status* out_status) noexcept
            {
                auto res = TFStoreCollectionOps::from_handle(collection)
                               .get_stats(ice::sonic::TF_MapOps::wrap(out_stats));
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
                auto res = TFStoreCollectionOps::from_handle(collection)
                               .get(ice::sonic::String::wrap(key), completion, user_data);
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
                auto res =
                    TFStoreCollectionOps::from_handle(collection)
                        .multi_get(ice::sonic::TF_VectorOps::wrap(keys), completion, user_data);
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
                auto res = TFStoreCollectionOps::from_handle(collection)
                               .set(
                                   ice::sonic::String::wrap(key),
                                   ice::sonic::String::wrap(value),
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
                auto res = TFStoreCollectionOps::from_handle(collection)
                               .multi_set(
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
                auto res = TFStoreCollectionOps::from_handle(collection)
                               .erase(ice::sonic::String::wrap(key), completion, user_data);
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
                auto res =
                    TFStoreCollectionOps::from_handle(collection)
                        .multi_erase(ice::sonic::TF_VectorOps::wrap(keys), completion, user_data);
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
                auto res = TFStoreCollectionOps::from_handle(collection)
                               .exists(ice::sonic::String::wrap(key), completion, user_data);
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
                auto res = TFStoreCollectionOps::from_handle(collection)
                               .rename(
                                   ice::sonic::String::wrap(old_key),
                                   ice::sonic::String::wrap(new_key),
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
                auto res =
                    TFStoreCollectionOps::from_handle(collection).clear(completion, user_data);
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
                auto res =
                    TFStoreCollectionOps::from_handle(collection)
                        .increment(ice::sonic::String::wrap(key), delta, completion, user_data);
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
                auto res = TFStoreCollectionOps::from_handle(collection)
                               .compare_and_swap(
                                   ice::sonic::String::wrap(key),
                                   ice::sonic::String::wrap(expected_value),
                                   ice::sonic::String::wrap(new_value),
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
                auto res =
                    TFStoreCollectionOps::from_handle(collection)
                        .expire(ice::sonic::String::wrap(key), ttl_seconds, completion, user_data);
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
                auto res = TFStoreCollectionOps::from_handle(collection)
                               .get_ttl(ice::sonic::String::wrap(key), completion, user_data);
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
                auto res = TFStoreCollectionOps::from_handle(collection)
                               .persist(ice::sonic::String::wrap(key), completion, user_data);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },

        };
    }

    const ::TFStoreCollectionOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const TFStoreCollection& get_handle() const noexcept
    {
        return m_handle;
    }


private:
    ::TFStoreCollectionOps m_vtable;
    TFStoreCollection m_handle;
};

} // namespace ice::builder
