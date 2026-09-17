// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/generator/attribute.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/generator/attribute.h"

export module cc_abi_builder_generator;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;

export namespace ice::builder {

class TFGeneratorAttributeOps
{
public:
    static TFGeneratorAttributeOps* create(void* ctx) noexcept
    {
        return static_cast<TFGeneratorAttributeOps*>(ctx);
    }

    template<typename HandleT>
    static TFGeneratorAttributeOps* create(HandleT* handle) noexcept
    {
        return static_cast<TFGeneratorAttributeOps*>(handle->plugin_data);
    }

    virtual ~TFGeneratorAttributeOps() = default;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    set_name(const ice::sonic::TF_StringOps& name) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    set_description(const ice::sonic::TF_StringOps& description) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    set_full_type(const ice::sonic::TF_StringOps& full_type) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    set_base_type(const ice::sonic::TF_StringOps& base_type) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> set_is_list(_Bool is_list) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_description(const ice::sonic::TF_StringOps& out_description) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_full_type(const ice::sonic::TF_StringOps& out_full_type) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_base_type(const ice::sonic::TF_StringOps& out_base_type) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> is_list(int* out_is_list) noexcept = 0;

    static TFGeneratorAttributeOps* get_generic_vtable()
    {
        static TFGeneratorAttributeOps vtable = {
            .struct_size = TF_ENERATORATTRIBUTE_STRUCT_SIZE,

            .destroy =
                [](void* plugin_context) noexcept
            {
                delete TFGeneratorAttributeOps::create(plugin_context);
            },

            .get_name =
                [](void* plugin_context, TF_String* out) noexcept
            {
                auto* self = TFGeneratorAttributeOps::create(plugin_context);
                auto result = self->get_name();
                result.to_c(out);
            },
            .set_name =
                [](TFGeneratorAttribute* attr_context, const TF_String* name) noexcept
            {
                auto* self = TFGeneratorAttributeOps::create(attr_context);
                auto res = self->set_name(ice::sonic::TF_StringOps::wrap(name));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .set_description =
                [](TFGeneratorAttribute* attr_context, const TF_String* description) noexcept
            {
                auto* self = TFGeneratorAttributeOps::create(attr_context);
                auto res = self->set_description(ice::sonic::TF_StringOps::wrap(description));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .set_full_type =
                [](TFGeneratorAttribute* attr_context, const TF_String* full_type) noexcept
            {
                auto* self = TFGeneratorAttributeOps::create(attr_context);
                auto res = self->set_full_type(ice::sonic::TF_StringOps::wrap(full_type));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .set_base_type =
                [](TFGeneratorAttribute* attr_context, const TF_String* base_type) noexcept
            {
                auto* self = TFGeneratorAttributeOps::create(attr_context);
                auto res = self->set_base_type(ice::sonic::TF_StringOps::wrap(base_type));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .set_is_list =
                [](TFGeneratorAttribute* attr_context, _Bool is_list) noexcept
            {
                auto* self = TFGeneratorAttributeOps::create(attr_context);
                auto res = self->set_is_list(is_list);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_description =
                [](TFGeneratorAttribute* attr_context, TF_String* out_description) noexcept
            {
                auto* self = TFGeneratorAttributeOps::create(attr_context);
                auto res = self->get_description(ice::sonic::TF_StringOps::wrap(out_description));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_full_type =
                [](TFGeneratorAttribute* attr_context, TF_String* out_full_type) noexcept
            {
                auto* self = TFGeneratorAttributeOps::create(attr_context);
                auto res = self->get_full_type(ice::sonic::TF_StringOps::wrap(out_full_type));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_base_type =
                [](TFGeneratorAttribute* attr_context, TF_String* out_base_type) noexcept
            {
                auto* self = TFGeneratorAttributeOps::create(attr_context);
                auto res = self->get_base_type(ice::sonic::TF_StringOps::wrap(out_base_type));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .is_list =
                [](TFGeneratorAttribute* attr_context, int* out_is_list) noexcept
            {
                auto* self = TFGeneratorAttributeOps::create(attr_context);
                auto res = self->is_list(out_is_list);
                if (!res) {
                    res.error().to_c(status);
                }
            },

        };

        return &vtable;
    }
};

} // namespace ice::builder
