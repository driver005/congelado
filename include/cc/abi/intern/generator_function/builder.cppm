// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from c/extern/generator_function/generator_function.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "c/extern/generator_function/generator_function.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"

export module cc_abi_builder_generator_function;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;

export namespace ice::builder {

class Generator_function
{
public:
    static Generator_function* create(void* ctx) noexcept
    {
        return static_cast<Generator_function*>(ctx);
    }

    template<typename HandleT>
    static Generator_function* create(HandleT* handle) noexcept
    {
        return reinterpret_cast<Generator_function*>(handle);
    }

    virtual ~Generator_function() = default;
    [[nodiscard]] std::expected<void, ice::Status> destroy_function() noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    add_parameter(const ice::sonic::String& name, const ice::sonic::String& type_text) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> add_node(
        const TF_Generator_Definition* def_context,
        const ice::sonic::Tensor& operands,
        const ice::sonic::Tensor& attrs,
        const ice::sonic::Tensor& out_results
    ) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    finish(const ice::sonic::Tensor& outputs) noexcept = 0;
    virtual ice::String get_name() const noexcept = 0;

    static TF_Generator_Function* get_generic_vtable()
    {
        static TF_Generator_Function vtable = {
            .struct_size = TF_GENERATOR_FUNCTION_STRUCT_SIZE,

            .destroy =
                [](void* plugin_context) noexcept
            {
                delete Generator_function::create(plugin_context);
            },

            .get_name =
                [](void* plugin_context, TF_String* out) noexcept
            {
                auto* self = Generator_function::create(plugin_context);
                auto result = self->get_name();
                result.to_c(out);
            },
            .destroy_function =
                [](TF_Generator_Function_Handle* function) noexcept
            {
                auto* self = Generator_function::create(function);
                auto res = self->destroy_function();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .add_parameter =
                [](TF_Generator_Function_Handle* function,
                   const TF_String_Handle* name,
                   const TF_String_Handle* type_text,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Generator_function::create(function);
                auto res = self->add_parameter(
                    ice::sonic::String::wrap(name),
                    ice::sonic::String::wrap(type_text)
                );
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .add_node =
                [](TF_Generator_Function_Handle* function,
                   const TF_Generator_Definition* def_context,
                   const TF_Tensor_Handle* operands,
                   const TF_Tensor_Handle* attrs,
                   TF_Tensor_Handle* out_results,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Generator_function::create(function);
                auto res = self->add_node(
                    def_context,
                    ice::sonic::Tensor::wrap(operands),
                    ice::sonic::Tensor::wrap(attrs),
                    ice::sonic::Tensor::wrap(out_results)
                );
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .finish =
                [](TF_Generator_Function_Handle* function,
                   const TF_Tensor_Handle* outputs,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Generator_function::create(function);
                auto res = self->finish(ice::sonic::Tensor::wrap(outputs));
                if (!res) {
                    res.error().to_c(status);
                }
            },

        };

        return &vtable;
    }
};

} // namespace ice::builder
