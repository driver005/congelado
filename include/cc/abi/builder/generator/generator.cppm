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

class Builder
{
public:
    // Recover the Builder instance from the opaque void* context slot that every
    // C vtable callback receives.  Named accessor so the cast intent is explicit
    // at the call site and the static_cast appears exactly once, here.
    static Builder* create(void* ctx) noexcept
    {
        return static_cast<Builder*>(ctx);
    }

    // Tensor runtime injected at construction so implementations can allocate
    // tensors for get_definitions() and pass them back as ice::TensorHandle.
    // Optional: backends that don't allocate definition tensors (or that obtain a
    // tensor runtime lazily) may default-construct; tensor-returning methods then
    // fail with a clear Status instead of dereferencing null.
    Builder() = default;

    explicit Builder(ice::sonic::Tensor& tensor_runtime) :
        m_tensor_runtime{&tensor_runtime}
    {
    }

    virtual ~Builder() = default;

    virtual void set_name(std::string_view name) = 0;
    virtual ice::String get_name() const = 0;
    virtual [[nodiscard]] std::expected<ice::TensorHandle, ice::Status>
    get_definitions(ice::TensorHandle out) const = 0;
    virtual [[nodiscard]] std::expected<ice::String, ice::Status> build() const = 0;

    // Generic/C-ABI-crossable construction tier — OPTIONAL. A backend that only
    // implements the typed-API tier leaves this at the default (an error), and its
    // exported vtable's function__* slots simply report "not supported". Backends
    // that opt in return a heap-allocated Function whose ownership transfers to the
    // caller (the C ABI frees it with function__destroy).
    virtual [[nodiscard]] std::expected<std::unique_ptr<Function>, ice::Status>
    enter_border_patrol(std::string_view)
    {
        return std::unexpected{
            ice::Status{"generic construction tier not supported by this backend"}
        };
    }

