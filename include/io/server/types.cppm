export module io_server:types;

import std;

export namespace transport::server {

enum class MethodBitMask : std::uint8_t {
    GET = 1 << 0,
    POST = 1 << 1,
    PUT = 1 << 2,
    DELETE = 1 << 3,
    PATCH = 1 << 4,
    HEAD = 1 << 5,
    OPTIONS = 1 << 6,
};

enum class EdgeKind : std::uint8_t { Path = 0, Param = 1, Wild = 2 };

enum class Method : std::uint8_t {
    GET = 0,
    POST = 1,
    PUT = 2,
    DELETE = 3,
    PATCH = 4,
    HEAD = 5,
    OPTIONS = 6,
    COUNT_ = 7
};


} // namespace transport::server
