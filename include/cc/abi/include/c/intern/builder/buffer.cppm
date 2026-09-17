// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/intern/buffer.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/intern/buffer.h"

export module cc_abi_builder_intern;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;

export namespace ice::builder {

class TF_BufferOps
{
public:
    static TF_BufferOps* create(void* ctx) noexcept
    {
        return static_cast<TF_BufferOps*>(ctx);
    }

    template<typename HandleT>
    static TF_BufferOps* create(HandleT* handle) noexcept
    {
        return static_cast<TF_BufferOps*>(handle->plugin_data);
    }

    virtual ~TF_BufferOps() = default;
    [[nodiscard]] std::expected<void, ice::Status>
    assign_from_string(const void* proto, size_t proto_len) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> delete_buffer() noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    get_buffer(TFBufferData* out_buffer) noexcept = 0;
    virtual ice::String get_name() const noexcept = 0;

    static TF_BufferOps* get_generic_vtable()
    {
        static TF_BufferOps vtable = {
            .struct_size = TF_BUFFER_STRUCT_SIZE,

            .get_name =
                [](void* plugin_context, TF_String* out) noexcept
            {
                auto* self = TF_BufferOps::create(plugin_context);
                auto result = self->get_name();
                result.to_c(out);
            },
            .assign_from_string =
                [](TF_Buffer* buffer, const void* proto, size_t proto_len) noexcept
            {
                auto* self = TF_BufferOps::create(buffer);
                auto res = self->assign_from_string(proto, proto_len);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .delete_buffer =
                [](TF_Buffer* buffer) noexcept
            {
                auto* self = TF_BufferOps::create(buffer);
                auto res = self->delete_buffer();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_buffer =
                [](TF_Buffer* buffer, TFBufferData* out_buffer) noexcept
            {
                auto* self = TF_BufferOps::create(buffer);
                auto res = self->get_buffer(out_buffer);
                if (!res) {
                    res.error().to_c(status);
                }
            },

        };

        return &vtable;
    }
};

} // namespace ice::builder
