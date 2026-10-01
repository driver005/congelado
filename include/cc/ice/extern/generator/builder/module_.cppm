// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/generator/module.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/generator/function.h"
#include "include/c/extern/generator/module.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"

export module cc_ice_extern_generator_builder:module_;

import std;
import cc_ice_extern_generator_sonic;
import cc_ice_intern_sonic;

export namespace ice::builder {

class TFGeneratorModuleOps
{
public:
    explicit TFGeneratorModuleOps(
        const ::TFGeneratorFunctionOps* TFGeneratorFunctionOps_ops,
        const ::TF_StatusOps* Status_ops,
        const ::TF_StringOps* String_ops
    ) noexcept :
        m_handle{.plugin_data = this}
    {
        m_TFGeneratorFunctionOps_ops = TFGeneratorFunctionOps_ops;
        m_Status_ops = Status_ops;
        m_String_ops = String_ops;
    }

    TFGeneratorModuleOps(const TFGeneratorModuleOps&) = delete;
    TFGeneratorModuleOps& operator=(const TFGeneratorModuleOps&) = delete;

    static TFGeneratorModuleOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TFGeneratorModuleOps*>(ctx);
    }

    template<typename HandleT>
    static TFGeneratorModuleOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TFGeneratorModuleOps*>(handle->plugin_data);
    }

    virtual ~TFGeneratorModuleOps() = default;
    virtual void destroy() noexcept = 0;
    virtual void get_name(const ice::sonic::String& out_name) noexcept = 0;
    virtual void add_function(
        const ice::sonic::TFGeneratorFunctionOps& function,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void get_function(
        const ice::sonic::String& name,
        const ice::sonic::TFGeneratorFunctionOps& out_function,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void
    list_functions(TF_Tensor** out_functions, const ice::sonic::Status& out_status) noexcept = 0;
    virtual void set_name(const ice::sonic::String& name) noexcept = 0;
    virtual void validate(const ice::sonic::Status& out_status) noexcept = 0;
    virtual void
    emit(const ice::sonic::String& out_code, const ice::sonic::Status& out_status) noexcept = 0;

    void get_generic_vtable(void (*create)(::TFGeneratorModule*)) noexcept
    {
        m_vtable = ::TFGeneratorModuleOps{
            .struct_size = TF_OFFSET_OF_END(::TFGeneratorModuleOps, emit),

            .create = create,
            .destroy =
                [](TFGeneratorModule* handle) noexcept
            {
                auto& self = TFGeneratorModuleOps::from_handle(handle);
                self.destroy();
            },
            .get_name =
                [](TFGeneratorModule* module, TF_String* out_name) noexcept
            {
                auto& self = TFGeneratorModuleOps::from_handle(module);
                self.get_name(self.wrap(std::type_identity<ice::sonic::String>{}, out_name));
            },
            .add_function =
                [](TFGeneratorModule* module,
                   TFGeneratorFunction* function,
                   TF_Status* out_status) noexcept
            {
                auto& self = TFGeneratorModuleOps::from_handle(module);
                self.add_function(
                    self.wrap(std::type_identity<ice::sonic::TFGeneratorFunctionOps>{}, function),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_function =
                [](TFGeneratorModule* module,
                   const TF_String* name,
                   TFGeneratorFunction* out_function,
                   TF_Status* out_status) noexcept
            {
                auto& self = TFGeneratorModuleOps::from_handle(module);
                self.get_function(
                    self.wrap(std::type_identity<ice::sonic::String>{}, name),
                    self.wrap(
                        std::type_identity<ice::sonic::TFGeneratorFunctionOps>{},
                        out_function
                    ),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .list_functions =
                [](TFGeneratorModule* module,
                   TF_Tensor** out_functions,
                   TF_Status* out_status) noexcept
            {
                auto& self = TFGeneratorModuleOps::from_handle(module);
                self.list_functions(
                    out_functions,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .set_name =
                [](TFGeneratorModule* module, const TF_String* name) noexcept
            {
                auto& self = TFGeneratorModuleOps::from_handle(module);
                self.set_name(self.wrap(std::type_identity<ice::sonic::String>{}, name));
            },
            .validate =
                [](TFGeneratorModule* module, TF_Status* out_status) noexcept
            {
                auto& self = TFGeneratorModuleOps::from_handle(module);
                self.validate(self.wrap(std::type_identity<ice::sonic::Status>{}, out_status));
            },
            .emit =
                [](TFGeneratorModule* module, TF_String* out_code, TF_Status* out_status) noexcept
            {
                auto& self = TFGeneratorModuleOps::from_handle(module);
                self.emit(
                    self.wrap(std::type_identity<ice::sonic::String>{}, out_code),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },

        };
    }

    ice::sonic::TFGeneratorFunctionOps wrap(
        std::type_identity<ice::sonic::TFGeneratorFunctionOps>,
        const ::TFGeneratorFunction* handle
    ) const noexcept
    {
        return ice::sonic::TFGeneratorFunctionOps{
            m_TFGeneratorFunctionOps_ops,
            const_cast<::TFGeneratorFunction*>(handle)
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

    const ::TFGeneratorModuleOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const ::TFGeneratorModule& get_handle() const noexcept
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
        registry.register_op(type, provider, const_cast<::TFGeneratorModuleOps*>(&m_vtable));
    }

private:
    ::TFGeneratorModuleOps m_vtable;
    ::TFGeneratorModule m_handle;

    const ::TFGeneratorFunctionOps* m_TFGeneratorFunctionOps_ops{nullptr};

    const ::TF_StatusOps* m_Status_ops{nullptr};

    const ::TF_StringOps* m_String_ops{nullptr};
};

} // namespace ice::builder
