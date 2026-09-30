// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/filesystem/random_access_file.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/filesystem/random_access_file.h"

export module cc_ice_extern_filesystem_builder:random_access_file;

import std;

export namespace ice::builder {

class TF_RandomAccessFileOps
{
public:
    TF_RandomAccessFileOps() noexcept :
        m_handle{.plugin_data = this}
    {
    }

    TF_RandomAccessFileOps(const TF_RandomAccessFileOps&) = delete;
    TF_RandomAccessFileOps& operator=(const TF_RandomAccessFileOps&) = delete;

    static TF_RandomAccessFileOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TF_RandomAccessFileOps*>(ctx);
    }

    template<typename HandleT>
    static TF_RandomAccessFileOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TF_RandomAccessFileOps*>(handle->plugin_data);
    }

    virtual ~TF_RandomAccessFileOps() = default;
    virtual void destroy() noexcept = 0;
    virtual void get_name(const ice::sonic::String& out_name) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    read(uint64_t offset, size_t n, char* buffer, int64_t* out_bytes_read) noexcept = 0;

    void get_generic_vtable() noexcept
    {
        m_vtable = ::TF_RandomAccessFileOps{
            .struct_size = TF_RANDOMACCESSFILE_STRUCT_SIZE,
            .destroy =
                [](TF_RandomAccessFile* file) noexcept
            {
                TF_RandomAccessFileOps::from_handle(file).destroy();
            },
            .get_name =
                [](TF_RandomAccessFile* file, TF_String* out_name) noexcept
            {
                TF_RandomAccessFileOps::from_handle(file).get_name(
                    ice::sonic::String::wrap(out_name)
                );
            },
            .read =
                [](TF_RandomAccessFile* file,
                   uint64_t offset,
                   size_t n,
                   char* buffer,
                   int64_t* out_bytes_read,
                   TF_Status* out_status) noexcept
            {
                auto res = TF_RandomAccessFileOps::from_handle(file)
                               .read(offset, n, buffer, out_bytes_read);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },

        };
    }

    const ::TF_RandomAccessFileOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const TF_RandomAccessFile& get_handle() const noexcept
    {
        return m_handle;
    }


private:
    ::TF_RandomAccessFileOps m_vtable;
    TF_RandomAccessFile m_handle;
};

} // namespace ice::builder
