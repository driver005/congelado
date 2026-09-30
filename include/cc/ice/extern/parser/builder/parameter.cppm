// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/parser/parameter.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/parser/parameter.h"

export module cc_ice_extern_parser_builder:parameter;

import std;

export namespace ice::builder {

class TFParserParameterOps
{
public:
    TFParserParameterOps() noexcept :
        m_handle{.plugin_data = this}
    {
    }

    TFParserParameterOps(const TFParserParameterOps&) = delete;
    TFParserParameterOps& operator=(const TFParserParameterOps&) = delete;

    static TFParserParameterOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TFParserParameterOps*>(ctx);
    }

    template<typename HandleT>
    static TFParserParameterOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TFParserParameterOps*>(handle->plugin_data);
    }

    virtual ~TFParserParameterOps() = default;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_typeinfo(const ice::sonic::TFParserTypeInfoOps& out_typeinfo) noexcept = 0;

    void get_generic_vtable() noexcept
    {
        m_vtable = ::TFParserParameterOps{
            .struct_size = TF_ARSERPARAMETER_STRUCT_SIZE,

            .get_name =
                [](void* plugin_context, TF_String* out) noexcept
            {
                auto result = TFParserParameterOps::from_handle(plugin_context).get_name();
                result.to_c(out);
            },
            .get_typeinfo =
                [](TFParserParameter* parameter,
                   TFParserTypeInfo* out_typeinfo,
                   TF_Status* out_status) noexcept
            {
                auto res = TFParserParameterOps::from_handle(parameter).get_typeinfo(
                    ice::sonic::TFParserTypeInfoOps::wrap(out_typeinfo)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },

        };
    }

    const ::TFParserParameterOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    virtual builder::String get_name() const noexcept = 0;

    const TFParserParameter& get_handle() const noexcept
    {
        return m_handle;
    }


private:
    ::TFParserParameterOps m_vtable;
    TFParserParameter m_handle;
};

} // namespace ice::builder
