// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from c/extern/map/map.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "c/extern/map/map.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"

export module cc_abi_builder_map;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;

export namespace ice::builder {

class Map
{
public:
    static Map* create(void* ctx) noexcept
    {
        return static_cast<Map*>(ctx);
    }

    template<typename HandleT>
    static Map* create(HandleT* handle) noexcept
    {
        return reinterpret_cast<Map*>(handle);
    }

    virtual ~Map() = default;
    [[nodiscard]] std::expected<void, ice::Status> new_map(
        size_t key_size,
        size_t value_size,
        TF_MapHashFn hash_fn,
        TF_MapCompareFn compare_fn,
        int allow_duplicates
    ) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    insert(const void* key, const void* value) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> find(const void* key) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> erase(const void* key) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> contains(const void* key) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> size() noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    for_each(TF_MapVisitor visitor, void* capture) noexcept = 0;
    virtual ice::String get_name() const noexcept = 0;

    static TF_Map* get_generic_vtable()
    {
        static TF_Map vtable = {
            .struct_size = TF_MAP_STRUCT_SIZE,
            .new_map =
                [](void* plugin_context,
                   size_t key_size,
                   size_t value_size,
                   TF_MapHashFn hash_fn,
                   TF_MapCompareFn compare_fn,
                   int allow_duplicates) noexcept
            {
                auto* self = Map::create(plugin_context);
                auto res =
                    self->new_map(key_size, value_size, hash_fn, compare_fn, allow_duplicates);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .insert =
                [](TF_Map_Handle* map, const void* key, const void* value) noexcept
            {
                auto* self = Map::create(map);
                auto res = self->insert(key, value);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .find =
                [](const TF_Map_Handle* map, const void* key) noexcept
            {
                auto* self = Map::create(map);
                auto res = self->find(key);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .erase =
                [](TF_Map_Handle* map, const void* key) noexcept
            {
                auto* self = Map::create(map);
                auto res = self->erase(key);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .contains =
                [](const TF_Map_Handle* map, const void* key) noexcept
            {
                auto* self = Map::create(map);
                auto res = self->contains(key);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .size =
                [](const TF_Map_Handle* map) noexcept
            {
                auto* self = Map::create(map);
                auto res = self->size();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .for_each =
                [](const TF_Map_Handle* map, TF_MapVisitor visitor, void* capture) noexcept
            {
                auto* self = Map::create(map);
                auto res = self->for_each(visitor, capture);
                if (!res) {
                    res.error().to_c(status);
                }
            },

            .destroy =
                [](void* plugin_context) noexcept
            {
                delete Map::create(plugin_context);
            },

        };

        return &vtable;
    }
};

} // namespace ice::builder
