// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/filesystem/random_access_file.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/filesystem/random_access_file.h"
#include "include/c/extern/registration/registration.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"

export module cc_ice_extern_filesystem_builder:random_access_file;

import std;
import cc_ice_intern_sonic;

export namespace ice::builder {

class TF_RandomAccessFileOps
{
public:
    explicit TF_RandomAccessFileOps(
        const ::TF_StatusOps* Status_ops,
        const ::TF_StringOps* String_ops
    ) noexcept :
        m_handle{.plugin_data = this}
    {
        m_Status_ops = Status_ops;
        m_String_ops = String_ops;
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
    virtual void read(
        uint64_t offset,
        size_t n,
        char* buffer,
        int64_t* out_bytes_read,
        const ice::sonic::Status& out_status
    ) noexcept = 0;

    void get_generic_vtable(void (*create)(::TF_RandomAccessFile*)) noexcept
    {
        m_vtable = ::TF_RandomAccessFileOps{
            .struct_size = TF_OFFSET_OF_END(::TF_RandomAccessFileOps, read),

            .create = create,
            .destroy =
                [](TF_RandomAccessFile* handle) noexcept
            {
                auto& self = TF_RandomAccessFileOps::from_handle(handle);
                self.destroy();
            },
            .get_name =
                [](TF_RandomAccessFile* file, TF_String* out_name) noexcept
            {
                auto& self = TF_RandomAccessFileOps::from_handle(file);
                self.get_name(self.wrap(std::type_identity<ice::sonic::String>{}, out_name));
            },
            .read =
                [](TF_RandomAccessFile* file,
                   uint64_t offset,
                   size_t n,
                   char* buffer,
                   int64_t* out_bytes_read,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_RandomAccessFileOps::from_handle(file);
                self.read(
                    offset,
                    n,
                    buffer,
                    out_bytes_read,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },

        };
    }

    ice::sonic::Status
    wrap(std::type_identity<ice::sonic::Status>, const ::TF_Status* handle) const noexcept
    {
        return ice::sonic::Status{m_Status_ops, const_cast<::TF_Status*>(handle)};
    }

    ice::sonic::String
    wrap(std::type_identity<ice::sonic::String>, const ::TF_String* handle) const noexcept
    {
        return ice::sonic::String{m_String_ops, const_cast<::TF_String*>(handle)};
    }

    const ::TF_RandomAccessFileOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const ::TF_RandomAccessFile& get_handle() const noexcept
    {
        return m_handle;
    }

    void register_ops(
        const ::TF_RegistrationOps& registry_ops,
        ::TF_Registration* registry_handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) const noexcept
    {
        registry_ops.register_op(
            registry_handle,
            type.get_handle(),
            provider.get_handle(),
            const_cast<::TF_RandomAccessFileOps*>(&m_vtable)
        );
    }

private:
    ::TF_RandomAccessFileOps m_vtable;
    ::TF_RandomAccessFile m_handle;

    const ::TF_StatusOps* m_Status_ops{nullptr};

    const ::TF_StringOps* m_String_ops{nullptr};
};

} // namespace ice::builder
