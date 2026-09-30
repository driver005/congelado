// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/intern/buffer.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/intern/buffer.h"

export module cc_ice_intern_sonic:buffer;

import std;
import cc_abi_sonic_registration;

export namespace ice::sonic {

class TF_BufferOps : public ice::sonic::Runtime<TF_BufferOps, TF_BufferOps>
{
public:
    explicit TF_BufferOps(TF_BufferOps* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "intern";

    void get_name(const ice::sonic::String& out_name) noexcept
    {
        m_ops->get_name(get_handle(), out_name.get_handle());
    }

    void assign_from_string(const void* proto, size_t proto_len) noexcept
    {
        m_ops->assign_from_string(get_handle(), proto, proto_len);
    }

    void delete_buffer() noexcept
    {
        m_ops->delete_buffer(get_handle());
    }

    void get_buffer(TFBufferData* out_buffer) noexcept
    {
        m_ops->get_buffer(get_handle(), out_buffer);
    }
};

} // namespace ice::sonic
