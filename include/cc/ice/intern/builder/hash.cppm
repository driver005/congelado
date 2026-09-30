// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/intern/hash.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/intern/hash.h"

export module cc_ice_intern_builder:hash;

import std;

export namespace ice::builder {

class TF_HashOps
{
public:
    TF_HashOps() noexcept :
        m_handle{.plugin_data = this}
    {
    }

    TF_HashOps(const TF_HashOps&) = delete;
    TF_HashOps& operator=(const TF_HashOps&) = delete;

    static TF_HashOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TF_HashOps*>(ctx);
    }

    template<typename HandleT>
    static TF_HashOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TF_HashOps*>(handle->plugin_data);
    }

    virtual ~TF_HashOps() = default;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    hash_bytes(const void* data, size_t size, size_t* out_hash) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    hash_combine(size_t seed, size_t value, size_t* out_hash) noexcept = 0;

    void get_generic_vtable() noexcept
    {
        m_vtable = ::TF_HashOps{
            .struct_size = TF_HASH_STRUCT_SIZE,

            .get_name =
                [](void* plugin_context, TF_String* out) noexcept
            {
                auto result = TF_HashOps::from_handle(plugin_context).get_name();
                result.to_c(out);
            },
            .hash_bytes =
                [](TF_Hash* hash,
                   const void* data,
                   size_t size,
                   size_t* out_hash,
                   TF_Status* out_status) noexcept
            {
                auto res = TF_HashOps::from_handle(hash).hash_bytes(data, size, out_hash);
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
                auto res = TF_HashOps::from_handle(hash).hash_combine(seed, value, out_hash);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },

        };
    }

    const ::TF_HashOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    virtual builder::String get_name() const noexcept = 0;

    const TF_Hash& get_handle() const noexcept
    {
        return m_handle;
    }


private:
    ::TF_HashOps m_vtable;
    TF_Hash m_handle;
};

} // namespace ice::builder
