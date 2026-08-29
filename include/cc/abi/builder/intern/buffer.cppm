module;

#include "c/intern/tf_buffer.h"

export module cc_abi_builder_intern:buffer;

import std;
import cc_abi_primitives;

export namespace ice::builder {

class Buffer
{
public:
    // Recover the Buffer instance from the opaque void* context slot that every
    // C vtable callback receives.  Named accessor so the cast intent is explicit
    // at the call site and the static_cast appears exactly once, here.
    static Buffer* create(void* ctx) noexcept
    {
        return static_cast<Buffer*>(ctx);
    }

    virtual ~Buffer() = default;

    virtual ice::String get_name() const noexcept = 0;

    // Range-first: the C ABI carries (const void*, size_t) — the C++ interface takes a
    // span; the vtable lambda below is the only place the raw pair exists.
    [[nodiscard]] virtual std::expected<TF_Buffer_Handle*, ice::Status>
    new_buffer_from_string(std::span<const std::byte> proto) noexcept = 0;
    [[nodiscard]] virtual std::expected<TF_Buffer_Handle*, ice::Status> new_buffer() noexcept = 0;
    virtual void delete_buffer(TF_Buffer_Handle* buffer) noexcept = 0;
    virtual TF_Buffer_Data get_buffer(TF_Buffer_Handle* buffer) noexcept = 0;

    static TF_Buffer* get_generic_vtable()
    {
        static TF_Buffer vtable = {
            .struct_size = TF_BUFFER_STRUCT_SIZE,
            .get_name =
                [](void* plugin_context, TF_String* out) noexcept
            {
                Buffer::create(plugin_context)->get_name().to_c(out);
            },
            .new_buffer_from_string =
                [](void* plugin_context, const void* proto, size_t proto_len) noexcept
                -> TF_Buffer_Handle*
            {
                auto res = Buffer::create(plugin_context)->new_buffer_from_string(
                    std::span{static_cast<const std::byte*>(proto), proto_len}
                );
                if (!res) {
                    return nullptr; // no status slot — P1 allocator contract
                }
                return res.value();
            },
            .new_buffer = [](void* plugin_context) noexcept -> TF_Buffer_Handle*
            {
                auto res = Buffer::create(plugin_context)->new_buffer();
                if (!res) {
                    return nullptr; // no status slot — P1 allocator contract
                }
                return res.value();
            },
            .delete_buffer =
                [](void* plugin_context, TF_Buffer_Handle* buffer) noexcept
            {
                Buffer::create(plugin_context)->delete_buffer(buffer);
            },
            .get_buffer = [](void* plugin_context, TF_Buffer_Handle* buffer) noexcept
                          -> TF_Buffer_Data
            {
                return Buffer::create(plugin_context)->get_buffer(buffer);
            }
        };
        return &vtable;
    }
};

} // namespace ice::builder
