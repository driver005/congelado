// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from c/extern/buffer/buffer.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "c/extern/buffer/buffer.h"

export module cc_abi_sonic_buffer;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;
import cc_abi_sonic_registration;

export namespace ice::sonic {

class Buffer : public ice::sonic::Runtime<Buffer, TF_Buffer>
{
public:
    explicit Buffer(TF_Buffer* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "buffer";

    [[nodiscard]] std::expected<void, ice::Status>
    new_buffer_from_string(const void* proto, size_t proto_len) noexcept
    {
        ice::Status status;
        m_ops->new_buffer_from_string(get_handle(), proto, proto_len, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> new_buffer() noexcept
    {
        ice::Status status;
        m_ops->new_buffer(get_handle(), status.get_handle());

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

    [[nodiscard]] std::expected<void, ice::Status> get_buffer() noexcept
    {
        ice::Status status;
        m_ops->get_buffer(get_handle(), status.get_handle());

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
