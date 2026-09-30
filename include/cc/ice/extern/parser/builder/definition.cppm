// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/parser/definition.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/parser/definition.h"

export module cc_ice_extern_parser_builder:definition;

import std;

export namespace ice::builder {

class TFParserDefinitionOps
{
public:
    TFParserDefinitionOps() noexcept :
        m_handle{.plugin_data = this}
    {
    }

    TFParserDefinitionOps(const TFParserDefinitionOps&) = delete;
    TFParserDefinitionOps& operator=(const TFParserDefinitionOps&) = delete;

    static TFParserDefinitionOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TFParserDefinitionOps*>(ctx);
    }

    template<typename HandleT>
    static TFParserDefinitionOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TFParserDefinitionOps*>(handle->plugin_data);
    }

    virtual ~TFParserDefinitionOps() = default;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    get_name(const ice::sonic::String& out_name) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    get_source_file(const ice::sonic::String& out_source_file) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    get_line_number(int* out_line_number) noexcept = 0;

    void get_generic_vtable() noexcept
    {
        m_vtable = ::TFParserDefinitionOps{
            .struct_size = TF_ARSERDEFINITION_STRUCT_SIZE,
            .get_name =
                [](TFParserDefinition* definition,
                   TF_String* out_name,
                   TF_Status* out_status) noexcept
            {
                auto res = TFParserDefinitionOps::from_handle(definition)
                               .get_name(ice::sonic::String::wrap(out_name));
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_source_file =
                [](TFParserDefinition* definition,
                   TF_String* out_source_file,
                   TF_Status* out_status) noexcept
            {
                auto res = TFParserDefinitionOps::from_handle(definition)
                               .get_source_file(ice::sonic::String::wrap(out_source_file));
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_line_number =
                [](TFParserDefinition* definition,
                   int* out_line_number,
                   TF_Status* out_status) noexcept
            {
                auto res =
                    TFParserDefinitionOps::from_handle(definition).get_line_number(out_line_number);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },

        };
    }

    const ::TFParserDefinitionOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const TFParserDefinition& get_handle() const noexcept
    {
        return m_handle;
    }


private:
    ::TFParserDefinitionOps m_vtable;
    TFParserDefinition m_handle;
};

} // namespace ice::builder
