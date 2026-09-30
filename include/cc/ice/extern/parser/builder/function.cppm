// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/parser/function.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/parser/function.h"

export module cc_ice_extern_parser_builder:function;

import std;

export namespace ice::builder {

class TFParserFunctionOps
{
public:
    TFParserFunctionOps() noexcept :
        m_handle{.plugin_data = this}
    {
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
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_parameter_count(int* out_count) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_parameter(int index, const ice::sonic::TFParserParameterOps& out_parameter) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_block_count(int* out_count) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_block(int index, const ice::sonic::TFParserBlockOps& out_block) noexcept = 0;

    void get_generic_vtable() noexcept
    {
        m_vtable = ::TFParserFunctionOps{
            .struct_size = TF_ARSERFUNCTION_STRUCT_SIZE,

            .get_name =
                [](void* plugin_context, TF_String* out) noexcept
            {
                auto result = TFParserFunctionOps::from_handle(plugin_context).get_name();
                result.to_c(out);
            },
            .get_parameter_count =
                [](TFParserFunction* function, int* out_count, TF_Status* out_status) noexcept
            {
                auto res =
                    TFParserFunctionOps::from_handle(function).get_parameter_count(out_count);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_parameter =
                [](TFParserFunction* function,
                   int index,
                   TFParserParameter* out_parameter,
                   TF_Status* out_status) noexcept
            {
                auto res = TFParserFunctionOps::from_handle(function).get_parameter(
                    index,
                    ice::sonic::TFParserParameterOps::wrap(out_parameter)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_block_count =
                [](TFParserFunction* function, int* out_count, TF_Status* out_status) noexcept
            {
                auto res = TFParserFunctionOps::from_handle(function).get_block_count(out_count);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_block =
                [](TFParserFunction* function,
                   int index,
                   TFParserBlock* out_block,
                   TF_Status* out_status) noexcept
            {
                auto res = TFParserFunctionOps::from_handle(function).get_block(
                    index,
                    ice::sonic::TFParserBlockOps::wrap(out_block)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },

        };
    }

    const ::TFParserFunctionOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    virtual builder::String get_name() const noexcept = 0;

    const TFParserFunction& get_handle() const noexcept
    {
        return m_handle;
    }


private:
    ::TFParserFunctionOps m_vtable;
    TFParserFunction m_handle;
};

} // namespace ice::builder
