// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/otel/otel.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/otel/otel.h"
#include "include/c/intern/tstring.h"

export module cc_ice_extern_otel_builder:otel;

import std;
import cc_ice_intern_sonic;

export namespace ice::builder {

class TF_OtelOps
{
public:
    explicit TF_OtelOps(const ::TF_StringOps* String_ops) noexcept :
        m_handle{.plugin_data = this}
    {
        m_String_ops = String_ops;
    }

    TF_OtelOps(const TF_OtelOps&) = delete;
    TF_OtelOps& operator=(const TF_OtelOps&) = delete;

    static TF_OtelOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TF_OtelOps*>(ctx);
    }

    template<typename HandleT>
    static TF_OtelOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TF_OtelOps*>(handle->plugin_data);
    }

    virtual ~TF_OtelOps() = default;
    virtual void destroy() noexcept = 0;
    virtual void get_name(const ice::sonic::String& out_name) noexcept = 0;

    void get_generic_vtable(void (*create)(::TF_Otel*)) noexcept
    {
        m_vtable = ::TF_OtelOps{
            .struct_size = TF_OFFSET_OF_END(::TF_OtelOps, get_name),

            .create = create,
            .destroy =
                [](TF_Otel* handle) noexcept
            {
                auto& self = TF_OtelOps::from_handle(handle);
                self.destroy();
            },
            .get_name =
                [](TF_Otel* otel, TF_String* out_name) noexcept
            {
                auto& self = TF_OtelOps::from_handle(otel);
                self.get_name(self.wrap(std::type_identity<ice::sonic::String>{}, out_name));
            },

        };
    }

    ice::sonic::String
    wrap(std::type_identity<ice::sonic::String>, const ::TF_String* handle) const noexcept
    {
        return ice::sonic::String{m_String_ops, const_cast<::TF_String*>(handle)};
    }

    const ::TF_OtelOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const ::TF_Otel& get_handle() const noexcept
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
        registry.register_op(type, provider, const_cast<::TF_OtelOps*>(&m_vtable));
    }

private:
    ::TF_OtelOps m_vtable;
    ::TF_Otel m_handle;

    const ::TF_StringOps* m_String_ops{nullptr};
};

} // namespace ice::builder
