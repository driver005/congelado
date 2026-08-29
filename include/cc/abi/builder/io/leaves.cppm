export module cc_abi_builder_io:leaves;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;
import :enums;

export namespace ice::builder::io {


// Request/Response — produced by Io::create_request/create_response, no independent existence
// outside their producing Io (mirrors ice::builder::Function's relationship to
// Builder::enter_border_patrol). get_method/get_path are plain accessors (no expected wrapper —
// the underlying C ABI never had a failure channel for them either).

class Request
{
public:
    virtual ~Request() = default;

    virtual Method get_method() = 0;
    virtual ice::String get_path() = 0;

    virtual std::expected<void, ice::Status>
    set_header(const ice::String& name, const ice::String& value) = 0;
    virtual std::expected<void, ice::Status> set_body(const ice::String& body
    ) = 0;
};

class Response
{
public:
    virtual ~Response() = default;

    virtual std::expected<void, ice::Status> set_status(std::int32_t status_code) = 0;
    virtual std::expected<void, ice::Status>
    set_header(const ice::String& name, const ice::String& value) = 0;
    virtual std::expected<void, ice::Status> set_body(const ice::String& body
    ) = 0;
};

} // namespace ice::builder
