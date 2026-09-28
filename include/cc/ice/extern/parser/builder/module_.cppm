// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/parser/module.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/parser/module.h"
inline constexpr auto k_struct_size_TFParserModuleOps =
    TF_OFFSET_OF_END(TFParserModuleOps, get_function);

export module cc_ice_extern_parser_builder:module_;

import std;
import cc_ice_support;
import c_intern;
import c_intern;
import c_intern;
import :function;
import c_intern;
import c_intern;
import :parameter;
import c_intern;
import c_intern;
import :typeinfo;
import c_intern;
import c_intern;
import c_intern;
import c_intern;
import c_intern;
import c_intern;
import c_intern;
import c_intern;
import :block;
import c_intern;
import c_intern;
import :node;
import c_intern;
import c_intern;
import :attribute;
import c_intern;
import c_intern;
import c_intern;
import :definition;
import c_intern;
import c_intern;

export namespace ice::builder {

class TFParserModuleOps
{
public:
    static TFParserModuleOps* create(void* ctx) noexcept
    {
        return static_cast<TFParserModuleOps*>(ctx);
    }

    template<typename HandleT>
    static TFParserModuleOps* create(HandleT* handle) noexcept
    {
        return static_cast<TFParserModuleOps*>(handle->plugin_data);
    }

    virtual ~TFParserModuleOps() = default;
    [[nodiscard]] virtual std::expected<void, TF_Status>
    get_name(const ice::sonic::TF_StringOps& out_name) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, TF_Status>
    get_function_count(int* out_count) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, TF_Status>
    get_function(int index, const ice::sonic::TFParserFunctionOps& out_function) noexcept = 0;

    static ::TFParserModuleOps* get_generic_vtable()
    {
        static ::TFParserModuleOps vtable = {
            .struct_size = k_struct_size_TFParserModuleOps,
            .get_name =
                [](TFParserModule* module, TF_String* out_name, TF_Status* out_status) noexcept
            {
                auto* self = TFParserModuleOps::create(module);
                auto res = self->get_name(*ice::builder::TF_StringOps::create(out_name));
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_function_count =
                [](TFParserModule* module, int* out_count, TF_Status* out_status) noexcept
            {
                auto* self = TFParserModuleOps::create(module);
                auto res = self->get_function_count(out_count);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_function =
                [](TFParserModule* module,
                   int index,
                   TFParserFunction* out_function,
                   TF_Status* out_status) noexcept
            {
                auto* self = TFParserModuleOps::create(module);
                auto res = self->get_function(
                    index,
                    *ice::builder::TFParserFunctionOps::create(out_function)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
        };

        return &vtable;
    }
};

} // namespace ice::builder
