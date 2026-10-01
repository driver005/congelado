// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/parser/node.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/parser/attribute.h"
#include "include/c/extern/parser/definition.h"
#include "include/c/extern/parser/node.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"

export module cc_ice_extern_parser_builder:node;

import std;
import cc_ice_extern_parser_sonic;
import cc_ice_intern_sonic;

export namespace ice::builder {

class TFParserNodeOps
{
public:
    explicit TFParserNodeOps(
        const ::TFParserAttributeOps* TFParserAttributeOps_ops,
        const ::TFParserDefinitionOps* TFParserDefinitionOps_ops,
        const ::TF_StatusOps* Status_ops,
        const ::TF_StringOps* String_ops
    ) noexcept :
        m_handle{.plugin_data = this}
    {
        m_TFParserAttributeOps_ops = TFParserAttributeOps_ops;
        m_TFParserDefinitionOps_ops = TFParserDefinitionOps_ops;
        m_Status_ops = Status_ops;
        m_String_ops = String_ops;
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
    virtual void destroy() noexcept = 0;
    virtual void
    get_name(const ice::sonic::String& out_name, const ice::sonic::Status& out_status) noexcept = 0;
    virtual void get_op_type(
        const ice::sonic::String& out_op_type,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void
    get_attribute_count(int* out_count, const ice::sonic::Status& out_status) noexcept = 0;
    virtual void get_attribute(
        int index,
        const ice::sonic::TFParserAttributeOps& out_attribute,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void get_definition(
        const ice::sonic::TFParserDefinitionOps& out_definition,
        const ice::sonic::Status& out_status
    ) noexcept = 0;

    void get_generic_vtable(void (*create)(::TFParserNode*)) noexcept
    {
        m_vtable = ::TFParserNodeOps{
            .struct_size = TF_OFFSET_OF_END(::TFParserNodeOps, get_definition),

            .create = create,
            .destroy =
                [](TFParserNode* handle) noexcept
            {
                auto& self = TFParserNodeOps::from_handle(handle);
                self.destroy();
            },
            .get_name =
                [](TFParserNode* node, TF_String* out_name, TF_Status* out_status) noexcept
            {
                auto& self = TFParserNodeOps::from_handle(node);
                self.get_name(
                    self.wrap(std::type_identity<ice::sonic::String>{}, out_name),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_op_type =
                [](TFParserNode* node, TF_String* out_op_type, TF_Status* out_status) noexcept
            {
                auto& self = TFParserNodeOps::from_handle(node);
                self.get_op_type(
                    self.wrap(std::type_identity<ice::sonic::String>{}, out_op_type),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_attribute_count =
                [](TFParserNode* node, int* out_count, TF_Status* out_status) noexcept
            {
                auto& self = TFParserNodeOps::from_handle(node);
                self.get_attribute_count(
                    out_count,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_attribute =
                [](TFParserNode* node,
                   int index,
                   TFParserAttribute* out_attribute,
                   TF_Status* out_status) noexcept
            {
                auto& self = TFParserNodeOps::from_handle(node);
                self.get_attribute(
                    index,
                    self.wrap(
                        std::type_identity<ice::sonic::TFParserAttributeOps>{},
                        out_attribute
                    ),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_definition =
                [](TFParserNode* node,
                   TFParserDefinition* out_definition,
                   TF_Status* out_status) noexcept
            {
                auto& self = TFParserNodeOps::from_handle(node);
                self.get_definition(
                    self.wrap(
                        std::type_identity<ice::sonic::TFParserDefinitionOps>{},
                        out_definition
                    ),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },

        };
    }

    ice::sonic::TFParserAttributeOps wrap(
        std::type_identity<ice::sonic::TFParserAttributeOps>,
        const ::TFParserAttribute* handle
    ) const noexcept
    {
        return ice::sonic::TFParserAttributeOps{
            m_TFParserAttributeOps_ops,
            const_cast<::TFParserAttribute*>(handle)
        };
    }

    ice::sonic::TFParserDefinitionOps wrap(
        std::type_identity<ice::sonic::TFParserDefinitionOps>,
        const ::TFParserDefinition* handle
    ) const noexcept
    {
        return ice::sonic::TFParserDefinitionOps{
            m_TFParserDefinitionOps_ops,
            const_cast<::TFParserDefinition*>(handle)
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

    const ::TFParserNodeOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const ::TFParserNode& get_handle() const noexcept
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
        registry.register_op(type, provider, const_cast<::TFParserNodeOps*>(&m_vtable));
    }

private:
    ::TFParserNodeOps m_vtable;
    ::TFParserNode m_handle;

    const ::TFParserAttributeOps* m_TFParserAttributeOps_ops{nullptr};

    const ::TFParserDefinitionOps* m_TFParserDefinitionOps_ops{nullptr};

    const ::TF_StatusOps* m_Status_ops{nullptr};

    const ::TF_StringOps* m_String_ops{nullptr};
};

} // namespace ice::builder
