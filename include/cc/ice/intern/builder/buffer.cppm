// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/intern/buffer.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/intern/buffer.h"
#include "include/c/intern/tstring.h"

export module cc_ice_intern_builder:buffer;

import std;
import cc_ice_intern_sonic;

export namespace ice::builder {

class TF_BufferOps
{
public:
    explicit TF_BufferOps(const ::TF_StringOps* String_ops) noexcept :
        m_handle{.plugin_data = this}
    {
        m_String_ops = String_ops;
    }

    TF_BufferOps(const TF_BufferOps&) = delete;
    TF_BufferOps& operator=(const TF_BufferOps&) = delete;

    static TF_BufferOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TF_BufferOps*>(ctx);
    }

    template<typename HandleT>
    static TF_BufferOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TF_BufferOps*>(handle->plugin_data);
    }

    virtual ~TF_BufferOps() = default;
    virtual void destroy() noexcept = 0;
    virtual void get_name(const ice::sonic::String& out_name) noexcept = 0;
    virtual void assign_from_string(const void* proto, size_t proto_len) noexcept = 0;
    virtual void delete_buffer() noexcept = 0;
    virtual void get_buffer(TFBufferData* out_buffer) noexcept = 0;

    void get_generic_vtable(void (*create)(::TF_Buffer*)) noexcept
    {
        m_vtable = ::TF_BufferOps{
            .struct_size = TF_OFFSET_OF_END(::TF_BufferOps, get_buffer),

            .create = create,
            .destroy =
                [](TF_Buffer* handle) noexcept
            {
                auto& self = TF_BufferOps::from_handle(handle);
                self.destroy();
            },
            .get_name =
                [](TF_Buffer* buffer, TF_String* out_name) noexcept
            {
                auto& self = TF_BufferOps::from_handle(buffer);
                self.get_name(self.wrap(std::type_identity<ice::sonic::String>{}, out_name));
            },
            .assign_from_string =
                [](TF_Buffer* buffer, const void* proto, size_t proto_len) noexcept
            {
                auto& self = TF_BufferOps::from_handle(buffer);
                self.assign_from_string(proto, proto_len);
            },
            .delete_buffer =
                [](TF_Buffer* buffer) noexcept
            {
                auto& self = TF_BufferOps::from_handle(buffer);
                self.delete_buffer();
            },
            .get_buffer =
                [](TF_Buffer* buffer, TFBufferData* out_buffer) noexcept
            {
                auto& self = TF_BufferOps::from_handle(buffer);
                self.get_buffer(out_buffer);
            },

        };
    }

    ice::sonic::String
    wrap(std::type_identity<ice::sonic::String>, const ::TF_String* handle) const noexcept
    {
        return ice::sonic::String{m_String_ops, const_cast<::TF_String*>(handle)};
    }

    const ::TF_BufferOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const ::TF_Buffer& get_handle() const noexcept
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
        registry.register_op(type, provider, const_cast<::TF_BufferOps*>(&m_vtable));
    }

private:
    ::TF_BufferOps m_vtable;
    ::TF_Buffer m_handle;

    const ::TF_StringOps* m_String_ops{nullptr};
};

} // namespace ice::builder
