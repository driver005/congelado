// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/filesystem/writable_file.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/filesystem/writable_file.h"

export module cc_ice_extern_filesystem_builder:writable_file;

import std;

export namespace ice::builder {

class TF_WritableFileOps
{
public:
    TF_WritableFileOps() noexcept :
        m_handle{.plugin_data = this}
    {
    }

    TF_WritableFileOps(const TF_WritableFileOps&) = delete;
    TF_WritableFileOps& operator=(const TF_WritableFileOps&) = delete;

    static TF_WritableFileOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TF_WritableFileOps*>(ctx);
    }

    template<typename HandleT>
    static TF_WritableFileOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TF_WritableFileOps*>(handle->plugin_data);
    }

    virtual ~TF_WritableFileOps() = default;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    append(const ice::sonic::String& buffer) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> tell(int64_t* out_position) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> flush() noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> sync() noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> close() noexcept = 0;

    void get_generic_vtable() noexcept
    {
        m_vtable = ::TF_WritableFileOps{
            .struct_size = TF_WRITABLEFILE_STRUCT_SIZE,

            .destroy =
                [](void* plugin_context) noexcept
            {
                std::unique_ptr<TF_WritableFileOps>{
                    &TF_WritableFileOps::from_handle(plugin_context)
                };
            },

            .get_name =
                [](void* plugin_context, TF_String* out) noexcept
            {
                auto result = TF_WritableFileOps::from_handle(plugin_context).get_name();
                result.to_c(out);
            },
            .append =
                [](TF_WritableFile* file, const TF_String* buffer, TF_Status* out_status) noexcept
            {
                auto res =
                    TF_WritableFileOps::from_handle(file).append(ice::sonic::String::wrap(buffer));
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .tell =
                [](TF_WritableFile* file, int64_t* out_position, TF_Status* out_status) noexcept
            {
                auto res = TF_WritableFileOps::from_handle(file).tell(out_position);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .flush =
                [](TF_WritableFile* file, TF_Status* out_status) noexcept
            {
                auto res = TF_WritableFileOps::from_handle(file).flush();
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .sync =
                [](TF_WritableFile* file, TF_Status* out_status) noexcept
            {
                auto res = TF_WritableFileOps::from_handle(file).sync();
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .close =
                [](TF_WritableFile* file, TF_Status* out_status) noexcept
            {
                auto res = TF_WritableFileOps::from_handle(file).close();
                if (!res) {
                    res.error().to_c(out_status);
                }
            },

        };
    }

    const ::TF_WritableFileOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    virtual builder::String get_name() const noexcept = 0;

    const TF_WritableFile& get_handle() const noexcept
    {
        return m_handle;
    }


private:
    ::TF_WritableFileOps m_vtable;
    TF_WritableFile m_handle;
};

} // namespace ice::builder
