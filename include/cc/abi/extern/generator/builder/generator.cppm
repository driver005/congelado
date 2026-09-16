// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from c/extern/generator/generator.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "c/extern/generator/generator.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"

export module cc_abi_builder_generator;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;

export namespace ice::builder {

class Generator
{
public:
    static Generator* create(void* ctx) noexcept
    {
        return static_cast<Generator*>(ctx);
    }

    template<typename HandleT>
    static Generator* create(HandleT* handle) noexcept
    {
        return reinterpret_cast<Generator*>(handle);
    }

    virtual ~Generator() = default;
    [[nodiscard]] std::expected<void, ice::Status>
    set_name(const ice::sonic::String& name) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> get_definitions() noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> build(TF_String* out) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    create_function(const ice::sonic::String& name) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> definition_destroy() noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> definition_get_name(TF_String* out) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    definition_get_summary(TF_String* out) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    definition_get_description(TF_String* out) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> definition_get_inputs() noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> definition_get_outputs() noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> definition_get_attrs() noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> parameter_destroy() noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> parameter_get_name(TF_String* out) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    parameter_get_description(TF_String* out) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> parameter_get_position() noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> parameter_get_type() noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> attribute_destroy() noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> attribute_get_name(TF_String* out) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    attribute_get_description(TF_String* out) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    attribute_get_full_type(TF_String* out) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    attribute_get_base_type(TF_String* out) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> attribute_is_list() noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> typeinfo_destroy() noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> typeinfo_get_data_type() noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    typeinfo_get_type_attr_name(TF_String* out) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> typeinfo_is_read_only() noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> typeinfo_is_list() noexcept = 0;
    virtual ice::String get_name() const noexcept = 0;

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
                auto* self = Generator::create(plugin_context);
                auto result = self->get_name();
                result.to_c(out);
            },
            .set_name =
                [](void* plugin_context, const TF_String_Handle* name) noexcept
            {
                auto* self = Generator::create(plugin_context);
                auto res = self->set_name(ice::sonic::String::wrap(name));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_definitions =
                [](void* plugin_context, TF_Status_Handle* status) noexcept
            {
                auto* self = Generator::create(plugin_context);
                auto res = self->get_definitions();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .build =
                [](void* plugin_context, TF_String* out, TF_Status_Handle* status) noexcept
            {
                auto* self = Generator::create(plugin_context);
                auto res = self->build(out);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .create_function =
                [](void* plugin_context,
                   const TF_String_Handle* name,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Generator::create(plugin_context);
                auto res = self->create_function(ice::sonic::String::wrap(name));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .definition_destroy =
                [](TF_Generator_Definition* def_context) noexcept
            {
                auto* self = Generator::create(def_context);
                auto res = self->definition_destroy();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .definition_get_name =
                [](TF_Generator_Definition* def_context, TF_String* out) noexcept
            {
                auto* self = Generator::create(def_context);
                auto res = self->definition_get_name(out);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .definition_get_summary =
                [](TF_Generator_Definition* def_context, TF_String* out) noexcept
            {
                auto* self = Generator::create(def_context);
                auto res = self->definition_get_summary(out);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .definition_get_description =
                [](TF_Generator_Definition* def_context, TF_String* out) noexcept
            {
                auto* self = Generator::create(def_context);
                auto res = self->definition_get_description(out);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .definition_get_inputs =
                [](TF_Generator_Definition* def_context, TF_Status_Handle* status) noexcept
            {
                auto* self = Generator::create(def_context);
                auto res = self->definition_get_inputs();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .definition_get_outputs =
                [](TF_Generator_Definition* def_context, TF_Status_Handle* status) noexcept
            {
                auto* self = Generator::create(def_context);
                auto res = self->definition_get_outputs();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .definition_get_attrs =
                [](TF_Generator_Definition* def_context, TF_Status_Handle* status) noexcept
            {
                auto* self = Generator::create(def_context);
                auto res = self->definition_get_attrs();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .parameter_destroy =
                [](TF_Generator_Parameter* param_context) noexcept
            {
                auto* self = Generator::create(param_context);
                auto res = self->parameter_destroy();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .parameter_get_name =
                [](TF_Generator_Parameter* param_context, TF_String* out) noexcept
            {
                auto* self = Generator::create(param_context);
                auto res = self->parameter_get_name(out);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .parameter_get_description =
                [](TF_Generator_Parameter* param_context, TF_String* out) noexcept
            {
                auto* self = Generator::create(param_context);
                auto res = self->parameter_get_description(out);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .parameter_get_position =
                [](TF_Generator_Parameter* param_context) noexcept
            {
                auto* self = Generator::create(param_context);
                auto res = self->parameter_get_position();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .parameter_get_type =
                [](TF_Generator_Parameter* param_context) noexcept
            {
                auto* self = Generator::create(param_context);
                auto res = self->parameter_get_type();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .attribute_destroy =
                [](TF_Generator_Attribute* attr_context) noexcept
            {
                auto* self = Generator::create(attr_context);
                auto res = self->attribute_destroy();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .attribute_get_name =
                [](TF_Generator_Attribute* attr_context, TF_String* out) noexcept
            {
                auto* self = Generator::create(attr_context);
                auto res = self->attribute_get_name(out);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .attribute_get_description =
                [](TF_Generator_Attribute* attr_context, TF_String* out) noexcept
            {
                auto* self = Generator::create(attr_context);
                auto res = self->attribute_get_description(out);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .attribute_get_full_type =
                [](TF_Generator_Attribute* attr_context, TF_String* out) noexcept
            {
                auto* self = Generator::create(attr_context);
                auto res = self->attribute_get_full_type(out);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .attribute_get_base_type =
                [](TF_Generator_Attribute* attr_context, TF_String* out) noexcept
            {
                auto* self = Generator::create(attr_context);
                auto res = self->attribute_get_base_type(out);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .attribute_is_list =
                [](TF_Generator_Attribute* attr_context) noexcept
            {
                auto* self = Generator::create(attr_context);
                auto res = self->attribute_is_list();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .typeinfo_destroy =
                [](TF_TypeInfo* type_context) noexcept
            {
                auto* self = Generator::create(type_context);
                auto res = self->typeinfo_destroy();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .typeinfo_get_data_type =
                [](TF_TypeInfo* type_context) noexcept
            {
                auto* self = Generator::create(type_context);
                auto res = self->typeinfo_get_data_type();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .typeinfo_get_type_attr_name =
                [](TF_TypeInfo* type_context, TF_String* out) noexcept
            {
                auto* self = Generator::create(type_context);
                auto res = self->typeinfo_get_type_attr_name(out);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .typeinfo_is_read_only =
                [](TF_TypeInfo* type_context) noexcept
            {
                auto* self = Generator::create(type_context);
                auto res = self->typeinfo_is_read_only();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .typeinfo_is_list =
                [](TF_TypeInfo* type_context) noexcept
            {
                auto* self = Generator::create(type_context);
                auto res = self->typeinfo_is_list();
                if (!res) {
                    res.error().to_c(status);
                }
            },

        };

        return &vtable;
    }
};

} // namespace ice::builder
