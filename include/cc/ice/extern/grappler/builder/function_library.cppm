// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/grappler/function_library.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/grappler/function_library.h"
#include "include/c/intern/buffer.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"

export module cc_ice_extern_grappler_builder:function_library;

import std;
import cc_ice_intern_sonic;

export namespace ice::builder {

class TFGrapplerFunctionLibraryOps
{
public:
    explicit TFGrapplerFunctionLibraryOps(
        const ::TF_BufferOps* TF_BufferOps_ops,
        const ::TF_StatusOps* Status_ops,
        const ::TF_StringOps* String_ops
    ) noexcept :
        m_handle{.plugin_data = this}
    {
        m_TF_BufferOps_ops = TF_BufferOps_ops;
        m_Status_ops = Status_ops;
        m_String_ops = String_ops;
    }

    TFGrapplerFunctionLibraryOps(const TFGrapplerFunctionLibraryOps&) = delete;
    TFGrapplerFunctionLibraryOps& operator=(const TFGrapplerFunctionLibraryOps&) = delete;

    static TFGrapplerFunctionLibraryOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TFGrapplerFunctionLibraryOps*>(ctx);
    }

    template<typename HandleT>
    static TFGrapplerFunctionLibraryOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TFGrapplerFunctionLibraryOps*>(handle->plugin_data);
    }

    virtual ~TFGrapplerFunctionLibraryOps() = default;
    virtual void destroy() noexcept = 0;
    virtual void look_up_op_def(
        const ice::sonic::String& name,
        const ice::sonic::TF_BufferOps& out_buf,
        const ice::sonic::Status& out_status
    ) noexcept = 0;

    void get_generic_vtable(void (*create)(::TFGrapplerFunctionLibrary*)) noexcept
    {
        m_vtable = ::TFGrapplerFunctionLibraryOps{
            .struct_size = TF_OFFSET_OF_END(::TFGrapplerFunctionLibraryOps, look_up_op_def),

            .create = create,
            .destroy =
                [](TFGrapplerFunctionLibrary* handle) noexcept
            {
                auto& self = TFGrapplerFunctionLibraryOps::from_handle(handle);
                self.destroy();
            },
            .look_up_op_def =
                [](TFGrapplerFunctionLibrary* lib,
                   const TF_String* name,
                   TF_Buffer* out_buf,
                   TF_Status* out_status) noexcept
            {
                auto& self = TFGrapplerFunctionLibraryOps::from_handle(lib);
                self.look_up_op_def(
                    self.wrap(std::type_identity<ice::sonic::String>{}, name),
                    self.wrap(std::type_identity<ice::sonic::TF_BufferOps>{}, out_buf),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },

        };
    }

    ice::sonic::TF_BufferOps
    wrap(std::type_identity<ice::sonic::TF_BufferOps>, const ::TF_Buffer* handle) const noexcept
    {
        return ice::sonic::TF_BufferOps{m_TF_BufferOps_ops, const_cast<::TF_Buffer*>(handle)};
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

    const ::TFGrapplerFunctionLibraryOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const ::TFGrapplerFunctionLibrary& get_handle() const noexcept
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
        registry
            .register_op(type, provider, const_cast<::TFGrapplerFunctionLibraryOps*>(&m_vtable));
    }

private:
    ::TFGrapplerFunctionLibraryOps m_vtable;
    ::TFGrapplerFunctionLibrary m_handle;

    const ::TF_BufferOps* m_TF_BufferOps_ops{nullptr};

    const ::TF_StatusOps* m_Status_ops{nullptr};

    const ::TF_StringOps* m_String_ops{nullptr};
};

} // namespace ice::builder
