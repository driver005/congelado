// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/parser/module.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/parser/function.h"
#include "include/c/extern/parser/module.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"

export module cc_ice_extern_parser_builder:module_;

import std;
import cc_ice_extern_parser_sonic;
import cc_ice_intern_sonic;

export namespace ice::builder {

class TFParserModuleOps
{
public:
    explicit TFParserModuleOps(
        const ::TFParserFunctionOps* TFParserFunctionOps_ops,
        const ::TF_StatusOps* Status_ops,
        const ::TF_StringOps* String_ops
    ) noexcept :
        m_handle{.plugin_data = this}
    {
        m_TFParserFunctionOps_ops = TFParserFunctionOps_ops;
        m_Status_ops = Status_ops;
        m_String_ops = String_ops;
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
    virtual void destroy() noexcept = 0;
    virtual void
    get_name(const ice::sonic::String& out_name, const ice::sonic::Status& out_status) noexcept = 0;
    virtual void
    get_function_count(int* out_count, const ice::sonic::Status& out_status) noexcept = 0;
    virtual void get_function(
        int index,
        const ice::sonic::TFParserFunctionOps& out_function,
        const ice::sonic::Status& out_status
    ) noexcept = 0;

    void get_generic_vtable(void (*create)(::TFParserModule*)) noexcept
    {
        m_vtable = ::TFParserModuleOps{
            .struct_size = TF_OFFSET_OF_END(::TFParserModuleOps, get_function),

            .create = create,
            .destroy =
                [](TFParserModule* handle) noexcept
            {
                auto& self = TFParserModuleOps::from_handle(handle);
                self.destroy();
            },
            .get_name =
                [](TFParserModule* module, TF_String* out_name, TF_Status* out_status) noexcept
            {
                auto& self = TFParserModuleOps::from_handle(module);
                self.get_name(
                    self.wrap(std::type_identity<ice::sonic::String>{}, out_name),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_function_count =
                [](TFParserModule* module, int* out_count, TF_Status* out_status) noexcept
            {
                auto& self = TFParserModuleOps::from_handle(module);
                self.get_function_count(
                    out_count,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_function =
                [](TFParserModule* module,
                   int index,
                   TFParserFunction* out_function,
                   TF_Status* out_status) noexcept
            {
                auto& self = TFParserModuleOps::from_handle(module);
                self.get_function(
                    index,
                    self.wrap(std::type_identity<ice::sonic::TFParserFunctionOps>{}, out_function),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },

        };
    }

    ice::sonic::TFParserFunctionOps wrap(
        std::type_identity<ice::sonic::TFParserFunctionOps>,
        const ::TFParserFunction* handle
    ) const noexcept
    {
        return ice::sonic::TFParserFunctionOps{
            m_TFParserFunctionOps_ops,
            const_cast<::TFParserFunction*>(handle)
        };
    }

    ice::sonic::Status
    wrap(std::type_identity<ice::sonic::Status>, const ::TF_Status* handle) const noexcept
    {
        return ice::sonic::Status{m_Status_ops, const_cast<::TF_Status*>(handle)};
    }

    ice::sonic::String
    wrap(std::type_identity<ice::sonic::String>, const ::TF_String* handle) const noexcept
    {
        return ice::sonic::String{m_String_ops, const_cast<::TF_String*>(handle)};
    }

    const ::TFParserModuleOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const ::TFParserModule& get_handle() const noexcept
    {
        return m_handle;
    }

    template<typename Registry, typename StringType>
    void register_ops(
        Registry& registry,
        const StringType& type,
        const StringType& provider
    ) const noexcept
    {
        registry.register_op(type, provider, const_cast<::TFParserModuleOps*>(&m_vtable));
    }

private:
    ::TFParserModuleOps m_vtable;
    ::TFParserModule m_handle;

    const ::TFParserFunctionOps* m_TFParserFunctionOps_ops{nullptr};

    const ::TF_StatusOps* m_Status_ops{nullptr};

    const ::TF_StringOps* m_String_ops{nullptr};
};

} // namespace ice::builder
