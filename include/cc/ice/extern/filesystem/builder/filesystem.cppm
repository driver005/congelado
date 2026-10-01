// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/filesystem/filesystem.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/filesystem/filesystem.h"
#include "include/c/intern/tstring.h"

export module cc_ice_extern_filesystem_builder:filesystem;

import std;
import cc_ice_intern_sonic;

export namespace ice::builder {

class TF_FilesystemOps
{
public:
    explicit TF_FilesystemOps(const ::TF_StringOps* String_ops) noexcept :
        m_handle{.plugin_data = this}
    {
        m_String_ops = String_ops;
    }

    TF_FilesystemOps(const TF_FilesystemOps&) = delete;
    TF_FilesystemOps& operator=(const TF_FilesystemOps&) = delete;

    static TF_FilesystemOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TF_FilesystemOps*>(ctx);
    }

    template<typename HandleT>
    static TF_FilesystemOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TF_FilesystemOps*>(handle->plugin_data);
    }

    virtual ~TF_FilesystemOps() = default;
    virtual void destroy() noexcept = 0;
    virtual void get_name(const ice::sonic::String& out_name) noexcept = 0;

    void get_generic_vtable(void (*create)(::TF_Filesystem*)) noexcept
    {
        m_vtable = ::TF_FilesystemOps{
            .struct_size = TF_OFFSET_OF_END(::TF_FilesystemOps, get_name),

            .create = create,
            .destroy =
                [](TF_Filesystem* handle) noexcept
            {
                auto& self = TF_FilesystemOps::from_handle(handle);
                self.destroy();
            },
            .get_name =
                [](TF_Filesystem* filesystem, TF_String* out_name) noexcept
            {
                auto& self = TF_FilesystemOps::from_handle(filesystem);
                self.get_name(self.wrap(std::type_identity<ice::sonic::String>{}, out_name));
            },

        };
    }

    ice::sonic::String
    wrap(std::type_identity<ice::sonic::String>, const ::TF_String* handle) const noexcept
    {
        return ice::sonic::String{m_String_ops, const_cast<::TF_String*>(handle)};
    }

    const ::TF_FilesystemOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const ::TF_Filesystem& get_handle() const noexcept
    {
        return m_handle;
    }

    template<typename Registry, typename StringType>
    void register_ops(
        Registry& registry,
        const StringType& type,
        const StringType& provider
    ) const noexcept
    {
        registry.register_op(type, provider, const_cast<::TF_FilesystemOps*>(&m_vtable));
    }

private:
    ::TF_FilesystemOps m_vtable;
    ::TF_Filesystem m_handle;

    const ::TF_StringOps* m_String_ops{nullptr};
};

} // namespace ice::builder
