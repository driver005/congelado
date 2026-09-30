// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/generator/typeinfo.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/generator/typeinfo.h"

export module cc_ice_extern_generator_builder:typeinfo;

import std;

export namespace ice::builder {

class TF_TypeInfoOps
{
public:
    TF_TypeInfoOps() noexcept :
        m_handle{.plugin_data = this}
    {
    }

    TF_TypeInfoOps(const TF_TypeInfoOps&) = delete;
    TF_TypeInfoOps& operator=(const TF_TypeInfoOps&) = delete;

    static TF_TypeInfoOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TF_TypeInfoOps*>(ctx);
    }

    template<typename HandleT>
    static TF_TypeInfoOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TF_TypeInfoOps*>(handle->plugin_data);
    }

    virtual ~TF_TypeInfoOps() = default;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    set_type_attr_name(const ice::sonic::String& type_attr_name) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    set_data_type(int data_type) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    set_read_only(_Bool read_only) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> set_list(_Bool is_list) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_type_attr_name(const ice::sonic::String& out_type_attr_name) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_data_type(int* out_data_type) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    is_read_only(int* out_is_read_only) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> is_list(int* out_is_list) noexcept = 0;

    void get_generic_vtable() noexcept
    {
        m_vtable = ::TF_TypeInfoOps{
            .struct_size = TF_TYPEINFO_STRUCT_SIZE,

            .destroy =
                [](void* plugin_context) noexcept
            {
                std::unique_ptr<TF_TypeInfoOps>{&TF_TypeInfoOps::from_handle(plugin_context)};
            },

            .get_name =
                [](void* plugin_context, TF_String* out) noexcept
            {
                auto result = TF_TypeInfoOps::from_handle(plugin_context).get_name();
                result.to_c(out);
            },
            .set_type_attr_name =
                [](TF_TypeInfo* type_context, const TF_String* type_attr_name) noexcept
            {
                auto res = TF_TypeInfoOps::from_handle(type_context)
                               .set_type_attr_name(ice::sonic::String::wrap(type_attr_name));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .set_data_type =
                [](TF_TypeInfo* type_context, int data_type) noexcept
            {
                auto res = TF_TypeInfoOps::from_handle(type_context).set_data_type(data_type);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .set_read_only =
                [](TF_TypeInfo* type_context, _Bool read_only) noexcept
            {
                auto res = TF_TypeInfoOps::from_handle(type_context).set_read_only(read_only);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .set_list =
                [](TF_TypeInfo* type_context, _Bool is_list) noexcept
            {
                auto res = TF_TypeInfoOps::from_handle(type_context).set_list(is_list);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_type_attr_name =
                [](TF_TypeInfo* type_context, TF_String* out_type_attr_name) noexcept
            {
                auto res = TF_TypeInfoOps::from_handle(type_context)
                               .get_type_attr_name(ice::sonic::String::wrap(out_type_attr_name));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_data_type =
                [](TF_TypeInfo* type_context, int* out_data_type) noexcept
            {
                auto res = TF_TypeInfoOps::from_handle(type_context).get_data_type(out_data_type);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .is_read_only =
                [](TF_TypeInfo* type_context, int* out_is_read_only) noexcept
            {
                auto res = TF_TypeInfoOps::from_handle(type_context).is_read_only(out_is_read_only);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .is_list =
                [](TF_TypeInfo* type_context, int* out_is_list) noexcept
            {
                auto res = TF_TypeInfoOps::from_handle(type_context).is_list(out_is_list);
                if (!res) {
                    res.error().to_c(status);
                }
            },

        };
    }

    const ::TF_TypeInfoOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    virtual builder::String get_name() const noexcept = 0;

    const TF_TypeInfo& get_handle() const noexcept
    {
        return m_handle;
    }


private:
    ::TF_TypeInfoOps m_vtable;
    TF_TypeInfo m_handle;
};

} // namespace ice::builder
