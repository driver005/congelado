// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/serde/serde.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/serde/serde.h"

export module cc_ice_extern_serde_sonic:serde;

import std;
import cc_ice_intern_sonic;

export namespace ice::sonic {

class TF_SerdeOps : public ice::sonic::Runtime<::TF_SerdeOps, ::TF_Serde>
{
public:
    template<typename Registry>
    TF_SerdeOps(
        Registry& registry,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry, type, provider)
    {
    }

    template<typename Registry>
    TF_SerdeOps(
        Registry& registry,
        ::TF_Serde* handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry, handle, type, provider)
    {
    }

    explicit TF_SerdeOps(const ::TF_SerdeOps* ops) noexcept :
        Runtime(ops)
    {
    }

    TF_SerdeOps(const ::TF_SerdeOps* ops, ::TF_Serde* handle) noexcept :
        Runtime(ops, handle)
    {
    }

    void create() const noexcept
    {
        m_ops->create(get_handle());
    }

    void destroy() const noexcept
    {
        m_ops->destroy(get_handle());
    }

    void get_name(const ice::sonic::String& out_name) const noexcept
    {
        m_ops->get_name(get_handle(), out_name.get_handle());
    }

    void get_content_type(const ice::sonic::String& out_content_type) const noexcept
    {
        m_ops->get_content_type(get_handle(), out_content_type.get_handle());
    }

    void get_format_name(const ice::sonic::String& out_format_name) const noexcept
    {
        m_ops->get_format_name(get_handle(), out_format_name.get_handle());
    }

    void encode(
        const ice::sonic::String& value_json,
        const ice::sonic::String& out_encoded,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->encode(
            get_handle(),
            value_json.get_handle(),
            out_encoded.get_handle(),
            out_status.get_handle()
        );
    }

    void decode(
        const ice::sonic::String& data,
        const ice::sonic::String& out_json,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->decode(
            get_handle(),
            data.get_handle(),
            out_json.get_handle(),
            out_status.get_handle()
        );
    }
};

} // namespace ice::sonic
