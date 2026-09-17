// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/intern/bitset.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/intern/bitset.h"

export module cc_abi_builder_intern;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;

export namespace ice::builder {

class TF_BitSetOps
{
public:
    static TF_BitSetOps* create(void* ctx) noexcept
    {
        return static_cast<TF_BitSetOps*>(ctx);
    }

    template<typename HandleT>
    static TF_BitSetOps* create(HandleT* handle) noexcept
    {
        return static_cast<TF_BitSetOps*>(handle->plugin_data);
    }

    virtual ~TF_BitSetOps() = default;
    [[nodiscard]] std::expected<void, ice::Status> set(size_t index) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> clear(size_t index) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> test(size_t index, int* out_result) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> flip(size_t index) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> count(size_t* out_count) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> size(size_t* out_size) noexcept = 0;
    virtual ice::String get_name() const noexcept = 0;

    static TF_BitSetOps* get_generic_vtable()
    {
        static TF_BitSetOps vtable = {
            .struct_size = TF_BITSET_STRUCT_SIZE,
            .set =
                [](TF_BitSet* bitset, size_t index) noexcept
            {
                auto* self = TF_BitSetOps::create(bitset);
                auto res = self->set(index);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .clear =
                [](TF_BitSet* bitset, size_t index) noexcept
            {
                auto* self = TF_BitSetOps::create(bitset);
                auto res = self->clear(index);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .test =
                [](const TF_BitSet* bitset,
                   size_t index,
                   int* out_result,
                   TF_Status* out_status) noexcept
            {
                auto* self = TF_BitSetOps::create(bitset);
                auto res = self->test(index, out_result);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .flip =
                [](TF_BitSet* bitset, size_t index, TF_Status* out_status) noexcept
            {
                auto* self = TF_BitSetOps::create(bitset);
                auto res = self->flip(index);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .count =
                [](const TF_BitSet* bitset, size_t* out_count) noexcept
            {
                auto* self = TF_BitSetOps::create(bitset);
                auto res = self->count(out_count);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .size =
                [](const TF_BitSet* bitset, size_t* out_size) noexcept
            {
                auto* self = TF_BitSetOps::create(bitset);
                auto res = self->size(out_size);
                if (!res) {
                    res.error().to_c(status);
                }
            },

            .destroy =
                [](void* plugin_context) noexcept
            {
                delete TF_BitSetOps::create(plugin_context);
            },

        };

        return &vtable;
    }
};

} // namespace ice::builder
