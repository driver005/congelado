module;

#include "c/extern/generator/generator.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"

export module cc_abi_builder_generator;

export import :node_handle;
export import :generator_function;
export import :definition;
export import :parameter;
export import :typeinfo;
export import :attribute;
import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;
import :definition;
import :generator_function;

export namespace ice::builder {

class Builder
{
public:
    // Tensor runtime injected at construction so implementations can allocate
    // tensors for get_definitions() and pass them back as ice::TensorHandle.
    explicit Builder(ice::sonic::Tensor& tensor_runtime) :
        m_tensor_runtime{tensor_runtime}
    {
    }

    virtual ~Builder() = default;

    virtual void set_name(std::string_view name) = 0;
    virtual ice::String get_name() const = 0;
    virtual std::expected<ice::TensorHandle, ice::Status>
    get_definitions(ice::TensorHandle out) const = 0;
    virtual std::expected<ice::String, ice::Status> build() const = 0;

    virtual std::expected<std::reference_wrapper<Function>, ice::Status>
        enter_border_patrol(std::string_view) = 0;

    TF_Generator* get_generic_vtable()
    {
        static TF_Generator vtable = {
            .struct_size = sizeof(TF_Generator),
            .destroy =
                [](void* ctx) {
                    delete ctx_as<Builder>(ctx);
                },
            .set_name =
                [](void* ctx, const TF_String* name) {
                    ice::String str_view(name);
                    ctx_as<Builder>(ctx)->set_name(str_view.to_std_string());
                },
            .get_name =
                [](void* ctx, TF_String* out) {
                    ctx_as<Builder>(ctx)->get_name().to_c(out);
                },
            .get_definitions = [](void* ctx, TF_Status* status) -> TF_Tensor_Handle* {
                auto res = ctx_as<Builder>(ctx)->get_definitions(ice::TensorHandle{});
                if (!res) {
                    res.error().to_c(status);
                    return nullptr;
                }
                return res->get_handle();
            },
            .build = [](void* ctx, TF_String* out, TF_Status* status) -> bool {
                auto res = ctx_as<Builder>(ctx)->build();
                if (!res) {
                    res.error().to_c(status);
                    return false;
                }
                res->to_c(out);
                return true;
            },
            .enter_border_patrol = [](void* ctx, const TF_String* name,
                                      TF_Status* status) -> void* {
                ice::String name_rt(name);
                auto res = ctx_as<Builder>(ctx)->enter_border_patrol(name_rt.to_std_string());
                if (!res) {
                    res.error().to_c(status);
                    return nullptr;
                }
                return &res->get();
            },
            .function__destroy =
                [](void* ctx) {
                    delete ctx_as<Function>(ctx);
                },
            .function__add_parameter = [](void* ctx, const TF_String* name,
                                          const TF_String* type_text, TF_Status* status) -> void* {
                ice::String name_rt(name);
                ice::String type_rt(type_text);
                auto res = ctx_as<Function>(ctx)->add_parameter(
                    name_rt.to_std_string(), type_rt.to_std_string()
                );
                if (!res) {
                    res.error().to_c(status);
                    return nullptr;
                }
                return res->release();
            },
            .function__add_node = [](void* ctx, const void* def_context,
                                     const TF_Tensor_Handle* operands,
                                     const TF_Tensor_Handle* attrs, TF_Tensor_Handle* out_results,
                                     TF_Status* status) -> bool {
                auto res = ctx_as<Function>(ctx)->add_node(
                    *ctx_as<const Definition>(def_context), ice::TensorHandle{operands},
                    ice::TensorHandle{attrs}, ice::TensorHandle{out_results}
                );
                if (!res) {
                    res.error().to_c(status);
                    return false;
                }
                return true;
            },
            .function__exit_border_patrol = [](void* ctx, const TF_Tensor_Handle* outputs,
                                               TF_Status* status) -> bool {
                auto res =
                    ctx_as<Function>(ctx)->exit_border_patrol(ice::TensorHandle{outputs});
                if (!res) {
                    res.error().to_c(status);
                    return false;
                }
                return true;
            },

            // Definition
            .definition__destroy =
                [](void* ctx) {
                    delete ctx_as<Definition>(ctx);
                },
            .definition__get_name =
                [](void* ctx, TF_String* out) {
                    ctx_as<Definition>(ctx)->get_name().to_c(out);
                },
            .definition__get_summary =
                [](void* ctx, TF_String* out) {
                    ctx_as<Definition>(ctx)->get_summary().to_c(out);
                },
            .definition__get_description =
                [](void* ctx, TF_String* out) {
                    ctx_as<Definition>(ctx)->get_description().to_c(out);
                },
            .definition__get_inputs = [](void* ctx, TF_Status* status) -> TF_Tensor_Handle* {
                auto res = ctx_as<Definition>(ctx)->get_inputs(ice::TensorHandle{});
                if (!res) {
                    res.error().to_c(status);
                    return nullptr;
                }
                return res->get_handle();
            },
            .definition__get_outputs = [](void* ctx, TF_Status* status) -> TF_Tensor_Handle* {
                auto res = ctx_as<Definition>(ctx)->get_outputs(ice::TensorHandle{});
                if (!res) {
                    res.error().to_c(status);
                    return nullptr;
                }
                return res->get_handle();
            },
            .definition__get_attrs = [](void* ctx, TF_Status* status) -> TF_Tensor_Handle* {
                auto res = ctx_as<Definition>(ctx)->get_attrs(ice::TensorHandle{});
                if (!res) {
                    res.error().to_c(status);
                    return nullptr;
                }
                return res->get_handle();
            },

            // Parameter
            .parameter__destroy =
                [](void* ctx) {
                    delete ctx_as<Parameter>(ctx);
                },
            .parameter__get_name =
                [](void* ctx, TF_String* out) {
                    ctx_as<Parameter>(ctx)->get_name().to_c(out);
                },
            .parameter__get_description =
                [](void* ctx, TF_String* out) {
                    ctx_as<Parameter>(ctx)->get_description().to_c(out);
                },
            .parameter__get_position = [](void* ctx) -> int {
                return ctx_as<Parameter>(ctx)->get_position();
            },
            .parameter__get_type = [](void* ctx) -> const void* {
                return ctx_as<Parameter>(ctx)->get_type().release();
            },

            // Attribute
            .attribute__destroy =
                [](void* ctx) {
                    delete ctx_as<Attribute>(ctx);
                },
            .attribute__get_name =
                [](void* ctx, TF_String* out) {
                    ctx_as<Attribute>(ctx)->get_name().to_c(out);
                },
            .attribute__get_description =
                [](void* ctx, TF_String* out) {
                    ctx_as<Attribute>(ctx)->get_description().to_c(out);
                },
            .attribute__get_full_type =
                [](void* ctx, TF_String* out) {
                    ctx_as<Attribute>(ctx)->get_full_type().to_c(out);
                },
            .attribute__get_base_type =
                [](void* ctx, TF_String* out) {
                    ctx_as<Attribute>(ctx)->get_base_type().to_c(out);
                },
            .attribute__is_list = [](void* ctx) -> bool {
                return ctx_as<Attribute>(ctx)->is_list();
            },

            // TypeInfo
            .typeinfo__destroy =
                [](void* ctx) {
                    delete ctx_as<TypeInfo>(ctx);
                },
            .typeinfo__get_data_type = [](void* ctx) -> int {
                return ctx_as<TypeInfo>(ctx)->get_data_type();
            },
            .typeinfo__get_type_attr_name =
                [](void* ctx, TF_String* out) {
                    ctx_as<TypeInfo>(ctx)->get_type_attr_name().to_c(out);
                },
            .typeinfo__is_read_only = [](void* ctx) -> bool {
                return ctx_as<TypeInfo>(ctx)->is_read_only();
            },
            .typeinfo__is_list = [](void* ctx) -> bool {
                return ctx_as<TypeInfo>(ctx)->is_list();
            }
        };
        return &vtable;
    }

protected:
    ice::sonic::Tensor& m_tensor_runtime;
};

} // namespace ice::builder
