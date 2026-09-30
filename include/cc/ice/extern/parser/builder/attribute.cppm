// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/parser/attribute.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/parser/attribute.h"

export module cc_ice_extern_parser_builder:attribute;

import std;

export namespace ice::builder {

class TFParserAttributeOps
{
public:
    TFParserAttributeOps() noexcept :
        m_handle{.plugin_data = this}
    {
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
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_value(TF_Tensor** out_value) noexcept = 0;

    void get_generic_vtable() noexcept
    {
        m_vtable = ::TFParserAttributeOps{
            .struct_size = TF_ARSERATTRIBUTE_STRUCT_SIZE,

            .get_name =
                [](void* plugin_context, TF_String* out) noexcept
            {
                auto result = TFParserAttributeOps::from_handle(plugin_context).get_name();
                result.to_c(out);
            },
            .get_value =
                [](TFParserAttribute* attribute,
                   TF_Tensor** out_value,
                   TF_Status* out_status) noexcept
            {
                auto res = TFParserAttributeOps::from_handle(attribute).get_value(out_value);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },

        };
    }

    const ::TFParserAttributeOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    virtual builder::String get_name() const noexcept = 0;

    const TFParserAttribute& get_handle() const noexcept
    {
        return m_handle;
    }


private:
    ::TFParserAttributeOps m_vtable;
    TFParserAttribute m_handle;
};

} // namespace ice::builder
