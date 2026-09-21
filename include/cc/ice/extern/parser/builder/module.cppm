// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/parser/module.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/parser/module.h"

export module cc_ice_builder_parser:module;

import std;

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
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_function_count(int* out_count) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_function(int index, const ice::sonic::TFParserFunctionOps& out_function) noexcept = 0;

    static TFParserModuleOps* get_generic_vtable()
    {
        static TFParserModuleOps vtable = {
            .struct_size = TF_ARSERMODULE_STRUCT_SIZE,

            .get_name =
                [](void* plugin_context, TF_String* out) noexcept
            {
                auto* self = TFParserModuleOps::create(plugin_context);
                auto result = self->get_name();
                result.to_c(out);
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
                auto res =
                    self->get_function(index, ice::sonic::TFParserFunctionOps::wrap(out_function));
                if (!res) {
                    res.error().to_c(out_status);
                }
            },

        };

        return &vtable;
    }

    builder::String get_name() const noexcept
    {
        builder::String result;
        m_ops->get_name(get_handle(), result.get_handle());
        return result;
    }
};

} // namespace ice::builder
