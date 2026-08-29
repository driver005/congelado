module;

#include "c/extern/protocol/protocol.h"

export module cc_abi_sonic_protocol;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;
import cc_abi_sonic_registration;

export namespace ice::sonic {

// Server — cross-plugin C ABI handle, produced only by the parent runtime's factory methods.

class Server
{
public:
    explicit Server(TF_Protocol* ops, void* server_context) noexcept :
        m_ops{ops},
        m_server_context{server_context}
    {
    }

    ~Server()
    {
        if (m_server_context && m_ops && m_ops->server__destroy) {
            m_ops->server__destroy(m_server_context);
        }
    }

    Server(const Server&) = delete;
    Server& operator=(const Server&) = delete;
    Server(Server&&) = delete;
    Server& operator=(Server&&) = delete;

    [[nodiscard]] std::expected<void, ice::Status> start() noexcept
    {
        ice::Status status;
        m_ops->server__start(m_server_context, status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> stop() noexcept
    {
        ice::Status status;
        m_ops->server__stop(m_server_context, status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<bool, ice::Status> is_running() noexcept
    {
        ice::Status status;
        bool result = m_ops->server__is_running(m_server_context, status.get_handle()) != 0;
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return result;
    }

private:
    TF_Protocol* m_ops;
    void* m_server_context;
};

// Runtime — the mainframe-facing protocol handle. Same in-process/cross-plugin duality as
// ice::sonic::Cache and ice::sonic::Generator.
class Protocol : public ice::sonic::Runtime<Protocol, TF_Protocol>
{
public:
    explicit Protocol(TF_Protocol* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "protocol";

    [[nodiscard]] std::expected<std::unique_ptr<ice::sonic::Server>, ice::Status> create_server() noexcept
    {
        ice::Status status;
        void* handle = m_ops->create_server(get_handle(), status.get_handle());
        if (!status.ok()) {
            if (handle) {
                m_ops->server__destroy(handle);
            }
            return std::unexpected{status};
        }
        return std::make_unique<ice::sonic::Server>(m_ops, handle);
    }

    ice::String get_name() const noexcept
    {
        ice::String out;
        m_ops->get_name(get_handle(), out.get_handle());
        return out;
    }

    ice::String get_bind_host() const noexcept
    {
        ice::String tf_bind_host;
        m_ops->get_bind_host(get_handle(), tf_bind_host.get_handle());
        return tf_bind_host;
    }

    std::uint16_t get_bind_port() const noexcept
    {
        return m_ops->get_bind_port(get_handle());
    }

    ice::String get_tls_cert() const noexcept
    {
        ice::String tf_tls_cert;
        m_ops->get_tls_cert(get_handle(), tf_tls_cert.get_handle());
        return tf_tls_cert;
    }

    ice::String get_tls_key() const noexcept
    {
        ice::String tf_tls_key;
        m_ops->get_tls_key(get_handle(), tf_tls_key.get_handle());
        return tf_tls_key;
    }
};

} // namespace ice::sonic
