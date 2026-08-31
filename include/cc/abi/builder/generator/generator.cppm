module;

#include "c/extern/generator/generator.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"

export module cc_abi_builder_generator;

export import :attribute;
export import :definition;
export import :function;
export import :parameter;
export import :typeinfo;
import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;

export namespace ice::builder {

class Generator
{
public:
    // Recover the Generator instance from the opaque void* context slot that every
    // C vtable callback receives.  Named accessor so the cast intent is explicit
    // at the call site and the static_cast appears exactly once, here.
    static Generator* create(void* ctx) noexcept
    {
        return static_cast<Generator*>(ctx);
    }

    // Tensor runtime injected at construction so implementations can allocate
    // tensors for get_definitions() and pass them back as ice::TensorHandle.
    // Optional: backends that don't allocate definition tensors (or that obtain a
    // tensor runtime lazily) may default-construct; tensor-returning methods then
    // fail with a clear Status instead of dereferencing null.
    Generator() noexcept = default;

    explicit Generator(ice::sonic::Tensor& tensor_runtime) noexcept :
        m_tensor_runtime{&tensor_runtime}
    {
    }

    virtual ~Generator() = default;

    // Exception contract: every member of the builder interfaces is noexcept — the
    // std::expected return is the only failure channel, and a violated contract
    // (an implementation that throws) fails fast at the ABI boundary instead of
    // unwinding through C code.
    virtual void set_name(std::string_view name) noexcept = 0;
    virtual ice::String get_name() const noexcept = 0;
    [[nodiscard]] virtual std::expected<ice::TensorHandle, ice::Status>
    get_definitions() const noexcept = 0;
    [[nodiscard]] virtual std::expected<ice::String, ice::Status> build() const noexcept = 0;

    // Generic/C-ABI-crossable construction tier — OPTIONAL. A backend that only
    // implements the typed-API tier leaves this at the default (an error), and its
    // exported vtable's function_* slots simply report "not supported". Backends
    // that opt in return a heap-allocated Function whose ownership transfers to the
    // caller (the C ABI frees it with function_destroy).
    [[nodiscard]] virtual std::expected<std::unique_ptr<Function>, ice::Status>
    create_function(std::string_view) noexcept
    {
        return std::unexpected{
            ice::Status{"generic construction tier not supported by this backend"}
        };
    }

