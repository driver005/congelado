module;

#include "c/extern/io/io.h"

export module cc_abi_sonic_io;

import std;
export import :leaves;
import cc_abi_primitives;
import cc_abi_sonic_intern;
import cc_abi_sonic_registration;

export namespace ice::sonic {

// Runtime — the mainframe-facing IO handle. Same in-process/cross-plugin duality as
// ice::sonic::Cache and ice::sonic::Generator.
class Io : public ice::sonic::Runtime<Io, TF_IO>
{
public:
    explicit Io(TF_IO* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "io";

    ice::String get_name() const noexcept
    {
        ice::String out;
        m_ops->get_name(get_handle(), out.get_handle());
        return out;
    }

    [[nodiscard]] std::expected<std::unique_ptr<ice::sonic::Request>, ice::Status> create_request() noexcept
    {
        ice::Status status;
        TF_IO_Request* handle = m_ops->create_request(get_handle(), status.get_handle());
        if (!status.ok()) {
            if (handle) {
                m_ops->request__destroy(handle);
            }
            return std::unexpected{status};
        }
        return std::make_unique<ice::sonic::Request>(m_ops, handle);
    }

    std::unique_ptr<ice::sonic::Response> create_response() noexcept
    {
        ice::Status status;
        TF_IO_Response* handle = m_ops->create_response(get_handle(), status.get_handle());
        if (!status.ok()) {
            if (handle) {
                m_ops->response__destroy(handle);
            }
            return nullptr;
        }
        return std::make_unique<ice::sonic::Response>(m_ops, handle);
    }
};

} // namespace ice::sonic
