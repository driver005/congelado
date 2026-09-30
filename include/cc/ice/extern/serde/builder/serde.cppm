// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/serde/serde.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/serde/serde.h"

export module cc_ice_extern_serde_builder:serde;

import std;

export namespace ice::builder {

class TF_SerdeOps
{
public:
    TF_SerdeOps() noexcept :
        m_handle{.plugin_data = this}
    {
    }

    TF_SerdeOps(const TF_SerdeOps&) = delete;
    TF_SerdeOps& operator=(const TF_SerdeOps&) = delete;

    static TF_SerdeOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TF_SerdeOps*>(ctx);
    }

    template<typename HandleT>
    static TF_SerdeOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TF_SerdeOps*>(handle->plugin_data);
    }

    virtual ~TF_SerdeOps() = default;
    virtual void destroy() noexcept = 0;
    virtual void get_name(const ice::sonic::String& out_name) noexcept = 0;
    virtual void get_content_type(const ice::sonic::String& out_content_type) noexcept = 0;
    virtual void get_format_name(const ice::sonic::String& out_format_name) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status> encode(
        const ice::sonic::String& value_json,
        const ice::sonic::String& out_encoded
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    decode(const ice::sonic::String& data, const ice::sonic::String& out_json) noexcept = 0;

    void get_generic_vtable() noexcept
    {
        m_vtable = ::TF_SerdeOps{
            .struct_size = TF_SERDE_STRUCT_SIZE,
            .destroy =
                [](TF_Serde* serde) noexcept
            {
                TF_SerdeOps::from_handle(serde).destroy();
            },
            .get_name =
                [](TF_Serde* serde, TF_String* out_name) noexcept
            {
                TF_SerdeOps::from_handle(serde).get_name(ice::sonic::String::wrap(out_name));
            },
            .get_content_type =
                [](TF_Serde* serde, TF_String* out_content_type) noexcept
            {
                TF_SerdeOps::from_handle(serde).get_content_type(
                    ice::sonic::String::wrap(out_content_type)
                );
            },
            .get_format_name =
                [](TF_Serde* serde, TF_String* out_format_name) noexcept
            {
                TF_SerdeOps::from_handle(serde).get_format_name(
                    ice::sonic::String::wrap(out_format_name)
                );
            },
            .encode =
                [](TF_Serde* serde,
                   const TF_String* value_json,
                   TF_String* out_encoded,
                   TF_Status* out_status) noexcept
            {
                auto res = TF_SerdeOps::from_handle(serde).encode(
                    ice::sonic::String::wrap(value_json),
                    ice::sonic::String::wrap(out_encoded)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .decode =
                [](TF_Serde* serde,
                   const TF_String* data,
                   TF_String* out_json,
                   TF_Status* out_status) noexcept
            {
                auto res = TF_SerdeOps::from_handle(serde).decode(
                    ice::sonic::String::wrap(data),
                    ice::sonic::String::wrap(out_json)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },

        };
    }

    const ::TF_SerdeOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const TF_Serde& get_handle() const noexcept
    {
        return m_handle;
    }


private:
    ::TF_SerdeOps m_vtable;
    TF_Serde m_handle;
};

} // namespace ice::builder