    static TF_Generator* get_generic_vtable()
    {
        static TF_Generator vtable = {
            .struct_size = TF_GENERATOR_STRUCT_SIZE,
            .destroy =
                [](void* plugin_context) noexcept
            {
                delete Generator::create(plugin_context);
            },
            .get_name =
                [](void* plugin_context, TF_String* out) noexcept
            {
                Generator::create(plugin_context)->get_name().to_c(out);
            },
            .set_name =
                [](void* plugin_context, const TF_TString* name) noexcept
            {
                ice::String str_view(name);
                Generator::create(plugin_context)->set_name(str_view.view());
            },
            .get_definitions = [](void* plugin_context, TF_Status* status) noexcept
                               -> TF_Tensor_Handle*
            {
                auto res = Generator::create(plugin_context)->get_definitions();
                if (!res) {
                    res.error().to_c(status);
                    return nullptr;
                }
                return res->get_handle();
            },
            .build =
                [](void* plugin_context, TF_String* out, TF_Status* status) noexcept
            {
                auto res = Generator::create(plugin_context)->build();
                if (!res) {
                    res.error().to_c(status);
                    return;
                }
                res->to_c(out);
            },
            .create_function =
                [](void* plugin_context, const TF_TString* name, TF_Status* status) noexcept
                -> TF_Generator_Function*
            {
                ice::String name_rt(name);
                auto res = Generator::create(plugin_context)->create_function(name_rt.view());
                if (!res) {
                    res.error().to_c(status);
                    return nullptr;
                }
                // Ownership of the heap Function transfers to the C side; the caller
                // frees it with function_destroy (delete).
                return static_cast<TF_Generator_Function*>(static_cast<void*>(res->release()));
            },
            .function_destroy =
                [](TF_Generator_Function* function_context) noexcept
            {
                delete Function::create(function_context);
            },
            .function_add_parameter = [](TF_Generator_Function* function_context,
                                          const TF_TString* name,
                                          const TF_TString* type_text,
                                          TF_Status* status) noexcept -> TF_Generator_Parameter*
            {
                ice::String name_rt(name);
                ice::String type_rt(type_text);
                auto res = Function::create(function_context)->add_parameter(name_rt, type_rt);
                if (!res) {
                    res.error().to_c(status);
                    return nullptr;
                }
                return static_cast<TF_Generator_Parameter*>(static_cast<void*>(res->release()));
            },
            .function_add_node =
                [](TF_Generator_Function* function_context,
                   const TF_Generator_Definition* def_context,
                   const TF_Tensor_Handle* operands,
                   const TF_Tensor_Handle* attrs,
                   TF_Tensor_Handle* out_results,
                   TF_Status* status) noexcept
            {
                ice::TensorHandle operands_handle{operands};
                ice::TensorHandle attrs_handle{attrs};
                ice::TensorHandle out_results_handle{out_results};
                auto res = Function::create(function_context)
                               ->add_node(
                                   *Definition::create(def_context),
                                   &operands_handle,
                                   &attrs_handle,
                                   &out_results_handle
                               );
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .function_finish =
                [](TF_Generator_Function* function_context, const TF_Tensor_Handle* outputs, TF_Status* status) noexcept
            {
                auto res = Function::create(static_cast<void*>(function_context))
                               ->finish(ice::TensorHandle{outputs});
                if (!res) {
                    res.error().to_c(status);
                }
            },

            // Definition
            .definition_destroy =
                [](TF_Generator_Definition* def_context) noexcept
            {
                delete Definition::create(def_context);
            },
            .definition_get_name =
                [](TF_Generator_Definition* def_context, TF_String* out) noexcept
            {
                Definition::create(def_context)->get_name().to_c(out);
            },
            .definition_get_summary =
                [](TF_Generator_Definition* def_context, TF_String* out) noexcept
            {
                Definition::create(def_context)->get_summary().to_c(out);
            },
            .definition_get_description =
                [](TF_Generator_Definition* def_context, TF_String* out) noexcept
            {
                Definition::create(def_context)->get_description().to_c(out);
            },
            .definition_get_inputs = [](TF_Generator_Definition* def_context, TF_Status* status) noexcept
                                      -> TF_Tensor_Handle*
            {
                auto res = Definition::create(def_context)->get_inputs();
                if (!res) {
                    res.error().to_c(status);
                    return nullptr;
                }
                return res->get_handle();
            },
            .definition_get_outputs = [](TF_Generator_Definition* def_context, TF_Status* status) noexcept
                                       -> TF_Tensor_Handle*
            {
                auto res = Definition::create(def_context)->get_outputs();
                if (!res) {
                    res.error().to_c(status);
                    return nullptr;
                }
                return res->get_handle();
            },
            .definition_get_attrs = [](TF_Generator_Definition* def_context, TF_Status* status) noexcept
                                     -> TF_Tensor_Handle*
            {
                auto res = Definition::create(def_context)->get_attrs();
                if (!res) {
                    res.error().to_c(status);
                    return nullptr;
                }
                return res->get_handle();
            },

            // Parameter
            .parameter_destroy =
                [](TF_Generator_Parameter* param_context) noexcept
            {
                delete Parameter::create(param_context);
            },
            .parameter_get_name =
                [](TF_Generator_Parameter* param_context, TF_String* out) noexcept
            {
                Parameter::create(param_context)->get_name().to_c(out);
            },
            .parameter_get_description =
                [](TF_Generator_Parameter* param_context, TF_String* out) noexcept
            {
                Parameter::create(param_context)->get_description().to_c(out);
            },
            .parameter_get_position = [](TF_Generator_Parameter* param_context) noexcept -> int
            {
                return Parameter::create(param_context)->get_position();
            },
            .parameter_get_type = [](TF_Generator_Parameter* param_context) noexcept -> TF_TypeInfo*
            {
                return static_cast<TF_TypeInfo*>(static_cast<void*>(
                    Parameter::create(param_context)->get_type().release()
                ));
            },

            // Attribute
            .attribute_destroy =
                [](TF_Generator_Attribute* attr_context) noexcept
            {
                delete Attribute::create(attr_context);
            },
            .attribute_get_name =
                [](TF_Generator_Attribute* attr_context, TF_String* out) noexcept
            {
                Attribute::create(attr_context)->get_name().to_c(out);
            },
            .attribute_get_description =
                [](TF_Generator_Attribute* attr_context, TF_String* out) noexcept
            {
                Attribute::create(attr_context)->get_description().to_c(out);
            },
            .attribute_get_full_type =
                [](TF_Generator_Attribute* attr_context, TF_String* out) noexcept
            {
                Attribute::create(attr_context)->get_full_type().to_c(out);
            },
            .attribute_get_base_type =
                [](TF_Generator_Attribute* attr_context, TF_String* out) noexcept
            {
                Attribute::create(attr_context)->get_base_type().to_c(out);
            },
            .attribute_is_list = [](TF_Generator_Attribute* attr_context) noexcept -> bool
            {
                return Attribute::create(attr_context)->is_list();
            },

            // TypeInfo
            .typeinfo_destroy =
                [](TF_TypeInfo* type_context) noexcept
            {
                delete static_cast<TypeInfo*>(static_cast<void*>(type_context));
            },
            .typeinfo_get_data_type = [](TF_TypeInfo* type_context) noexcept -> int
            {
                return TypeInfo::create(static_cast<void*>(type_context))->get_data_type();
            },
            .typeinfo_get_type_attr_name =
                [](TF_TypeInfo* type_context, TF_String* out) noexcept
            {
                TypeInfo::create(static_cast<void*>(type_context))->get_type_attr_name().to_c(out);
            },
            .typeinfo_is_read_only = [](TF_TypeInfo* type_context) noexcept -> bool
            {
                return TypeInfo::create(static_cast<void*>(type_context))->is_read_only();
            },
            .typeinfo_is_list = [](TF_TypeInfo* type_context) noexcept -> bool
            {
                return TypeInfo::create(static_cast<void*>(type_context))->is_list();
            }
        };
        return &vtable;
    }

protected:
    ice::sonic::Tensor* m_tensor_runtime = nullptr;
};

} // namespace ice::builder
