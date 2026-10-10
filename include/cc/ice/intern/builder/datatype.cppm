// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/intern/datatype.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/registration/registration.h"
#include "include/c/intern/datatype.h"
#include "include/c/intern/tstring.h"

export module cc_ice_intern_builder:datatype;

import std;
import cc_ice_intern_sonic;

export namespace ice::builder {

class TF_DataTypeOps
{
public:
    explicit TF_DataTypeOps(const ::TF_StringOps* String_ops) noexcept :
        m_handle{.plugin_data = this}
    {
        m_String_ops = String_ops;
    }

    TF_DataTypeOps(const TF_DataTypeOps&) = delete;
    TF_DataTypeOps& operator=(const TF_DataTypeOps&) = delete;

    static TF_DataTypeOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TF_DataTypeOps*>(ctx);
    }

    template<typename HandleT>
    static TF_DataTypeOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TF_DataTypeOps*>(handle->plugin_data);
    }

    virtual ~TF_DataTypeOps() = default;
    virtual void destroy() noexcept = 0;
    virtual void get_name(const ice::sonic::String& out_name) noexcept = 0;
    virtual void datatype_size(TFDataTypeEnum dt, size_t* out_size) noexcept = 0;

    void get_generic_vtable(void (*create)(::TF_DataType*)) noexcept
    {
        m_vtable = ::TF_DataTypeOps{
            .struct_size = TF_OFFSET_OF_END(::TF_DataTypeOps, datatype_size),

            .create = create,
            .destroy =
                [](TF_DataType* handle) noexcept
            {
                auto& self = TF_DataTypeOps::from_handle(handle);
                self.destroy();
            },
            .get_name =
                [](TF_DataType* datatype, TF_String* out_name) noexcept
            {
                auto& self = TF_DataTypeOps::from_handle(datatype);
                self.get_name(self.wrap(std::type_identity<ice::sonic::String>{}, out_name));
            },
            .datatype_size =
                [](TF_DataType* datatype, TFDataTypeEnum dt, size_t* out_size) noexcept
            {
                auto& self = TF_DataTypeOps::from_handle(datatype);
                self.datatype_size(dt, out_size);
            },

        };
    }

    ice::sonic::String
    wrap(std::type_identity<ice::sonic::String>, const ::TF_String* handle) const noexcept
    {
        return ice::sonic::String{m_String_ops, const_cast<::TF_String*>(handle)};
    }

    const ::TF_DataTypeOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const ::TF_DataType& get_handle() const noexcept
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
            const_cast<::TF_DataTypeOps*>(&m_vtable)
        );
    }

private:
    ::TF_DataTypeOps m_vtable;
    ::TF_DataType m_handle;

    const ::TF_StringOps* m_String_ops{nullptr};
};

} // namespace ice::builder