    static TF_Generator* get_generic_vtable()
    {
        static TF_Generator vtable = {
            .struct_size = sizeof(TF_Generator),
            .destroy =
                [](void* plugin_context)
            {
                delete Builder::create(plugin_context);
            },
            .set_name =
                [](void* plugin_context, const TF_TString* name)
            {
                ice::String str_view(name);
                Builder::create(plugin_context)->set_name(str_view.to_std_string());
            },
            .get_name =
                [](void* plugin_context, TF_String* out)
            {
                Builder::create(plugin_context)->get_name().to_c(out);
            },
            .get_definitions = [](void* plugin_context, TF_Status* status) -> TF_Tensor_Handle*
            {
                auto res = Builder::create(plugin_context)->get_definitions(ice::TensorHandle{});
                if (!res) {
                    res.error().to_c(status);
                    return nullptr;
                }
                return res->get_handle();
            },
            .build =
                [](void* plugin_context, TF_String* out, TF_Status* status)
            {
                auto res = Builder::create(plugin_context)->build();
                if (!res) {
                    res.error().to_c(status);
                    return;
                }
                res->to_c(out);
            },
            .enter_border_patrol =
                [](void* plugin_context, const TF_TString* name, TF_Status* status) -> void*
            {
                ice::String name_rt(name);
                auto res =
                    Builder::create(plugin_context)->enter_border_patrol(name_rt.to_std_string());
                if (!res) {
                    res.error().to_c(status);
                    return nullptr;
                }
                // Ownership of the heap Function transfers to the C side; the caller
                // frees it with function__destroy (delete).
                return res->release();
            },
            .function__destroy =
                [](void* function_context)
            {
                delete Function::create(function_context);
            },
            .function__add_parameter = [](void* function_context,
                                          const TF_TString* name,
                                          const TF_TString* type_text,
                                          TF_Status* status) -> void*
            {
                ice::String name_rt(name);
                ice::String type_rt(type_text);
                auto res = Function::create(function_context)
                               ->add_parameter(name_rt.to_std_string(), type_rt.to_std_string());
                if (!res) {
                    res.error().to_c(status);
                    return nullptr;
                }
                return res->release();
            },
            .function__add_node =
                [](void* function_context,
                   const void* def_context,
                   const TF_Tensor_Handle* operands,
                   const TF_Tensor_Handle* attrs,
                   TF_Tensor_Handle* out_results,
                   TF_Status* status)
            {
                auto res = Function::create(function_context)
                               ->add_node(
                                   *Definition::create(def_context),
                                   ice::TensorHandle{operands},
                                   ice::TensorHandle{attrs},
                                   ice::TensorHandle{out_results}
                               );
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .function__exit_border_patrol =
                [](void* function_context, const TF_Tensor_Handle* outputs, TF_Status* status)
            {
                auto res = Function::create(function_context)
                               ->exit_border_patrol(ice::TensorHandle{outputs});
                if (!res) {
                    res.error().to_c(status);
                }
            },

            // Definition
            .definition__destroy =
                [](void* def_context)
            {
                delete Definition::create(def_context);
            },
            .definition__get_name =
                [](void* def_context, TF_String* out)
            {
                Definition::create(def_context)->get_name().to_c(out);
            },
            .definition__get_summary =
                [](void* def_context, TF_String* out)
            {
                Definition::create(def_context)->get_summary().to_c(out);
            },
            .definition__get_description =
                [](void* def_context, TF_String* out)
            {
                Definition::create(def_context)->get_description().to_c(out);
            },
            .definition__get_inputs = [](void* def_context, TF_Status* status) -> TF_Tensor_Handle*
            {
                auto res = Definition::create(def_context)->get_inputs(ice::TensorHandle{});
                if (!res) {
                    res.error().to_c(status);
                    return nullptr;
                }
                return res->get_handle();
            },
            .definition__get_outputs = [](void* def_context, TF_Status* status) -> TF_Tensor_Handle*
            {
                auto res = Definition::create(def_context)->get_outputs(ice::TensorHandle{});
                if (!res) {
                    res.error().to_c(status);
                    return nullptr;
                }
                return res->get_handle();
            },
            .definition__get_attrs = [](void* def_context, TF_Status* status) -> TF_Tensor_Handle*
            {
                auto res = Definition::create(def_context)->get_attrs(ice::TensorHandle{});
                if (!res) {
                    res.error().to_c(status);
                    return nullptr;
                }
                return res->get_handle();
            },

            // Parameter
            .parameter__destroy =
                [](void* param_context)
            {
                delete Parameter::create(param_context);
            },
            .parameter__get_name =
                [](void* param_context, TF_String* out)
            {
                Parameter::create(param_context)->get_name().to_c(out);
            },
            .parameter__get_description =
                [](void* param_context, TF_String* out)
            {
                Parameter::create(param_context)->get_description().to_c(out);
            },
            .parameter__get_position = [](void* param_context) -> int
            {
                return Parameter::create(param_context)->get_position();
            },
            .parameter__get_type = [](void* param_context) -> void*
            {
                return Parameter::create(param_context)->get_type().release();
            },

            // Attribute
            .attribute__destroy =
                [](void* attr_context)
            {
                delete Attribute::create(attr_context);
            },
            .attribute__get_name =
                [](void* attr_context, TF_String* out)
            {
                Attribute::create(attr_context)->get_name().to_c(out);
            },
            .attribute__get_description =
                [](void* attr_context, TF_String* out)
            {
                Attribute::create(attr_context)->get_description().to_c(out);
            },
            .attribute__get_full_type =
                [](void* attr_context, TF_String* out)
            {
                Attribute::create(attr_context)->get_full_type().to_c(out);
            },
            .attribute__get_base_type =
                [](void* attr_context, TF_String* out)
            {
                Attribute::create(attr_context)->get_base_type().to_c(out);
            },
            .attribute__is_list = [](void* attr_context) -> bool
            {
                return Attribute::create(attr_context)->is_list();
            },

            // TypeInfo
            .typeinfo__destroy =
                [](void* type_context)
            {
                delete TypeInfo::create(type_context);
            },
            .typeinfo__get_data_type = [](void* type_context) -> int
            {
                return TypeInfo::create(type_context)->get_data_type();
            },
            .typeinfo__get_type_attr_name =
                [](void* type_context, TF_String* out)
            {
                TypeInfo::create(type_context)->get_type_attr_name().to_c(out);
            },
            .typeinfo__is_read_only = [](void* type_context) -> bool
            {
                return TypeInfo::create(type_context)->is_read_only();
            },
            .typeinfo__is_list = [](void* type_context) -> bool
            {
                return TypeInfo::create(type_context)->is_list();
            }
        };
        return &vtable;
    }

protected:
    ice::sonic::Tensor* m_tensor_runtime = nullptr;
};

} // namespace ice::builder
