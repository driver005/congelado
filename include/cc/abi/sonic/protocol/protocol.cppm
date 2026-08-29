module;

#include "c/extern/protocol/protocol.h"

export module cc_abi_sonic_protocol;

import std;
import cc_abi_sonic_intern;
import cc_abi_primitives;
import cc_abi_sonic_registration;
export namespace ice::sonic {

// ServerRuntime — cross-plugin C ABI handle, produced only by the parent runtime's factory methods.

class ServerRuntime : public ice::builder::Server
{
public:
    explicit ServerRuntime(TF_Protocol* ops, void* server_context) :
        m_ops{ops}, m_server_context{server_context}
    {
    }

    ~ServerProtocol() {
        if (m_server_context && m_ops && m_ops->server__destroy) {
            m_ops->server__destroy(m_server_context);
        }
    }

    std::expected<void, ice::Status> start()
    {


        ice::Status status;
        m_ops->server__start(m_server_context, status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    std::expected<void, ice::Status> stop()
    {


        ice::Status status;
        m_ops->server__stop(m_server_context, status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    std::expected<bool, ice::Status> is_running()
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
class Protocol : public ice::sonic::Runtime<Protocol, TF_Protocol, /*PassNameToFactory=*/true>
{
public:
    static constexpr std::string_view domain_name = "protocol";

    std::expected<std::unique_ptr<ice::builder::Server>, ice::Status>
    create_server()
    {


        ice::Status status;
        void* handle =
            this->m_ops->create_server(this->get_handle(), status.get_handle());
        if (!status.ok()) {
            if (handle) {
                this->m_ops->server__destroy(handle);
            }
            return std::unexpected{status};
        }
        return std::make_unique<ServerRuntime>(this->m_ops, handle);
    }

    ice::String get_name() const
    {


        ice::String tf_name;
        this->m_ops->get_name(this->get_handle(), tf_name.get_handle());
        return std::move(tf_name);
    }

    ice::String get_bind_host() const
    {


        ice::String tf_bind_host;
        this->m_ops->get_bind_host(this->get_handle(), tf_bind_host.get_handle());
        return std::move(tf_bind_host);
    }

    std::uint16_t get_bind_port() const
    {


        return this->m_ops->get_bind_port(this->get_handle());
    }

    ice::String get_tls_cert() const
    {


        ice::String tf_tls_cert;
        this->m_ops->get_tls_cert(this->get_handle(), tf_tls_cert.get_handle());
        return std::move(tf_tls_cert);
    }

    ice::String get_tls_key() const
    {


        ice::String tf_tls_key;
        this->m_ops->get_tls_key(this->get_handle(), tf_tls_key.get_handle());
        return std::move(tf_tls_key);
    }

public:
    explicit Protocol(TF_Protocol* ops, void* plugin_context) : Runtime(ops, plugin_context) {}
};

} // namespace ice::sonic
