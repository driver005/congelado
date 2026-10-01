// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/parser/function.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/parser/block.h"
#include "include/c/extern/parser/function.h"
#include "include/c/extern/parser/parameter.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"

export module cc_ice_extern_parser_builder:function;

import std;
import cc_ice_extern_parser_sonic;
import cc_ice_intern_sonic;

export namespace ice::builder {

class TFParserFunctionOps
{
public:
    explicit TFParserFunctionOps(
        const ::TFParserBlockOps* TFParserBlockOps_ops,
        const ::TFParserParameterOps* TFParserParameterOps_ops,
        const ::TF_StatusOps* Status_ops,
        const ::TF_StringOps* String_ops
    ) noexcept :
        m_handle{.plugin_data = this}
    {
        m_TFParserBlockOps_ops = TFParserBlockOps_ops;
        m_TFParserParameterOps_ops = TFParserParameterOps_ops;
        m_Status_ops = Status_ops;
        m_String_ops = String_ops;
    }

    TFParserFunctionOps(const TFParserFunctionOps&) = delete;
    TFParserFunctionOps& operator=(const TFParserFunctionOps&) = delete;

    static TFParserFunctionOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TFParserFunctionOps*>(ctx);
    }

    template<typename HandleT>
    static TFParserFunctionOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TFParserFunctionOps*>(handle->plugin_data);
    }

    virtual ~TFParserFunctionOps() = default;
    virtual void destroy() noexcept = 0;
    virtual void
    get_name(const ice::sonic::String& out_name, const ice::sonic::Status& out_status) noexcept = 0;
    virtual void
    get_parameter_count(int* out_count, const ice::sonic::Status& out_status) noexcept = 0;
    virtual void get_parameter(
        int index,
        const ice::sonic::TFParserParameterOps& out_parameter,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void get_block_count(int* out_count, const ice::sonic::Status& out_status) noexcept = 0;
    virtual void get_block(
        int index,
        const ice::sonic::TFParserBlockOps& out_block,
        const ice::sonic::Status& out_status
    ) noexcept = 0;

    void get_generic_vtable(void (*create)(::TFParserFunction*)) noexcept
    {
        m_vtable = ::TFParserFunctionOps{
            .struct_size = TF_OFFSET_OF_END(::TFParserFunctionOps, get_block),

            .create = create,
            .destroy =
                [](TFParserFunction* handle) noexcept
            {
                auto& self = TFParserFunctionOps::from_handle(handle);
                self.destroy();
            },
            .get_name =
                [](TFParserFunction* function, TF_String* out_name, TF_Status* out_status) noexcept
            {
                auto& self = TFParserFunctionOps::from_handle(function);
                self.get_name(
                    self.wrap(std::type_identity<ice::sonic::String>{}, out_name),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_parameter_count =
                [](TFParserFunction* function, int* out_count, TF_Status* out_status) noexcept
            {
                auto& self = TFParserFunctionOps::from_handle(function);
                self.get_parameter_count(
                    out_count,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_parameter =
                [](TFParserFunction* function,
                   int index,
                   TFParserParameter* out_parameter,
                   TF_Status* out_status) noexcept
            {
                auto& self = TFParserFunctionOps::from_handle(function);
                self.get_parameter(
                    index,
                    self.wrap(
                        std::type_identity<ice::sonic::TFParserParameterOps>{},
                        out_parameter
                    ),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_block_count =
                [](TFParserFunction* function, int* out_count, TF_Status* out_status) noexcept
            {
                auto& self = TFParserFunctionOps::from_handle(function);
                self.get_block_count(
                    out_count,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_block =
                [](TFParserFunction* function,
                   int index,
                   TFParserBlock* out_block,
                   TF_Status* out_status) noexcept
            {
                auto& self = TFParserFunctionOps::from_handle(function);
                self.get_block(
                    index,
                    self.wrap(std::type_identity<ice::sonic::TFParserBlockOps>{}, out_block),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },

        };
    }

    ice::sonic::TFParserBlockOps wrap(
        std::type_identity<ice::sonic::TFParserBlockOps>,
        const ::TFParserBlock* handle
    ) const noexcept
    {
        return ice::sonic::TFParserBlockOps{
            m_TFParserBlockOps_ops,
            const_cast<::TFParserBlock*>(handle)
        };
    }

    ice::sonic::TFParserParameterOps wrap(
        std::type_identity<ice::sonic::TFParserParameterOps>,
        const ::TFParserParameter* handle
    ) const noexcept
    {
        return ice::sonic::TFParserParameterOps{
            m_TFParserParameterOps_ops,
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

    template<typename Registry, typename StringType>
    void register_ops(
        Registry& registry,
        const StringType& type,
        const StringType& provider
    ) const noexcept
    {
        registry.register_op(type, provider, const_cast<::TFParserFunctionOps*>(&m_vtable));
    }

private:
    ::TFParserFunctionOps m_vtable;
    ::TFParserFunction m_handle;

    const ::TFParserBlockOps* m_TFParserBlockOps_ops{nullptr};

    const ::TFParserParameterOps* m_TFParserParameterOps_ops{nullptr};

    const ::TF_StatusOps* m_Status_ops{nullptr};

    const ::TF_StringOps* m_String_ops{nullptr};
};

} // namespace ice::builder
