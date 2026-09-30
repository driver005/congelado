// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/parser/node.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/parser/node.h"

export module cc_ice_extern_parser_builder:node;

import std;

export namespace ice::builder {

class TFParserNodeOps
{
public:
    TFParserNodeOps() noexcept :
        m_handle{.plugin_data = this}
    {
    }

    TFParserNodeOps(const TFParserNodeOps&) = delete;
    TFParserNodeOps& operator=(const TFParserNodeOps&) = delete;

    static TFParserNodeOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TFParserNodeOps*>(ctx);
    }

    template<typename HandleT>
    static TFParserNodeOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TFParserNodeOps*>(handle->plugin_data);
    }

    virtual ~TFParserNodeOps() = default;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    get_name(const ice::sonic::String& out_name) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    get_op_type(const ice::sonic::String& out_op_type) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    get_attribute_count(int* out_count) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    get_attribute(int index, const ice::sonic::TFParserAttributeOps& out_attribute) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    get_definition(const ice::sonic::TFParserDefinitionOps& out_definition) noexcept = 0;

    void get_generic_vtable() noexcept
    {
        m_vtable = ::TFParserNodeOps{
            .struct_size = TF_ARSERNODE_STRUCT_SIZE,
            .get_name =
                [](TFParserNode* node, TF_String* out_name, TF_Status* out_status) noexcept
            {
                auto res =
                    TFParserNodeOps::from_handle(node).get_name(ice::sonic::String::wrap(out_name));
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_op_type =
                [](TFParserNode* node, TF_String* out_op_type, TF_Status* out_status) noexcept
            {
                auto res = TFParserNodeOps::from_handle(node).get_op_type(
                    ice::sonic::String::wrap(out_op_type)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_attribute_count =
                [](TFParserNode* node, int* out_count, TF_Status* out_status) noexcept
            {
                auto res = TFParserNodeOps::from_handle(node).get_attribute_count(out_count);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_attribute =
                [](TFParserNode* node,
                   int index,
                   TFParserAttribute* out_attribute,
                   TF_Status* out_status) noexcept
            {
                auto res = TFParserNodeOps::from_handle(node).get_attribute(
                    index,
                    ice::sonic::TFParserAttributeOps::wrap(out_attribute)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_definition =
                [](TFParserNode* node,
                   TFParserDefinition* out_definition,
                   TF_Status* out_status) noexcept
            {
                auto res = TFParserNodeOps::from_handle(node).get_definition(
                    ice::sonic::TFParserDefinitionOps::wrap(out_definition)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },

        };
    }

    const ::TFParserNodeOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const TFParserNode& get_handle() const noexcept
    {
        return m_handle;
    }


private:
    ::TFParserNodeOps m_vtable;
    TFParserNode m_handle;
};

} // namespace ice::builder
