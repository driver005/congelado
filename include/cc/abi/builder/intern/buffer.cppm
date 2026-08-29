module;

#include "c/intern/tf_buffer.h"

export module cc_abi_builder_intern:buffer;

import std;
import :ctx;

export namespace ice::builder {

class Buffer
{
public:
    virtual ~Buffer() = default;

    virtual TF_Buffer_Handle* new_buffer_from_string(const void* proto, size_t proto_len) = 0;
    virtual TF_Buffer_Handle* new_buffer() = 0;
    virtual void delete_buffer(TF_Buffer_Handle* buffer) = 0;
    virtual TF_Buffer_Handle get_buffer(TF_Buffer_Handle* buffer) = 0;

    static TF_Buffer* get_generic_vtable()
    {
        static TF_Buffer vtable = {
            .struct_size = sizeof(TF_Buffer),
            .TF_NewBufferFromString = [](void* ctx, const void* proto,
                                         size_t proto_len) -> TF_Buffer_Handle* {
                return ctx_as<Buffer>(ctx)->new_buffer_from_string(proto, proto_len);
            },
            .TF_NewBuffer = [](void* ctx) -> TF_Buffer_Handle* {
                return ctx_as<Buffer>(ctx)->new_buffer();
            },
            .TF_DeleteBuffer =
                [](void* ctx, TF_Buffer_Handle* buffer) {
                    ctx_as<Buffer>(ctx)->delete_buffer(buffer);
                },
            .TF_GetBuffer = [](void* ctx, TF_Buffer_Handle* buffer) -> TF_Buffer_Handle {
                return ctx_as<Buffer>(ctx)->get_buffer(buffer);
            }
        };
        return &vtable;
    }
};

} // namespace ice::builder
