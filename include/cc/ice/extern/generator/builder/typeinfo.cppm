// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/generator/typeinfo.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/generator/typeinfo.h"
#include "include/c/extern/registration/registration.h"
#include "include/c/intern/tstring.h"

export module cc_ice_extern_generator_builder:typeinfo;

import std;
import cc_ice_intern_sonic;

export namespace ice::builder {

class TF_TypeInfoOps
{
public:
    explicit TF_TypeInfoOps(const ::TF_StringOps* String_ops) noexcept :
        m_handle{.plugin_data = this}
    {
        m_String_ops = String_ops;
    }

    TF_TypeInfoOps(const TF_TypeInfoOps&) = delete;
    TF_TypeInfoOps& operator=(const TF_TypeInfoOps&) = delete;

    static TF_TypeInfoOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TF_TypeInfoOps*>(ctx);
    }

    template<typename HandleT>
    static TF_TypeInfoOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TF_TypeInfoOps*>(handle->plugin_data);
    }

    virtual ~TF_TypeInfoOps() = default;
    virtual void destroy() noexcept = 0;
    virtual void get_name(const ice::sonic::String& out_name) noexcept = 0;
    virtual void set_type_attr_name(const ice::sonic::String& type_attr_name) noexcept = 0;
    virtual void set_data_type(int data_type) noexcept = 0;
    virtual void set_read_only(_Bool read_only) noexcept = 0;
    virtual void set_list(_Bool is_list) noexcept = 0;
    virtual void get_type_attr_name(const ice::sonic::String& out_type_attr_name) noexcept = 0;
    virtual void get_data_type(int* out_data_type) noexcept = 0;
    virtual void is_read_only(int* out_is_read_only) noexcept = 0;
    virtual void is_list(int* out_is_list) noexcept = 0;

    void get_generic_vtable(void (*create)(::TF_TypeInfo*)) noexcept
    {
        m_vtable = ::TF_TypeInfoOps{
            .struct_size = TF_OFFSET_OF_END(::TF_TypeInfoOps, is_list),

            .create = create,
            .destroy =
                [](TF_TypeInfo* handle) noexcept
            {
                auto& self = TF_TypeInfoOps::from_handle(handle);
                self.destroy();
            },
            .get_name =
                [](TF_TypeInfo* type_context, TF_String* out_name) noexcept
            {
                auto& self = TF_TypeInfoOps::from_handle(type_context);
                self.get_name(self.wrap(std::type_identity<ice::sonic::String>{}, out_name));
            },
            .set_type_attr_name =
                [](TF_TypeInfo* type_context, const TF_String* type_attr_name) noexcept
            {
                auto& self = TF_TypeInfoOps::from_handle(type_context);
                self.set_type_attr_name(
                    self.wrap(std::type_identity<ice::sonic::String>{}, type_attr_name)
                );
            },
            .set_data_type =
                [](TF_TypeInfo* type_context, int data_type) noexcept
            {
                auto& self = TF_TypeInfoOps::from_handle(type_context);
                self.set_data_type(data_type);
            },
            .set_read_only =
                [](TF_TypeInfo* type_context, _Bool read_only) noexcept
            {
                auto& self = TF_TypeInfoOps::from_handle(type_context);
                self.set_read_only(read_only);
            },
            .set_list =
                [](TF_TypeInfo* type_context, _Bool is_list) noexcept
            {
                auto& self = TF_TypeInfoOps::from_handle(type_context);
                self.set_list(is_list);
            },
            .get_type_attr_name =
                [](TF_TypeInfo* type_context, TF_String* out_type_attr_name) noexcept
            {
                auto& self = TF_TypeInfoOps::from_handle(type_context);
                self.get_type_attr_name(
                    self.wrap(std::type_identity<ice::sonic::String>{}, out_type_attr_name)
                );
            },
            .get_data_type =
                [](TF_TypeInfo* type_context, int* out_data_type) noexcept
            {
                auto& self = TF_TypeInfoOps::from_handle(type_context);
                self.get_data_type(out_data_type);
            },
            .is_read_only =
                [](TF_TypeInfo* type_context, int* out_is_read_only) noexcept
            {
                auto& self = TF_TypeInfoOps::from_handle(type_context);
                self.is_read_only(out_is_read_only);
            },
            .is_list =
                [](TF_TypeInfo* type_context, int* out_is_list) noexcept
            {
                auto& self = TF_TypeInfoOps::from_handle(type_context);
                self.is_list(out_is_list);
            },

        };
    }

    ice::sonic::String
    wrap(std::type_identity<ice::sonic::String>, const ::TF_String* handle) const noexcept
    {
        return ice::sonic::String{m_String_ops, const_cast<::TF_String*>(handle)};
    }

    const ::TF_TypeInfoOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const ::TF_TypeInfo& get_handle() const noexcept
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
            const_cast<::TF_TypeInfoOps*>(&m_vtable)
        );
    }

private:
    ::TF_TypeInfoOps m_vtable;
    ::TF_TypeInfo m_handle;

    const ::TF_StringOps* m_String_ops{nullptr};
};

} // namespace ice::builder
