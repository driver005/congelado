// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from c/extern/set/set.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "c/extern/set/set.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"

export module cc_abi_builder_set;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;

export namespace ice::builder {

class Set
{
public:
    static Set* create(void* ctx) noexcept
    {
        return static_cast<Set*>(ctx);
    }

    template<typename HandleT>
    static Set* create(HandleT* handle) noexcept
    {
        return reinterpret_cast<Set*>(handle);
    }

    virtual ~Set() = default;
    [[nodiscard]] std::expected<void, ice::Status> new_set(
        size_t key_size,
        TF_SetHashFn hash_fn,
        TF_SetCompareFn compare_fn,
        int allow_duplicates
    ) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> insert(const void* key) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> find(const void* key) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> erase(const void* key) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> contains(const void* key) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> size() noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    for_each(TF_SetVisitor visitor, void* capture) noexcept = 0;
    virtual ice::String get_name() const noexcept = 0;

    static TF_Set* get_generic_vtable()
    {
        static TF_Set vtable = {
            .struct_size = TF_SET_STRUCT_SIZE,
            .new_set =
                [](void* plugin_context,
                   size_t key_size,
                   TF_SetHashFn hash_fn,
                   TF_SetCompareFn compare_fn,
                   int allow_duplicates) noexcept
            {
                auto* self = Set::create(plugin_context);
                auto res = self->new_set(key_size, hash_fn, compare_fn, allow_duplicates);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .insert =
                [](TF_Set_Handle* set, const void* key) noexcept
            {
                auto* self = Set::create(set);
                auto res = self->insert(key);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .find =
                [](const TF_Set_Handle* set, const void* key) noexcept
            {
                auto* self = Set::create(set);
                auto res = self->find(key);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .erase =
                [](TF_Set_Handle* set, const void* key) noexcept
            {
                auto* self = Set::create(set);
                auto res = self->erase(key);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .contains =
                [](const TF_Set_Handle* set, const void* key) noexcept
            {
                auto* self = Set::create(set);
                auto res = self->contains(key);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .size =
                [](const TF_Set_Handle* set) noexcept
            {
                auto* self = Set::create(set);
                auto res = self->size();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .for_each =
                [](const TF_Set_Handle* set, TF_SetVisitor visitor, void* capture) noexcept
            {
                auto* self = Set::create(set);
                auto res = self->for_each(visitor, capture);
                if (!res) {
                    res.error().to_c(status);
                }
            },

            .destroy =
                [](void* plugin_context) noexcept
            {
                delete Set::create(plugin_context);
            },

        };

        return &vtable;
    }
};

} // namespace ice::builder
