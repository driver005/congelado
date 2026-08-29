module;

#include "c/intern/tf_buffer.h"

export module cc_abi_sonic_intern:buffer;

import std;
import :runtime;
import cc_abi_primitives;

export namespace ice::sonic {

class Buffer : public Runtime<Buffer, TF_Buffer>
{
public:
    static constexpr std::string_view domain_name = "buffer";

    ice::String get_name() const noexcept
    {
        ice::String out;
        m_ops->get_name(m_host_context, out.get_handle());
        return out;
    }

    // Range-first: the C vtable carries (const void*, size_t) — this C++ interface takes a
    // span and adapts at the single boundary line below.
    [[nodiscard]] std::expected<TF_Buffer_Handle*, ice::Status>
    new_buffer_from_string(std::span<const std::byte> proto) noexcept
    {
        TF_Buffer_Handle* handle =
            m_ops->new_buffer_from_string(m_host_context, proto.data(), proto.size());
        if (handle == nullptr) {
            return std::unexpected{ice::Status{"buffer allocation failed"}};
        }
        return handle;
    }

    [[nodiscard]] std::expected<TF_Buffer_Handle*, ice::Status> new_buffer() noexcept
    {
        TF_Buffer_Handle* handle = m_ops->new_buffer(m_host_context);
        if (handle == nullptr) {
            return std::unexpected{ice::Status{"buffer allocation failed"}};
        }
        return handle;
    }

    void delete_buffer(TF_Buffer_Handle* buffer) noexcept
    {
        m_ops->delete_buffer(m_host_context, buffer);
    }

    TF_Buffer_Data get_buffer(TF_Buffer_Handle* buffer) noexcept
    {
        return m_ops->get_buffer(m_host_context, buffer);
    }
};

} // namespace ice::sonic
