// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/filesystem/random_access_file.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/filesystem/random_access_file.h"

export module cc_abi_builder_filesystem;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;

export namespace ice::builder {

class TF_RandomAccessFileOps
{
public:
    static TF_RandomAccessFileOps* create(void* ctx) noexcept
    {
        return static_cast<TF_RandomAccessFileOps*>(ctx);
    }

    template<typename HandleT>
    static TF_RandomAccessFileOps* create(HandleT* handle) noexcept
    {
        return static_cast<TF_RandomAccessFileOps*>(handle->plugin_data);
    }

    virtual ~TF_RandomAccessFileOps() = default;
    [[nodiscard]] std::expected<void, ice::Status>
    read(uint64_t offset, size_t n, char* buffer, int64_t* out_bytes_read) noexcept = 0;
    virtual ice::String get_name() const noexcept = 0;

    static TF_RandomAccessFileOps* get_generic_vtable()
    {
        static TF_RandomAccessFileOps vtable = {
            .struct_size = TF_RANDOMACCESSFILE_STRUCT_SIZE,

            .destroy =
                [](void* plugin_context) noexcept
            {
                delete TF_RandomAccessFileOps::create(plugin_context);
            },

            .get_name =
                [](void* plugin_context, TF_String* out) noexcept
            {
                auto* self = TF_RandomAccessFileOps::create(plugin_context);
                auto result = self->get_name();
                result.to_c(out);
            },
            .read =
                [](TF_RandomAccessFile* file,
                   uint64_t offset,
                   size_t n,
                   char* buffer,
                   int64_t* out_bytes_read,
                   TF_Status* out_status) noexcept
            {
                auto* self = TF_RandomAccessFileOps::create(file);
                auto res = self->read(offset, n, buffer, out_bytes_read);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },

        };

        return &vtable;
    }
};

} // namespace ice::builder
