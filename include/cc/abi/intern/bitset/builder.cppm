// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from c/extern/bitset/bitset.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "c/extern/bitset/bitset.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"

export module cc_abi_builder_bitset;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;

export namespace ice::builder {

class Bitset
{
public:
    static Bitset* create(void* ctx) noexcept
    {
        return static_cast<Bitset*>(ctx);
    }

    template<typename HandleT>
    static Bitset* create(HandleT* handle) noexcept
    {
        return reinterpret_cast<Bitset*>(handle);
    }

    virtual ~Bitset() = default;
    [[nodiscard]] std::expected<void, ice::Status> new_bitset(size_t bit_count) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> set(size_t index) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> clear(size_t index) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> test(size_t index) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> flip(size_t index) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> count() noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> size() noexcept = 0;
    virtual ice::String get_name() const noexcept = 0;

    static TF_BitSet* get_generic_vtable()
    {
        static TF_BitSet vtable = {
            .struct_size = TF_BITSET_STRUCT_SIZE,
            .new_bitset =
                [](void* plugin_context, size_t bit_count) noexcept
            {
                auto* self = Bitset::create(plugin_context);
                auto res = self->new_bitset(bit_count);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .set =
                [](TF_BitSet_Handle* bitset, size_t index) noexcept
            {
                auto* self = Bitset::create(bitset);
                auto res = self->set(index);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .clear =
                [](TF_BitSet_Handle* bitset, size_t index) noexcept
            {
                auto* self = Bitset::create(bitset);
                auto res = self->clear(index);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .test =
                [](const TF_BitSet_Handle* bitset, size_t index) noexcept
            {
                auto* self = Bitset::create(bitset);
                auto res = self->test(index);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .flip =
                [](TF_BitSet_Handle* bitset, size_t index) noexcept
            {
                auto* self = Bitset::create(bitset);
                auto res = self->flip(index);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .count =
                [](const TF_BitSet_Handle* bitset) noexcept
            {
                auto* self = Bitset::create(bitset);
                auto res = self->count();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .size =
                [](const TF_BitSet_Handle* bitset) noexcept
            {
                auto* self = Bitset::create(bitset);
                auto res = self->size();
                if (!res) {
                    res.error().to_c(status);
                }
            },

            .destroy =
                [](void* plugin_context) noexcept
            {
                delete Bitset::create(plugin_context);
            },

        };

        return &vtable;
    }
};

} // namespace ice::builder
