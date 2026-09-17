// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/filesystem/writable_file.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/filesystem/writable_file.h"

export module cc_abi_builder_filesystem;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;

export namespace ice::builder {

class TF_WritableFileOps
{
public:
    static TF_WritableFileOps* create(void* ctx) noexcept
    {
        return static_cast<TF_WritableFileOps*>(ctx);
    }

    template<typename HandleT>
    static TF_WritableFileOps* create(HandleT* handle) noexcept
    {
        return static_cast<TF_WritableFileOps*>(handle->plugin_data);
    }

    virtual ~TF_WritableFileOps() = default;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    append(const ice::sonic::TF_StringOps& buffer) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> tell(int64_t* out_position) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> flush() noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> sync() noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> close() noexcept = 0;

    static TF_WritableFileOps* get_generic_vtable()
    {
        static TF_WritableFileOps vtable = {
            .struct_size = TF_WRITABLEFILE_STRUCT_SIZE,

            .destroy =
                [](void* plugin_context) noexcept
            {
                delete TF_WritableFileOps::create(plugin_context);
            },

            .get_name =
                [](void* plugin_context, TF_String* out) noexcept
            {
                auto* self = TF_WritableFileOps::create(plugin_context);
                auto result = self->get_name();
                result.to_c(out);
            },
            .append =
                [](TF_WritableFile* file, const TF_String* buffer, TF_Status* out_status) noexcept
            {
                auto* self = TF_WritableFileOps::create(file);
                auto res = self->append(ice::sonic::TF_StringOps::wrap(buffer));
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .tell =
                [](TF_WritableFile* file, int64_t* out_position, TF_Status* out_status) noexcept
            {
                auto* self = TF_WritableFileOps::create(file);
                auto res = self->tell(out_position);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .flush =
                [](TF_WritableFile* file, TF_Status* out_status) noexcept
            {
                auto* self = TF_WritableFileOps::create(file);
                auto res = self->flush();
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .sync =
                [](TF_WritableFile* file, TF_Status* out_status) noexcept
            {
                auto* self = TF_WritableFileOps::create(file);
                auto res = self->sync();
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .close =
                [](TF_WritableFile* file, TF_Status* out_status) noexcept
            {
                auto* self = TF_WritableFileOps::create(file);
                auto res = self->close();
                if (!res) {
                    res.error().to_c(out_status);
                }
            },

        };

        return &vtable;
    }
};

} // namespace ice::builder
