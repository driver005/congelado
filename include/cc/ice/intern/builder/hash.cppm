// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/intern/hash.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/intern/hash.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"

export module cc_ice_intern_builder:hash;

import std;
import cc_ice_intern_sonic;

export namespace ice::builder {

class TF_HashOps
{
public:
    explicit TF_HashOps(const ::TF_StatusOps* Status_ops, const ::TF_StringOps* String_ops) noexcept
        :
        m_handle{.plugin_data = this}
    {
        m_Status_ops = Status_ops;
        m_String_ops = String_ops;
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
    virtual void destroy() noexcept = 0;
    virtual void get_name(const ice::sonic::String& out_name) noexcept = 0;
    virtual void hash_bytes(
        const void* data,
        size_t size,
        size_t* out_hash,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void hash_combine(
        size_t seed,
        size_t value,
        size_t* out_hash,
        const ice::sonic::Status& out_status
    ) noexcept = 0;

    void get_generic_vtable(void (*create)(::TF_Hash*)) noexcept
    {
        m_vtable = ::TF_HashOps{
            .struct_size = TF_OFFSET_OF_END(::TF_HashOps, hash_combine),

            .create = create,
            .destroy =
                [](TF_Hash* handle) noexcept
            {
                auto& self = TF_HashOps::from_handle(handle);
                self.destroy();
            },
            .get_name =
                [](TF_Hash* hash, TF_String* out_name) noexcept
            {
                auto& self = TF_HashOps::from_handle(hash);
                self.get_name(self.wrap(std::type_identity<ice::sonic::String>{}, out_name));
            },
            .hash_bytes =
                [](TF_Hash* hash,
                   const void* data,
                   size_t size,
                   size_t* out_hash,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_HashOps::from_handle(hash);
                self.hash_bytes(
                    data,
                    size,
                    out_hash,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .hash_combine =
                [](TF_Hash* hash,
                   size_t seed,
                   size_t value,
                   size_t* out_hash,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_HashOps::from_handle(hash);
                self.hash_combine(
                    seed,
                    value,
                    out_hash,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },

        };
    }

    ice::sonic::Status
    wrap(std::type_identity<ice::sonic::Status>, const ::TF_Status* handle) const noexcept
    {
        return ice::sonic::Status{m_Status_ops, const_cast<::TF_Status*>(handle)};
    }

    ice::sonic::String
    wrap(std::type_identity<ice::sonic::String>, const ::TF_String* handle) const noexcept
    {
        return ice::sonic::String{m_String_ops, const_cast<::TF_String*>(handle)};
    }

    const ::TF_HashOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const ::TF_Hash& get_handle() const noexcept
    {
        return m_handle;
    }

    template<typename Registry, typename StringType>
    void register_ops(
        Registry& registry,
        const StringType& type,
        const StringType& provider
    ) const noexcept
    {
        registry.register_op(type, provider, const_cast<::TF_HashOps*>(&m_vtable));
    }

private:
    ::TF_HashOps m_vtable;
    ::TF_Hash m_handle;

    const ::TF_StatusOps* m_Status_ops{nullptr};

    const ::TF_StringOps* m_String_ops{nullptr};
};

} // namespace ice::builder
