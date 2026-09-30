// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/generator/node.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/generator/node.h"

export module cc_ice_extern_generator_builder:node;

import std;

export namespace ice::builder {

class TFGeneratorNodeOps
{
public:
    TFGeneratorNodeOps() noexcept :
        m_handle{.plugin_data = this}
    {
    }

    TFGeneratorNodeOps(const TFGeneratorNodeOps&) = delete;
    TFGeneratorNodeOps& operator=(const TFGeneratorNodeOps&) = delete;

    static TFGeneratorNodeOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TFGeneratorNodeOps*>(ctx);
    }

    template<typename HandleT>
    static TFGeneratorNodeOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TFGeneratorNodeOps*>(handle->plugin_data);
    }

    virtual ~TFGeneratorNodeOps() = default;
    virtual void destroy() noexcept = 0;
    virtual void get_name(const ice::sonic::String& out_name) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    set_operand(int index, const ice::sonic::String& var_name) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    set_output_name(int index, const ice::sonic::String& var_name) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    set_attr(const ice::sonic::String& name, const void* value, size_t value_size) noexcept = 0;
    virtual void get_operand(int index, const ice::sonic::String& out_operand) noexcept = 0;
    virtual void get_output_name(int index, const ice::sonic::String& out_output_name) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    get_definition(const ice::sonic::TFGeneratorDefinitionOps& out_definition) noexcept = 0;

    void get_generic_vtable() noexcept
    {
        m_vtable = ::TFGeneratorNodeOps{
            .struct_size = TF_ENERATORNODE_STRUCT_SIZE,
            .destroy =
                [](TFGeneratorNode* node_context) noexcept
            {
                TFGeneratorNodeOps::from_handle(node_context).destroy();
            },
            .get_name =
                [](TFGeneratorNode* node_context, TF_String* out_name) noexcept
            {
                TFGeneratorNodeOps::from_handle(node_context)
                    .get_name(ice::sonic::String::wrap(out_name));
            },
            .set_operand =
                [](TFGeneratorNode* node_context,
                   int index,
                   const TF_String* var_name,
                   TF_Status* out_status) noexcept
            {
                auto res = TFGeneratorNodeOps::from_handle(node_context)
                               .set_operand(index, ice::sonic::String::wrap(var_name));
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .set_output_name =
                [](TFGeneratorNode* node_context,
                   int index,
                   const TF_String* var_name,
                   TF_Status* out_status) noexcept
            {
                auto res = TFGeneratorNodeOps::from_handle(node_context)
                               .set_output_name(index, ice::sonic::String::wrap(var_name));
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .set_attr =
                [](TFGeneratorNode* node_context,
                   const TF_String* name,
                   const void* value,
                   size_t value_size,
                   TF_Status* out_status) noexcept
            {
                auto res = TFGeneratorNodeOps::from_handle(node_context)
                               .set_attr(ice::sonic::String::wrap(name), value, value_size);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_operand =
                [](TFGeneratorNode* node_context, int index, TF_String* out_operand) noexcept
            {
                TFGeneratorNodeOps::from_handle(node_context)
                    .get_operand(index, ice::sonic::String::wrap(out_operand));
            },
            .get_output_name =
                [](TFGeneratorNode* node_context, int index, TF_String* out_output_name) noexcept
            {
                TFGeneratorNodeOps::from_handle(node_context)
                    .get_output_name(index, ice::sonic::String::wrap(out_output_name));
            },
            .get_definition =
                [](TFGeneratorNode* node_context,
                   TFGeneratorDefinition* out_definition,
                   TF_Status* out_status) noexcept
            {
                auto res =
                    TFGeneratorNodeOps::from_handle(node_context)
                        .get_definition(ice::sonic::TFGeneratorDefinitionOps::wrap(out_definition));
                if (!res) {
                    res.error().to_c(out_status);
                }
            },

        };
    }

    const ::TFGeneratorNodeOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const TFGeneratorNode& get_handle() const noexcept
    {
        return m_handle;
    }


private:
    ::TFGeneratorNodeOps m_vtable;
    TFGeneratorNode m_handle;
};

} // namespace ice::builder
