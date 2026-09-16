// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from c/extern/buffer/buffer.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "c/extern/buffer/buffer.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"

export module cc_abi_builder_buffer;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;

export namespace ice::builder {

class Buffer
{
public:
    static Buffer* create(void* ctx) noexcept
    {
        return static_cast<Buffer*>(ctx);
    }

    template<typename HandleT>
    static Buffer* create(HandleT* handle) noexcept
    {
        return reinterpret_cast<Buffer*>(handle);
    }

    virtual ~Buffer() = default;
    [[nodiscard]] std::expected<void, ice::Status>
    new_buffer_from_string(const void* proto, size_t proto_len) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> new_buffer() noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> delete_buffer() noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> get_buffer() noexcept = 0;
    virtual ice::String get_name() const noexcept = 0;

    static TF_Buffer* get_generic_vtable()
    {
        static TF_Buffer vtable = {
            .struct_size = TF_BUFFER_STRUCT_SIZE,

            .get_name =
                [](void* plugin_context, TF_String* out) noexcept
            {
                auto* self = Buffer::create(plugin_context);
                auto result = self->get_name();
                result.to_c(out);
            },
            .new_buffer_from_string =
                [](void* plugin_context, const void* proto, size_t proto_len) noexcept
            {
                auto* self = Buffer::create(plugin_context);
                auto res = self->new_buffer_from_string(proto, proto_len);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .new_buffer =
                [](void* plugin_context) noexcept
            {
                auto* self = Buffer::create(plugin_context);
                auto res = self->new_buffer();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .delete_buffer =
                [](TF_Buffer_Handle* buffer) noexcept
            {
                auto* self = Buffer::create(buffer);
                auto res = self->delete_buffer();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_buffer =
                [](TF_Buffer_Handle* buffer) noexcept
            {
                auto* self = Buffer::create(buffer);
                auto res = self->get_buffer();
                if (!res) {
                    res.error().to_c(status);
                }
            },

        };

        return &vtable;
    }
};

} // namespace ice::builder
