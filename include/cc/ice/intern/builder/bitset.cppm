// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/intern/bitset.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/intern/bitset.h"

export module cc_ice_intern_builder:bitset;

import std;

export namespace ice::builder {

class TF_BitSetOps
{
public:
    TF_BitSetOps() noexcept :
        m_handle{.plugin_data = this}
    {
    }

    TF_BitSetOps(const TF_BitSetOps&) = delete;
    TF_BitSetOps& operator=(const TF_BitSetOps&) = delete;

    static TF_BitSetOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TF_BitSetOps*>(ctx);
    }

    template<typename HandleT>
    static TF_BitSetOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TF_BitSetOps*>(handle->plugin_data);
    }

    virtual ~TF_BitSetOps() = default;
    [[nodiscard]] virtual std::expected<void, ice::Status> set(size_t index) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> clear(size_t index) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    test(size_t index, int* out_result) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> flip(size_t index) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> count(size_t* out_count) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> size(size_t* out_size) noexcept = 0;

    void get_generic_vtable() noexcept
    {
        m_vtable = ::TF_BitSetOps{
            .struct_size = TF_BITSET_STRUCT_SIZE,
            .set =
                [](TF_BitSet* bitset, size_t index) noexcept
            {
                auto res = TF_BitSetOps::from_handle(bitset).set(index);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .clear =
                [](TF_BitSet* bitset, size_t index) noexcept
            {
                auto res = TF_BitSetOps::from_handle(bitset).clear(index);
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
                auto res = TF_BitSetOps::from_handle(bitset).test(index, out_result);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .flip =
                [](TF_BitSet* bitset, size_t index, TF_Status* out_status) noexcept
            {
                auto res = TF_BitSetOps::from_handle(bitset).flip(index);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .count =
                [](const TF_BitSet* bitset, size_t* out_count) noexcept
            {
                auto res = TF_BitSetOps::from_handle(bitset).count(out_count);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .size =
                [](const TF_BitSet* bitset, size_t* out_size) noexcept
            {
                auto res = TF_BitSetOps::from_handle(bitset).size(out_size);
                if (!res) {
                    res.error().to_c(status);
                }
            },

            .destroy =
                [](void* plugin_context) noexcept
            {
                std::unique_ptr<TF_BitSetOps>{&TF_BitSetOps::from_handle(plugin_context)};
            },

        };
    }

    const ::TF_BitSetOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const TF_BitSet& get_handle() const noexcept
    {
        return m_handle;
    }


private:
    ::TF_BitSetOps m_vtable;
    TF_BitSet m_handle;
};

} // namespace ice::builder
