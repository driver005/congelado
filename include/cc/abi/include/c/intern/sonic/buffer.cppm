// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/intern/buffer.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/intern/buffer.h"

export module cc_abi_sonic_intern;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;
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

    [[nodiscard]] std::expected<void, ice::Status>
    assign_from_string(const void* proto, size_t proto_len) noexcept
    {
        ice::Status status;
        m_ops->assign_from_string(get_handle(), proto, proto_len, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> delete_buffer() noexcept
    {
        ice::Status status;
        m_ops->delete_buffer(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> get_buffer(TFBufferData* out_buffer) noexcept
    {
        ice::Status status;
        m_ops->get_buffer(get_handle(), out_buffer, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    ice::String get_name() const noexcept
    {
        ice::String result;
        m_ops->get_name(get_handle(), result.get_handle());
        return result;
    }
};

} // namespace ice::sonic
