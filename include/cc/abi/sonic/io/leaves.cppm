module;

#include "c/extern/io/io.h"

export module cc_abi_sonic_io:leaves;

import std;
import cc_abi_sonic_intern;
import cc_abi_primitives;
export namespace ice::sonic::io {

// RequestRuntime/ResponseRuntime — cross-plugin C ABI handle, produced only by the parent runtime's factory methods.

class RequestRuntime : public ice::builder::Request
{
public:
    ~RequestRuntime() { if (m_handle && m_ops) m_ops->request__destroy(m_handle); }

    RequestRuntime(const RequestRuntime&) = delete;
    RequestRuntime& operator=(const RequestRuntime&) = delete;
    RequestRuntime(RequestRuntime&&) = delete;
    RequestRuntime& operator=(RequestRuntime&&) = delete;

    explicit RequestRuntime(TF_IO* ops, void* handle) : m_ops{ops}, m_handle{handle} {}

    ice::builder::Method get_method()
    {


        return ice::builder::method_from_c(m_ops->request__get_method(m_handle));
    }

    ice::String get_path()
    {


        ice::String tf_path;
        m_ops->request__get_path(m_handle, tf_path.get_handle());
        return std::move(tf_path);
    }

    std::expected<void, ice::Status> set_header(
        const ice::String& name, const ice::String& value
    )
    {


        ice::Status status;
        m_ops->request__set_header(
            m_handle, name.get_handle(), value.get_handle(), status.get_handle()
        );
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    std::expected<void, ice::Status> set_body(const ice::String& body
    )
    {


        ice::Status status;
        m_ops->request__set_body(m_handle, body.get_handle(), status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

private:
    TF_IO* m_ops; void* m_handle;
};

class ResponseRuntime : public ice::builder::Response
{
public:
    ~ResponseRuntime() { if (m_handle && m_ops) m_ops->response__destroy(m_handle); }

    ResponseRuntime(const ResponseRuntime&) = delete;
    ResponseRuntime& operator=(const ResponseRuntime&) = delete;
    ResponseRuntime(ResponseRuntime&&) = delete;
    ResponseRuntime& operator=(ResponseRuntime&&) = delete;

    explicit ResponseRuntime(TF_IO* ops, void* handle) : m_ops{ops}, m_handle{handle} {}

    std::expected<void, ice::Status> set_status(std::int32_t status_code)
    {


        ice::Status status;
        m_ops->response__set_status(m_handle, status_code, status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    std::expected<void, ice::Status> set_header(
        const ice::String& name, const ice::String& value
    )
    {


        ice::Status status;
        m_ops->response__set_header(
            m_handle, name.get_handle(), value.get_handle(), status.get_handle()
        );
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    std::expected<void, ice::Status> set_body(const ice::String& body
    )
    {


        ice::Status status;
        m_ops->response__set_body(m_handle, body.get_handle(), status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

private:
    TF_IO* m_ops; void* m_handle;
};

} // namespace ice::sonic
