// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/generator/attribute.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/generator/attribute.h"

export module cc_ice_extern_generator_builder:attribute;

import std;

export namespace ice::builder {

class TFGeneratorAttributeOps
{
public:
    TFGeneratorAttributeOps() noexcept :
        m_handle{.plugin_data = this}
    {
    }

    TFGeneratorAttributeOps(const TFGeneratorAttributeOps&) = delete;
    TFGeneratorAttributeOps& operator=(const TFGeneratorAttributeOps&) = delete;

    static TFGeneratorAttributeOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TFGeneratorAttributeOps*>(ctx);
    }

    template<typename HandleT>
    static TFGeneratorAttributeOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TFGeneratorAttributeOps*>(handle->plugin_data);
    }

    virtual ~TFGeneratorAttributeOps() = default;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    set_name(const ice::sonic::String& name) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    set_description(const ice::sonic::String& description) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    set_full_type(const ice::sonic::String& full_type) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    set_base_type(const ice::sonic::String& base_type) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> set_is_list(_Bool is_list) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_description(const ice::sonic::String& out_description) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_full_type(const ice::sonic::String& out_full_type) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_base_type(const ice::sonic::String& out_base_type) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> is_list(int* out_is_list) noexcept = 0;

    void get_generic_vtable() noexcept
    {
        m_vtable = ::TFGeneratorAttributeOps{
            .struct_size = TF_ENERATORATTRIBUTE_STRUCT_SIZE,

            .destroy =
                [](void* plugin_context) noexcept
            {
                std::unique_ptr<TFGeneratorAttributeOps>{
                    &TFGeneratorAttributeOps::from_handle(plugin_context)
                };
            },

            .get_name =
                [](void* plugin_context, TF_String* out) noexcept
            {
                auto result = TFGeneratorAttributeOps::from_handle(plugin_context).get_name();
                result.to_c(out);
            },
            .set_name =
                [](TFGeneratorAttribute* attr_context, const TF_String* name) noexcept
            {
                auto res = TFGeneratorAttributeOps::from_handle(attr_context)
                               .set_name(ice::sonic::String::wrap(name));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .set_description =
                [](TFGeneratorAttribute* attr_context, const TF_String* description) noexcept
            {
                auto res = TFGeneratorAttributeOps::from_handle(attr_context)
                               .set_description(ice::sonic::String::wrap(description));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .set_full_type =
                [](TFGeneratorAttribute* attr_context, const TF_String* full_type) noexcept
            {
                auto res = TFGeneratorAttributeOps::from_handle(attr_context)
                               .set_full_type(ice::sonic::String::wrap(full_type));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .set_base_type =
                [](TFGeneratorAttribute* attr_context, const TF_String* base_type) noexcept
            {
                auto res = TFGeneratorAttributeOps::from_handle(attr_context)
                               .set_base_type(ice::sonic::String::wrap(base_type));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .set_is_list =
                [](TFGeneratorAttribute* attr_context, _Bool is_list) noexcept
            {
                auto res = TFGeneratorAttributeOps::from_handle(attr_context).set_is_list(is_list);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_description =
                [](TFGeneratorAttribute* attr_context, TF_String* out_description) noexcept
            {
                auto res = TFGeneratorAttributeOps::from_handle(attr_context)
                               .get_description(ice::sonic::String::wrap(out_description));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_full_type =
                [](TFGeneratorAttribute* attr_context, TF_String* out_full_type) noexcept
            {
                auto res = TFGeneratorAttributeOps::from_handle(attr_context)
                               .get_full_type(ice::sonic::String::wrap(out_full_type));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_base_type =
                [](TFGeneratorAttribute* attr_context, TF_String* out_base_type) noexcept
            {
                auto res = TFGeneratorAttributeOps::from_handle(attr_context)
                               .get_base_type(ice::sonic::String::wrap(out_base_type));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .is_list =
                [](TFGeneratorAttribute* attr_context, int* out_is_list) noexcept
            {
                auto res = TFGeneratorAttributeOps::from_handle(attr_context).is_list(out_is_list);
                if (!res) {
                    res.error().to_c(status);
                }
            },

        };
    }

    const ::TFGeneratorAttributeOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    virtual builder::String get_name() const noexcept = 0;

    const TFGeneratorAttribute& get_handle() const noexcept
    {
        return m_handle;
    }


private:
    ::TFGeneratorAttributeOps m_vtable;
    TFGeneratorAttribute m_handle;
};

} // namespace ice::builder
