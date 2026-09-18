// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/intern/hash.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/intern/hash.h"

export module cc_ice_builder_intern:hash;

import std;

export namespace ice::builder {

class TF_HashOps
{
public:
    static TF_HashOps* create(void* ctx) noexcept
    {
        return static_cast<TF_HashOps*>(ctx);
    }

    template<typename HandleT>
    static TF_HashOps* create(HandleT* handle) noexcept
    {
        return static_cast<TF_HashOps*>(handle->plugin_data);
    }

    virtual ~TF_HashOps() = default;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    hash_bytes(const void* data, size_t size, size_t* out_hash) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    hash_combine(size_t seed, size_t value, size_t* out_hash) noexcept = 0;

    static TF_HashOps* get_generic_vtable()
    {
        static TF_HashOps vtable = {
            .struct_size = TF_HASH_STRUCT_SIZE,

            .get_name =
                [](void* plugin_context, TF_String* out) noexcept
            {
                auto* self = TF_HashOps::create(plugin_context);
                auto result = self->get_name();
                result.to_c(out);
            },
            .hash_bytes =
                [](TF_Hash* hash,
                   const void* data,
                   size_t size,
                   size_t* out_hash,
                   TF_Status* out_status) noexcept
            {
                auto* self = TF_HashOps::create(hash);
                auto res = self->hash_bytes(data, size, out_hash);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .hash_combine =
                [](TF_Hash* hash,
                   size_t seed,
                   size_t value,
                   size_t* out_hash,
                   TF_Status* out_status) noexcept
            {
                auto* self = TF_HashOps::create(hash);
                auto res = self->hash_combine(seed, value, out_hash);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },

        };

        return &vtable;
    }

    builder::String get_name() const noexcept
    {
        builder::String result;
        m_ops->get_name(get_handle(), result.get_handle());
        return result;
    }
};

} // namespace ice::builder
