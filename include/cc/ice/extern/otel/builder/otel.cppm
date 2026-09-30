// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/otel/otel.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/otel/otel.h"

export module cc_ice_extern_otel_builder:otel;

import std;

export namespace ice::builder {

class TF_OtelOps
{
public:
    TF_OtelOps() noexcept :
        m_handle{.plugin_data = this}
    {
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

    void get_generic_vtable() noexcept
    {
        m_vtable = ::TF_OtelOps{
            .struct_size = TF_OTEL_STRUCT_SIZE,
            .destroy =
                [](TF_Otel* otel) noexcept
            {
                TF_OtelOps::from_handle(otel).destroy();
            },
            .get_name =
                [](TF_Otel* otel, TF_String* out_name) noexcept
            {
                TF_OtelOps::from_handle(otel).get_name(ice::sonic::String::wrap(out_name));
            },

        };
    }

    const ::TF_OtelOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const TF_Otel& get_handle() const noexcept
    {
        return m_handle;
    }


private:
    ::TF_OtelOps m_vtable;
    TF_Otel m_handle;
};

} // namespace ice::builder
