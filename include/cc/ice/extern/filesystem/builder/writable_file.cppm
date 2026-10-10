// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/filesystem/writable_file.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/filesystem/writable_file.h"
#include "include/c/extern/registration/registration.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"

export module cc_ice_extern_filesystem_builder:writable_file;

import std;
import cc_ice_intern_sonic;

export namespace ice::builder {

class TF_WritableFileOps
{
public:
    explicit TF_WritableFileOps(
        const ::TF_StatusOps* Status_ops,
        const ::TF_StringOps* String_ops
    ) noexcept :
        m_handle{.plugin_data = this}
    {
        m_Status_ops = Status_ops;
        m_String_ops = String_ops;
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
    virtual void destroy() noexcept = 0;
    virtual void get_name(const ice::sonic::String& out_name) noexcept = 0;
    virtual void
    append(const ice::sonic::String& buffer, const ice::sonic::Status& out_status) noexcept = 0;
    virtual void tell(int64_t* out_position, const ice::sonic::Status& out_status) noexcept = 0;
    virtual void flush(const ice::sonic::Status& out_status) noexcept = 0;
    virtual void sync(const ice::sonic::Status& out_status) noexcept = 0;
    virtual void close(const ice::sonic::Status& out_status) noexcept = 0;

    void get_generic_vtable(void (*create)(::TF_WritableFile*)) noexcept
    {
        m_vtable = ::TF_WritableFileOps{
            .struct_size = TF_OFFSET_OF_END(::TF_WritableFileOps, close),

            .create = create,
            .destroy =
                [](TF_WritableFile* handle) noexcept
            {
                auto& self = TF_WritableFileOps::from_handle(handle);
                self.destroy();
            },
            .get_name =
                [](TF_WritableFile* file, TF_String* out_name) noexcept
            {
                auto& self = TF_WritableFileOps::from_handle(file);
                self.get_name(self.wrap(std::type_identity<ice::sonic::String>{}, out_name));
            },
            .append =
                [](TF_WritableFile* file, const TF_String* buffer, TF_Status* out_status) noexcept
            {
                auto& self = TF_WritableFileOps::from_handle(file);
                self.append(
                    self.wrap(std::type_identity<ice::sonic::String>{}, buffer),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .tell =
                [](TF_WritableFile* file, int64_t* out_position, TF_Status* out_status) noexcept
            {
                auto& self = TF_WritableFileOps::from_handle(file);
                self.tell(
                    out_position,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .flush =
                [](TF_WritableFile* file, TF_Status* out_status) noexcept
            {
                auto& self = TF_WritableFileOps::from_handle(file);
                self.flush(self.wrap(std::type_identity<ice::sonic::Status>{}, out_status));
            },
            .sync =
                [](TF_WritableFile* file, TF_Status* out_status) noexcept
            {
                auto& self = TF_WritableFileOps::from_handle(file);
                self.sync(self.wrap(std::type_identity<ice::sonic::Status>{}, out_status));
            },
            .close =
                [](TF_WritableFile* file, TF_Status* out_status) noexcept
            {
                auto& self = TF_WritableFileOps::from_handle(file);
                self.close(self.wrap(std::type_identity<ice::sonic::Status>{}, out_status));
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

    const ::TF_WritableFileOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const ::TF_WritableFile& get_handle() const noexcept
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
            const_cast<::TF_WritableFileOps*>(&m_vtable)
        );
    }

private:
    ::TF_WritableFileOps m_vtable;
    ::TF_WritableFile m_handle;

    const ::TF_StatusOps* m_Status_ops{nullptr};

    const ::TF_StringOps* m_String_ops{nullptr};
};

} // namespace ice::builder
