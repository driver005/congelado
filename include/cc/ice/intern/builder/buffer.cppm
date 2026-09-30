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
    [[nodiscard]] virtual std::expected<void, ice::Status>
    assign_from_string(const void* proto, size_t proto_len) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> delete_buffer() noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_buffer(TFBufferData* out_buffer) noexcept = 0;

    void get_generic_vtable() noexcept
    {
        m_vtable = ::TF_BufferOps{
            .struct_size = TF_BUFFER_STRUCT_SIZE,

            .get_name =
                [](void* plugin_context, TF_String* out) noexcept
            {
                auto result = TF_BufferOps::from_handle(plugin_context).get_name();
                result.to_c(out);
            },
            .assign_from_string =
                [](TF_Buffer* buffer, const void* proto, size_t proto_len) noexcept
            {
                auto res = TF_BufferOps::from_handle(buffer).assign_from_string(proto, proto_len);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .delete_buffer =
                [](TF_Buffer* buffer) noexcept
            {
                auto res = TF_BufferOps::from_handle(buffer).delete_buffer();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_buffer =
                [](TF_Buffer* buffer, TFBufferData* out_buffer) noexcept
            {
                auto res = TF_BufferOps::from_handle(buffer).get_buffer(out_buffer);
                if (!res) {
                    res.error().to_c(status);
                }
            },

        };
    }

    const ::TF_BufferOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    virtual builder::String get_name() const noexcept = 0;

    const TF_Buffer& get_handle() const noexcept
    {
        return m_handle;
    }


private:
    ::TF_BufferOps m_vtable;
    TF_Buffer m_handle;
};

} // namespace ice::builder
