module;

#include "c/extern/io/io.h"

export module cc_abi_sonic_io:leaves;

import std;
import cc_abi_sonic_intern;
import cc_abi_primitives;

export namespace ice::sonic {

// Request/Response — cross-plugin C ABI handle, produced only by the parent runtime's
// factory methods.

class Request
{
public:
    ~Request()
    {
        if (m_ops && m_handle) {
            m_ops->request__destroy(m_handle);
        }
    }

    Request(const Request&) = delete;
    Request& operator=(const Request&) = delete;
    Request(Request&&) = delete;
    Request& operator=(Request&&) = delete;

    explicit Request(TF_IO* ops, TF_IO_Request* handle) :
        m_ops{ops},
        m_handle{handle}
    {
    }

    ice::Method get_method()
    {
        return ice::method_from_c(m_ops->request__get_method(m_handle));
    }

    ice::String get_path()
    {
        ice::String tf_path;
        m_ops->request__get_path(m_handle, tf_path.get_handle());
        return tf_path;
    }

    [[nodiscard]] std::expected<void, ice::Status>
    set_header(const ice::String& name, const ice::String& value)
    {
        ice::Status status;
        m_ops->request__set_header(
            m_handle,
            name.get_handle(),
            value.get_handle(),
            status.get_handle()
        );
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> set_body(const ice::String& body)
    {
        ice::Status status;
        m_ops->request__set_body(m_handle, body.get_handle(), status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

private:
    TF_IO* m_ops;
    TF_IO_Request* m_handle;
};

class Response
{
public:
    ~Response()
    {
        if (m_ops && m_handle) {
            m_ops->response__destroy(m_handle);
        }
    }

    Response(const Response&) = delete;
    Response& operator=(const Response&) = delete;
    Response(Response&&) = delete;
    Response& operator=(Response&&) = delete;

    explicit Response(TF_IO* ops, TF_IO_Response* handle) :
        m_ops{ops},
        m_handle{handle}
    {
    }

    [[nodiscard]] std::expected<void, ice::Status> set_status(std::int32_t status_code)
    {
        ice::Status status;
        m_ops->response__set_status(m_handle, status_code, status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    set_header(const ice::String& name, const ice::String& value)
    {
        ice::Status status;
        m_ops->response__set_header(
            m_handle,
            name.get_handle(),
            value.get_handle(),
            status.get_handle()
        );
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> set_body(const ice::String& body)
    {
        ice::Status status;
        m_ops->response__set_body(m_handle, body.get_handle(), status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

private:
    TF_IO* m_ops;
    TF_IO_Response* m_handle;
};

} // namespace ice::sonic
