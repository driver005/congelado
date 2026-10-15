// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/parser/function.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/parser/block.h"
#include "include/c/extern/parser/function.h"
#include "include/c/extern/parser/parameter.h"
#include "include/c/intern/registration.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"

export module cc_ice_extern_parser_builder:function;

import std;
import cc_ice_extern_parser_sonic;
import cc_ice_intern_sonic;

export namespace ice::builder {

class ParserFunction
{
public:
    explicit ParserFunction(
        const ::TFParserBlockOps* ParserBlock_ops,
        const ::TFParserParameterOps* ParserParameter_ops,
        const ::TF_StatusOps* Status_ops,
        const ::TF_StringOps* String_ops
    ) noexcept :
        m_handle{.plugin_data = this}
    {
        m_ParserBlock_ops = ParserBlock_ops;
        m_ParserParameter_ops = ParserParameter_ops;
        m_Status_ops = Status_ops;
        m_String_ops = String_ops;
    }

    ParserFunction(const ParserFunction&) = delete;
    ParserFunction& operator=(const ParserFunction&) = delete;

    static ParserFunction& from_handle(void* ctx) noexcept
    {
        return *static_cast<ParserFunction*>(ctx);
    }

    template<typename HandleT>
    static ParserFunction& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<ParserFunction*>(handle->plugin_data);
    }

    virtual ~ParserFunction() = default;
    virtual void destroy() noexcept = 0;
    virtual void
    get_name(const ice::sonic::String& out_name, const ice::sonic::Status& out_status) noexcept = 0;
    virtual void
    get_parameter_count(int& out_count, const ice::sonic::Status& out_status) noexcept = 0;
    virtual void get_parameter(
        int index,
        const ice::sonic::ParserParameter& out_parameter,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void get_block_count(int& out_count, const ice::sonic::Status& out_status) noexcept = 0;
    virtual void get_block(
        int index,
        const ice::sonic::ParserBlock& out_block,
        const ice::sonic::Status& out_status
    ) noexcept = 0;

    void get_generic_vtable(void (*create)(::TFParserFunction*)) noexcept
    {
        m_vtable = ::TFParserFunctionOps{
            .struct_size = TF_OFFSET_OF_END(::TFParserFunctionOps, get_block),

            .create = create,
            .destroy =
                [](::TFParserFunction* handle) noexcept
            {
                auto& self = ParserFunction::from_handle(handle);
                self.destroy();
            },
            .get_name =
                [](::TFParserFunction* function,
                   ::TF_String* out_name,
                   ::TF_Status* out_status) noexcept
            {
                auto& self = ParserFunction::from_handle(function);
                self.get_name(
                    self.wrap(std::type_identity<ice::sonic::String>{}, out_name),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_parameter_count =
                [](::TFParserFunction* function, int* out_count, ::TF_Status* out_status) noexcept
            {
                auto& self = ParserFunction::from_handle(function);
                self.get_parameter_count(
                    *out_count,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_parameter =
                [](::TFParserFunction* function,
                   int index,
                   ::TFParserParameter* out_parameter,
                   ::TF_Status* out_status) noexcept
            {
                auto& self = ParserFunction::from_handle(function);
                self.get_parameter(
                    index,
                    self.wrap(std::type_identity<ice::sonic::ParserParameter>{}, out_parameter),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_block_count =
                [](::TFParserFunction* function, int* out_count, ::TF_Status* out_status) noexcept
            {
                auto& self = ParserFunction::from_handle(function);
                self.get_block_count(
                    *out_count,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_block =
                [](::TFParserFunction* function,
                   int index,
                   ::TFParserBlock* out_block,
                   ::TF_Status* out_status) noexcept
            {
                auto& self = ParserFunction::from_handle(function);
                self.get_block(
                    index,
                    self.wrap(std::type_identity<ice::sonic::ParserBlock>{}, out_block),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },

        };
    }

    ice::sonic::ParserBlock
    wrap(std::type_identity<ice::sonic::ParserBlock>, const ::TFParserBlock* handle) const noexcept
    {
        return ice::sonic::ParserBlock{m_ParserBlock_ops, const_cast<::TFParserBlock*>(handle)};
    }

    ice::sonic::ParserParameter wrap(
        std::type_identity<ice::sonic::ParserParameter>,
        const ::TFParserParameter* handle
    ) const noexcept
    {
        return ice::sonic::ParserParameter{
            m_ParserParameter_ops,
            const_cast<::TFParserParameter*>(handle)
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

    const ::TFParserFunctionOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const ::TFParserFunction& get_handle() const noexcept
    {
        return m_handle;
    }

    void register_ops(
        const ice::sonic::Registration& registry,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) const noexcept
    {
        registry.register_op(type, provider, const_cast<::TFParserFunctionOps*>(&m_vtable));
    }

private:
    ::TFParserFunctionOps m_vtable;
    ::TFParserFunction m_handle;

    const ::TFParserBlockOps* m_ParserBlock_ops{nullptr};

    const ::TFParserParameterOps* m_ParserParameter_ops{nullptr};

    const ::TF_StatusOps* m_Status_ops{nullptr};

    const ::TF_StringOps* m_String_ops{nullptr};
};

} // namespace ice::builder
