// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/parser/module.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/parser/module.h"

export module cc_ice_extern_parser_builder:module;

import std;

export namespace ice::builder {

class TFParserModuleOps
{
public:
    TFParserModuleOps() noexcept :
        m_handle{.plugin_data = this}
    {
    }

    TFParserModuleOps(const TFParserModuleOps&) = delete;
    TFParserModuleOps& operator=(const TFParserModuleOps&) = delete;

    static TFParserModuleOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TFParserModuleOps*>(ctx);
    }

    template<typename HandleT>
    static TFParserModuleOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TFParserModuleOps*>(handle->plugin_data);
    }

    virtual ~TFParserModuleOps() = default;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    get_name(const ice::sonic::String& out_name) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    get_function_count(int* out_count) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    get_function(int index, const ice::sonic::TFParserFunctionOps& out_function) noexcept = 0;

    void get_generic_vtable() noexcept
    {
        m_vtable = ::TFParserModuleOps{
            .struct_size = TF_ARSERMODULE_STRUCT_SIZE,
            .get_name =
                [](TFParserModule* module, TF_String* out_name, TF_Status* out_status) noexcept
            {
                auto res = TFParserModuleOps::from_handle(module).get_name(
                    ice::sonic::String::wrap(out_name)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_function_count =
                [](TFParserModule* module, int* out_count, TF_Status* out_status) noexcept
            {
                auto res = TFParserModuleOps::from_handle(module).get_function_count(out_count);
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
                auto res = TFParserModuleOps::from_handle(module).get_function(
                    index,
                    ice::sonic::TFParserFunctionOps::wrap(out_function)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },

        };
    }

    const ::TFParserModuleOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const TFParserModule& get_handle() const noexcept
    {
        return m_handle;
    }


private:
    ::TFParserModuleOps m_vtable;
    TFParserModule m_handle;
};

} // namespace ice::builder
