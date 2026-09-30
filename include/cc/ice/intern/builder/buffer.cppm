// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/intern/buffer.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/intern/buffer.h"

export module cc_ice_intern_builder:buffer;

import std;

export namespace ice::builder {

class TF_BufferOps
{
public:
    TF_BufferOps() noexcept :
        m_handle{.plugin_data = this}
    {
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
    virtual void get_name(const ice::sonic::String& out_name) noexcept = 0;
    virtual void assign_from_string(const void* proto, size_t proto_len) noexcept = 0;
    virtual void delete_buffer() noexcept = 0;
    virtual void get_buffer(TFBufferData* out_buffer) noexcept = 0;

    void get_generic_vtable() noexcept
    {
        m_vtable = ::TF_BufferOps{
            .struct_size = TF_BUFFER_STRUCT_SIZE,
            .get_name =
                [](TF_Buffer* buffer, TF_String* out_name) noexcept
            {
                TF_BufferOps::from_handle(buffer).get_name(ice::sonic::String::wrap(out_name));
            },
            .assign_from_string =
                [](TF_Buffer* buffer, const void* proto, size_t proto_len) noexcept
            {
                TF_BufferOps::from_handle(buffer).assign_from_string(proto, proto_len);
            },
            .delete_buffer =
                [](TF_Buffer* buffer) noexcept
            {
                TF_BufferOps::from_handle(buffer).delete_buffer();
            },
            .get_buffer =
                [](TF_Buffer* buffer, TFBufferData* out_buffer) noexcept
            {
                TF_BufferOps::from_handle(buffer).get_buffer(out_buffer);
            },

        };
    }

    const ::TF_BufferOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const TF_Buffer& get_handle() const noexcept
    {
        return m_handle;
    }


private:
    ::TF_BufferOps m_vtable;
    TF_Buffer m_handle;
};

} // namespace ice::builder
