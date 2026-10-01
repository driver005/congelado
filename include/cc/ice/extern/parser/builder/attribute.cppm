// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/parser/attribute.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/parser/attribute.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"

export module cc_ice_extern_parser_builder:attribute;

import std;
import cc_ice_intern_sonic;

export namespace ice::builder {

class TFParserAttributeOps
{
public:
    explicit TFParserAttributeOps(
        const ::TF_StatusOps* Status_ops,
        const ::TF_StringOps* String_ops
    ) noexcept :
        m_handle{.plugin_data = this}
    {
        m_Status_ops = Status_ops;
        m_String_ops = String_ops;
    }

    TFParserAttributeOps(const TFParserAttributeOps&) = delete;
    TFParserAttributeOps& operator=(const TFParserAttributeOps&) = delete;

    static TFParserAttributeOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TFParserAttributeOps*>(ctx);
    }

    template<typename HandleT>
    static TFParserAttributeOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TFParserAttributeOps*>(handle->plugin_data);
    }

    virtual ~TFParserAttributeOps() = default;
    virtual void destroy() noexcept = 0;
    virtual void
    get_name(const ice::sonic::String& out_name, const ice::sonic::Status& out_status) noexcept = 0;
    virtual void
    get_value(TF_Tensor** out_value, const ice::sonic::Status& out_status) noexcept = 0;

    void get_generic_vtable(void (*create)(::TFParserAttribute*)) noexcept
    {
        m_vtable = ::TFParserAttributeOps{
            .struct_size = TF_OFFSET_OF_END(::TFParserAttributeOps, get_value),

            .create = create,
            .destroy =
                [](TFParserAttribute* handle) noexcept
            {
                auto& self = TFParserAttributeOps::from_handle(handle);
                self.destroy();
            },
            .get_name =
                [](TFParserAttribute* attribute,
                   TF_String* out_name,
                   TF_Status* out_status) noexcept
            {
                auto& self = TFParserAttributeOps::from_handle(attribute);
                self.get_name(
                    self.wrap(std::type_identity<ice::sonic::String>{}, out_name),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_value =
                [](TFParserAttribute* attribute,
                   TF_Tensor** out_value,
                   TF_Status* out_status) noexcept
            {
                auto& self = TFParserAttributeOps::from_handle(attribute);
                self.get_value(
                    out_value,
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

    const ::TFParserAttributeOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const ::TFParserAttribute& get_handle() const noexcept
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
        registry.register_op(type, provider, const_cast<::TFParserAttributeOps*>(&m_vtable));
    }

private:
    ::TFParserAttributeOps m_vtable;
    ::TFParserAttribute m_handle;

    const ::TF_StatusOps* m_Status_ops{nullptr};

    const ::TF_StringOps* m_String_ops{nullptr};
};

} // namespace ice::builder
