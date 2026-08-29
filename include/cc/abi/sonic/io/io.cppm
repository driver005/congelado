module;

#include "c/extern/io/io.h"

export module cc_abi_sonic_io;

export import :leaves;
export import :enums;
import std;
import cc_abi_sonic_intern;
import cc_abi_primitives;
import cc_abi_sonic_registration;
import :leaves;


export namespace ice::sonic {

// Runtime — the mainframe-facing IO handle. Same in-process/cross-plugin duality as
// ice::sonic::Cache and ice::sonic::Generator.
class Io : public ice::sonic::Runtime<Io, TF_IO, /*PassNameToFactory=*/true>
{
public:
    static constexpr std::string_view domain_name = "io";

    std::expected<std::unique_ptr<ice::builder::Request>, ice::Status>
    create_request()
    {


        ice::Status status;
        void* handle = this->m_ops->create_request(this->get_handle(), status.get_handle());
        if (!status.ok()) {
            if (handle) {
                this->m_ops->request__destroy(handle);
            }
            return std::unexpected{status};
        }
        return std::make_unique<RequestRuntime>(this->m_ops, handle);
    }

    std::expected<std::unique_ptr<ice::builder::Response>, ice::Status>
    create_response()
    {


        ice::Status status;
        void* handle =
            this->m_ops->create_response(this->get_handle(), status.get_handle());
        if (!status.ok()) {
            if (handle) {
                this->m_ops->response__destroy(handle);
            }
            return std::unexpected{status};
        }
        return std::make_unique<ResponseRuntime>(this->m_ops, handle);
    }

public:
    explicit Io(TF_IO* ops, void* plugin_context) : Runtime(ops, plugin_context) {}
};

} // namespace ice::sonic
