export module interfaces:io_types;

import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace interfaces::io::types {

enum class Status : std::uint16_t {
    // 1xx Informational
    CONTINUE = 100,
    SWITCHING_PROTOCOLS = 101,
    PROCESSING = 102,
    EARLY_HINTS = 103,

    // 2xx Success
    OK = 200,
    CREATED = 201,
    ACCEPTED = 202,
    NON_AUTHORITATIVE_INFORMATION = 203,
    NO_CONTENT = 204,
    RESET_CONTENT = 205,
    PARTIAL_CONTENT = 206,

    // 3xx Redirection
    MULTIPLE_CHOICES = 300,
    MOVED_PERMANENTLY = 301,
    FOUND = 302,
    SEE_OTHER = 303,
    NOT_MODIFIED = 304,
    TEMPORARY_REDIRECT = 307,
    PERMANENT_REDIRECT = 308,

    // 4xx Client Errors
    BAD_REQUEST = 400,
    UNAUTHORIZED = 401,
    PAYMENT_REQUIRED = 402,
    FORBIDDEN = 403,
    NOT_FOUND = 404,
    METHOD_NOT_ALLOWED = 405,
    NOT_ACCEPTABLE = 406,
    REQUEST_TIMEOUT = 408,
    CONFLICT = 409,
    GONE = 410,
    LENGTH_REQUIRED = 411,
    PAYLOAD_TOO_LARGE = 413,
    URI_TOO_LONG = 414,
    UNSUPPORTED_MEDIA_TYPE = 415,
    UNPROCESSABLE_CONTENT = 422,
    TOO_MANY_REQUESTS = 429,

    // 5xx Server Errors
    INTERNAL_SERVER_ERROR = 500,
    NOT_IMPLEMENTED = 501,
    BAD_GATEWAY = 502,
    SERVICE_UNAVAILABLE = 503,
    GATEWAY_TIMEOUT = 504,
    HTTP_VERSION_NOT_SUPPORTED = 505,
};

[[nodiscard]] constexpr std::uint16_t status_code(Status status) noexcept {
    return std::to_underlying(status);
}

enum class Method : std::uint8_t {
    GET = 0,
    POST = 1,
    PUT = 2,
    DELETE = 3,
    PATCH = 4,
    HEAD = 5,
    OPTIONS = 6,
    CONNECT = 7,
    TRACE = 8,
    UNKNOWN = 9
};

[[nodiscard]] constexpr std::string_view method_str(Method method) noexcept {
    switch (method) {
    case Method::GET:
        return "GET";
    case Method::HEAD:
        return "HEAD";
    case Method::POST:
        return "POST";
    case Method::PUT:
        return "PUT";
    case Method::DELETE:
        return "DELETE";
    case Method::CONNECT:
        return "CONNECT";
    case Method::OPTIONS:
        return "OPTIONS";
    case Method::TRACE:
        return "TRACE";
    case Method::PATCH:
        return "PATCH";
    default:
        return "";
    }
}

[[nodiscard]] inline Method parse_method(std::string_view view) noexcept {
    // Empty string can't be a method, no cap — straight to unknown, nothing left to check.
    if (view.empty()) [[unlikely]] {
        return Method::UNKNOWN;
    }

    // Dispatch off the first char first — narrows every candidate down to at most a couple
    // exact-match checks instead of testing all nine methods in sequence.
    switch (view[0]) {  // FIXME(clang-tidy): unchecked operator[], consider .at()
    case 'G':
        if (view == "GET") {
            return Method::GET;
        }
        break;
    case 'H':
        if (view == "HEAD") {
            return Method::HEAD;
        }
        break;
    case 'P':
        // G/H/D/C/O/T only ever match one method apiece, but P covers three (POST/PUT/PATCH),
        // so length breaks the tie before the string compare even runs.
        switch (view.size()) {
        case 4:
            if (view == "POST") {
                return Method::POST;
            }
            break;
        case 3:
            if (view == "PUT") {
                return Method::PUT;
            }
            break;
        case 5:
            if (view == "PATCH") {
                return Method::PATCH;
            }
            break;
        default:
            break;
        }
        break;
    case 'D':
        if (view == "DELETE") {
            return Method::DELETE;
        }
        break;
    case 'C':
        if (view == "CONNECT") {
            return Method::CONNECT;
        }
        break;
    case 'O':
        if (view == "OPTIONS") {
            return Method::OPTIONS;
        }
        break;
    case 'T':
        if (view == "TRACE") {
            return Method::TRACE;
        }
        break;
    default:
        break;
    }

    return Method::UNKNOWN;
}


enum class Token : std::uint8_t {
    NONE = 0,
    AUTHORITY = 1,
    METHOD = 2,
    PATH = 3,
    SCHEME = 4,
    STATUS = 5,
    ACCEPT_CHARSET = 6,
    ACCEPT_ENCODING = 7,
    ACCEPT_LANGUAGE = 8,
    ACCEPT_RANGES = 9,
    ACCEPT = 10,
    ACCESS_CONTROL_ALLOW_ORIGIN = 11,
    AGE = 12,
    ALLOW = 13,
    AUTHORIZATION = 14,
    CACHE_CONTROL = 15,
    CONTENT_DISPOSITION = 16,
    CONTENT_ENCODING = 17,
    CONTENT_LANGUAGE = 18,
    CONTENT_LENGTH = 19,
    CONTENT_LOCATION = 20,
    CONTENT_RANGE = 21,
    CONTENT_TYPE = 22,
    COOKIE = 23,
    DATE = 24,
    E_TAG = 25,
    EXPECT = 26,
    EXPIRES = 27,
    FROM = 28,
    HOST = 29,
    IF_MATCH = 30,
    IF_MODIFIED_SINCE = 31,
    IF_NONE_MATCH = 32,
    IF_RANGE = 33,
    IF_UNMODIFIED_SINCE = 34,
    LAST_MODIFIED = 35,
    LINK = 36,
    LOCATION = 37,
    MAX_FORWARDS = 38,
    PROXY_AUTHENTICATE = 39,
    PROXY_AUTHORIZATION = 40,
    RANGE = 41,
    REFERER = 42,
    REFRESH = 43,
    RETRY_AFTER = 44,
    SERVER = 45,
    SET_COOKIE = 46,
    STRICT_TRANSPORT_SECURITY = 47,
    TRANSFER_ENCODING = 48,
    USER_AGENT = 49,
    VARY = 50,
    VIA = 51,
    WWW_AUTHENTICATE = 52,
    ACCESS_CONTROL_ALLOW_CREDENTIALS = 53,
    ACCESS_CONTROL_ALLOW_HEADERS = 54,
    ACCESS_CONTROL_ALLOW_METHODS = 55,
    ACCESS_CONTROL_EXPOSE_HEADERS = 56,
    ACCESS_CONTROL_REQUEST_HEADERS = 57,
    ACCESS_CONTROL_REQUEST_METHOD = 58,
    ALT_SVC = 59,
    CONTENT_SECURITY_POLICY = 60,
    EARLY_DATA = 61,
    EXPECT_CT = 62,
    FORWARDED = 63,
    ORIGIN = 64,
    PURPOSE = 65,
    TIMING_ALLOW_ORIGIN = 66,
    UPGRADE_INSECURE_REQUESTS = 67,
    X_CONTENT_TYPE_OPTIONS = 68,
    X_FORWARDED_FOR = 69,
    X_FRAME_OPTIONS = 70,
    X_XSS_PROTECTION = 71,
};

} // namespace interfaces::io::types

// Non-exported: per-length lookup helpers for tokenize() below. Each one holds exactly the
// handful of header names that share that exact length, so pulling them out of tokenize's
// switch drops every "if" here back to nesting level 0 (no more nesting penalty from being
// inside the outer switch) without changing which names match or the order they're checked in.
namespace interfaces::io::types {

[[nodiscard]] constexpr std::optional<Token> tokenize_length_3(std::string_view name) noexcept {
    if (name == "age") {
        return Token::AGE;
    }
    if (name == "via") {
        return Token::VIA;
    }
    return std::nullopt;
}

[[nodiscard]] constexpr std::optional<Token> tokenize_length_4(std::string_view name) noexcept {
    if (name == "date") {
        return Token::DATE;
    }
    if (name == "etag") {
        return Token::E_TAG;
    }
    if (name == "from") {
        return Token::FROM;
    }
    if (name == "host") {
        return Token::HOST;
    }
    if (name == "link") {
        return Token::LINK;
    }
    if (name == "vary") {
        return Token::VARY;
    }
    return std::nullopt;
}

[[nodiscard]] constexpr std::optional<Token> tokenize_length_5(std::string_view name) noexcept {
    if (name == ":path") {
        return Token::PATH;
    }
    if (name == "allow") {
        return Token::ALLOW;
    }
    if (name == "range") {
        return Token::RANGE;
    }
    return std::nullopt;
}

[[nodiscard]] constexpr std::optional<Token> tokenize_length_6(std::string_view name) noexcept {
    if (name == "accept") {
        return Token::ACCEPT;
    }
    if (name == "cookie") {
        return Token::COOKIE;
    }
    if (name == "expect") {
        return Token::EXPECT;
    }
    if (name == "origin") {
        return Token::ORIGIN;
    }
    if (name == "server") {
        return Token::SERVER;
    }
    return std::nullopt;
}

[[nodiscard]] constexpr std::optional<Token> tokenize_length_7(std::string_view name) noexcept {
    if (name == ":method") {
        return Token::METHOD;
    }
    if (name == ":scheme") {
        return Token::SCHEME;
    }
    if (name == ":status") {
        return Token::STATUS;
    }
    if (name == "alt-svc") {
        return Token::ALT_SVC;
    }
    if (name == "expires") {
        return Token::EXPIRES;
    }
    if (name == "purpose") {
        return Token::PURPOSE;
    }
    if (name == "referer") {
        return Token::REFERER;
    }
    if (name == "refresh") {
        return Token::REFRESH;
    }
    return std::nullopt;
}

[[nodiscard]] constexpr std::optional<Token> tokenize_length_8(std::string_view name) noexcept {
    if (name == "if-match") {
        return Token::IF_MATCH;
    }
    if (name == "if-range") {
        return Token::IF_RANGE;
    }
    if (name == "location") {
        return Token::LOCATION;
    }
    return std::nullopt;
}

[[nodiscard]] constexpr std::optional<Token> tokenize_length_9(std::string_view name) noexcept {
    if (name == "expect-ct") {
        return Token::EXPECT_CT;
    }
    if (name == "forwarded") {
        return Token::FORWARDED;
    }
    return std::nullopt;
}

[[nodiscard]] constexpr std::optional<Token> tokenize_length_10(std::string_view name) noexcept {
    if (name == ":authority") {
        return Token::AUTHORITY;
    }
    if (name == "early-data") {
        return Token::EARLY_DATA;
    }
    if (name == "set-cookie") {
        return Token::SET_COOKIE;
    }
    if (name == "user-agent") {
        return Token::USER_AGENT;
    }
    return std::nullopt;
}

[[nodiscard]] constexpr std::optional<Token> tokenize_length_11(std::string_view name) noexcept {
    if (name == "retry-after") {
        return Token::RETRY_AFTER;
    }
    return std::nullopt;
}

[[nodiscard]] constexpr std::optional<Token> tokenize_length_12(std::string_view name) noexcept {
    if (name == "accept-ranges") {
        return Token::ACCEPT_RANGES;
    }
    if (name == "content-type") {
        return Token::CONTENT_TYPE;
    }
    if (name == "max-forwards") {
        return Token::MAX_FORWARDS;
    }
    return std::nullopt;
}

[[nodiscard]] constexpr std::optional<Token> tokenize_length_13(std::string_view name) noexcept {
    if (name == "authorization") {
        return Token::AUTHORIZATION;
    }
    if (name == "cache-control") {
        return Token::CACHE_CONTROL;
    }
    if (name == "content-range") {
        return Token::CONTENT_RANGE;
    }
    if (name == "if-none-match") {
        return Token::IF_NONE_MATCH;
    }
    if (name == "last-modified") {
        return Token::LAST_MODIFIED;
    }
    return std::nullopt;
}

[[nodiscard]] constexpr std::optional<Token> tokenize_length_14(std::string_view name) noexcept {
    if (name == "accept-charset") {
        return Token::ACCEPT_CHARSET;
    }
    if (name == "content-length") {
        return Token::CONTENT_LENGTH;
    }
    return std::nullopt;
}

[[nodiscard]] constexpr std::optional<Token> tokenize_length_15(std::string_view name) noexcept {
    if (name == "accept-encoding") {
        return Token::ACCEPT_ENCODING;
    }
    if (name == "accept-language") {
        return Token::ACCEPT_LANGUAGE;
    }
    return std::nullopt;
}

[[nodiscard]] constexpr std::optional<Token> tokenize_length_16(std::string_view name) noexcept {
    if (name == "content-encoding") {
        return Token::CONTENT_ENCODING;
    }
    if (name == "content-language") {
        return Token::CONTENT_LANGUAGE;
    }
    if (name == "content-location") {
        return Token::CONTENT_LOCATION;
    }
    if (name == "www-authenticate") {
        return Token::WWW_AUTHENTICATE;
    }
    return std::nullopt;
}

[[nodiscard]] constexpr std::optional<Token> tokenize_length_17(std::string_view name) noexcept {
    if (name == "if-modified-since") {
        return Token::IF_MODIFIED_SINCE;
    }
    if (name == "transfer-encoding") {
        return Token::TRANSFER_ENCODING;
    }
    return std::nullopt;
}

[[nodiscard]] constexpr std::optional<Token> tokenize_length_18(std::string_view name) noexcept {
    if (name == "proxy-authenticate") {
        return Token::PROXY_AUTHENTICATE;
    }
    if (name == "x-xss-protection") {
        return Token::X_XSS_PROTECTION;
    }
    return std::nullopt;
}

[[nodiscard]] constexpr std::optional<Token> tokenize_length_19(std::string_view name) noexcept {
    if (name == "content-disposition") {
        return Token::CONTENT_DISPOSITION;
    }
    if (name == "if-unmodified-since") {
        return Token::IF_UNMODIFIED_SINCE;
    }
    if (name == "proxy-authorization") {
        return Token::PROXY_AUTHORIZATION;
    }
    if (name == "timing-allow-origin") {
        return Token::TIMING_ALLOW_ORIGIN;
    }
    if (name == "x-frame-options") {
        return Token::X_FRAME_OPTIONS;
    }
    return std::nullopt;
}

[[nodiscard]] constexpr std::optional<Token> tokenize_length_20(std::string_view name) noexcept {
    if (name == "x-forwarded-for") {
        return Token::X_FORWARDED_FOR;
    }
    return std::nullopt;
}

[[nodiscard]] constexpr std::optional<Token> tokenize_length_22(std::string_view name) noexcept {
    if (name == "x-content-type-options") {
        return Token::X_CONTENT_TYPE_OPTIONS;
    }
    return std::nullopt;
}

[[nodiscard]] constexpr std::optional<Token> tokenize_length_23(std::string_view name) noexcept {
    if (name == "content-security-policy") {
        return Token::CONTENT_SECURITY_POLICY;
    }
    return std::nullopt;
}

[[nodiscard]] constexpr std::optional<Token> tokenize_length_25(std::string_view name) noexcept {
    if (name == "strict-transport-security") {
        return Token::STRICT_TRANSPORT_SECURITY;
    }
    if (name == "upgrade-insecure-requests") {
        return Token::UPGRADE_INSECURE_REQUESTS;
    }
    return std::nullopt;
}

[[nodiscard]] constexpr std::optional<Token> tokenize_length_27(std::string_view name) noexcept {
    if (name == "access-control-allow-origin") {
        return Token::ACCESS_CONTROL_ALLOW_ORIGIN;
    }
    return std::nullopt;
}

[[nodiscard]] constexpr std::optional<Token> tokenize_length_28(std::string_view name) noexcept {
    if (name == "access-control-allow-headers") {
        return Token::ACCESS_CONTROL_ALLOW_HEADERS;
    }
    if (name == "access-control-allow-methods") {
        return Token::ACCESS_CONTROL_ALLOW_METHODS;
    }
    return std::nullopt;
}

[[nodiscard]] constexpr std::optional<Token> tokenize_length_29(std::string_view name) noexcept {
    if (name == "access-control-expose-headers") {
        return Token::ACCESS_CONTROL_EXPOSE_HEADERS;
    }
    if (name == "access-control-request-method") {
        return Token::ACCESS_CONTROL_REQUEST_METHOD;
    }
    return std::nullopt;
}

[[nodiscard]] constexpr std::optional<Token> tokenize_length_30(std::string_view name) noexcept {
    if (name == "access-control-request-headers") {
        return Token::ACCESS_CONTROL_REQUEST_HEADERS;
    }
    return std::nullopt;
}

[[nodiscard]] constexpr std::optional<Token> tokenize_length_32(std::string_view name) noexcept {
    if (name == "access-control-allow-credentials") {
        return Token::ACCESS_CONTROL_ALLOW_CREDENTIALS;
    }
    return std::nullopt;
}

} // namespace interfaces::io::types

export namespace interfaces::io::types {

[[nodiscard]] constexpr std::optional<Token> tokenize(std::string_view name) noexcept {
    // Bucket by string length first — most header names don't collide on anything else, so this
    // shrinks the actual string comparisons down to just the handful sharing that exact length.
    // Each bucket's string compares live in tokenize_length_N() above; dispatch here is a flat
    // switch with one call per case, so this function itself carries no branching complexity.
    switch (name.length()) {
    case 3:
        return tokenize_length_3(name);
    case 4:
        return tokenize_length_4(name);
    case 5:
        return tokenize_length_5(name);
    case 6:
        return tokenize_length_6(name);
    case 7:
        return tokenize_length_7(name);
    case 8:
        return tokenize_length_8(name);
    case 9:
        return tokenize_length_9(name);
    case 10:
        return tokenize_length_10(name);
    case 11:
        return tokenize_length_11(name);
    case 12:
        return tokenize_length_12(name);
    case 13:
        return tokenize_length_13(name);
    case 14:
        return tokenize_length_14(name);
    case 15:
        return tokenize_length_15(name);
    case 16:
        return tokenize_length_16(name);
    case 17:
        return tokenize_length_17(name);
    case 18:
        return tokenize_length_18(name);
    case 19:
        return tokenize_length_19(name);
    case 20:
        return tokenize_length_20(name);
    case 22:
        return tokenize_length_22(name);
    case 23:
        return tokenize_length_23(name);
    case 25:
        return tokenize_length_25(name);
    case 27:
        return tokenize_length_27(name);
    case 28:
        return tokenize_length_28(name);
    case 29:
        return tokenize_length_29(name);
    case 30:
        return tokenize_length_30(name);
    case 32:
        return tokenize_length_32(name);
    default:
        return std::nullopt;
    }
}

constexpr std::string_view token_to_string(const Token &tkst) noexcept {
    switch (tkst) {
    case Token::AUTHORITY:
        return ":authority";
    case Token::METHOD:
        return ":method";
    case Token::PATH:
        return ":path";
    case Token::SCHEME:
        return ":scheme";
    case Token::STATUS:
        return ":status";
    case Token::ACCEPT_CHARSET:
        return "accept-charset";
    case Token::ACCEPT_ENCODING:
        return "accept-encoding";
    case Token::ACCEPT_LANGUAGE:
        return "accept-language";
    case Token::ACCEPT_RANGES:
        return "accept-ranges";
    case Token::ACCEPT:
        return "accept";
    case Token::ACCESS_CONTROL_ALLOW_ORIGIN:
        return "access-control-allow-origin";
    case Token::AGE:
        return "age";
    case Token::ALLOW:
        return "allow";
    case Token::AUTHORIZATION:
        return "authorization";
    case Token::CACHE_CONTROL:
        return "cache-control";
    case Token::CONTENT_DISPOSITION:
        return "content-disposition";
    case Token::CONTENT_ENCODING:
        return "content-encoding";
    case Token::CONTENT_LANGUAGE:
        return "content-language";
    case Token::CONTENT_LENGTH:
        return "content-length";
    case Token::CONTENT_LOCATION:
        return "content-location";
    case Token::CONTENT_RANGE:
        return "content-range";
    case Token::CONTENT_TYPE:
        return "content-type";
    case Token::COOKIE:
        return "cookie";
    case Token::DATE:
        return "date";
    case Token::E_TAG:
        return "etag";
    case Token::EXPECT:
        return "expect";
    case Token::EXPIRES:
        return "expires";
    case Token::FROM:
        return "from";
    case Token::HOST:
        return "host";
    case Token::IF_MATCH:
        return "if-match";
    case Token::IF_MODIFIED_SINCE:
        return "if-modified-since";
    case Token::IF_NONE_MATCH:
        return "if-none-match";
    case Token::IF_RANGE:
        return "if-range";
    case Token::IF_UNMODIFIED_SINCE:
        return "if-unmodified-since";
    case Token::LAST_MODIFIED:
        return "last-modified";
    case Token::LINK:
        return "link";
    case Token::LOCATION:
        return "location";
    case Token::MAX_FORWARDS:
        return "max-forwards";
    case Token::PROXY_AUTHENTICATE:
        return "proxy-authenticate";
    case Token::PROXY_AUTHORIZATION:
        return "proxy-authorization";
    case Token::RANGE:
        return "range";
    case Token::REFERER:
        return "referer";
    case Token::REFRESH:
        return "refresh";
    case Token::RETRY_AFTER:
        return "retry-after";
    case Token::SERVER:
        return "server";
    case Token::SET_COOKIE:
        return "set-cookie";
    case Token::STRICT_TRANSPORT_SECURITY:
        return "strict-transport-security";
    case Token::TRANSFER_ENCODING:
        return "transfer-encoding";
    case Token::USER_AGENT:
        return "user-agent";
    case Token::VARY:
        return "vary";
    case Token::VIA:
        return "via";
    case Token::WWW_AUTHENTICATE:
        return "www-authenticate";
    case Token::ACCESS_CONTROL_ALLOW_CREDENTIALS:
        return "access-control-allow-credentials";
    case Token::ACCESS_CONTROL_ALLOW_HEADERS:
        return "access-control-allow-headers";
    case Token::ACCESS_CONTROL_ALLOW_METHODS:
        return "access-control-allow-methods";
    case Token::ACCESS_CONTROL_EXPOSE_HEADERS:
        return "access-control-expose-headers";
    case Token::ACCESS_CONTROL_REQUEST_HEADERS:
        return "access-control-request-headers";
    case Token::ACCESS_CONTROL_REQUEST_METHOD:
        return "access-control-request-method";
    case Token::ALT_SVC:
        return "alt-svc";
    case Token::CONTENT_SECURITY_POLICY:
        return "content-security-policy";
    case Token::EARLY_DATA:
        return "early-data";
    case Token::EXPECT_CT:
        return "expect-ct";
    case Token::FORWARDED:
        return "forwarded";
    case Token::ORIGIN:
        return "origin";
    case Token::PURPOSE:
        return "purpose";
    case Token::TIMING_ALLOW_ORIGIN:
        return "timing-allow-origin";
    case Token::UPGRADE_INSECURE_REQUESTS:
        return "upgrade-insecure-requests";
    case Token::X_CONTENT_TYPE_OPTIONS:
        return "x-content-type-options";
    case Token::X_FORWARDED_FOR:
        return "x-forwarded-for";
    case Token::X_FRAME_OPTIONS:
        return "x-frame-options";
    case Token::X_XSS_PROTECTION:
        return "x-xss-protection";
    case Token::NONE:
        return "";
    }
}

} // namespace interfaces::io::types

#ifdef CONGELADO_TEST
namespace interfaces::io::types::tests {
using namespace boost::ut;

suite<"status_code"> status_code_suite = [] {
    "status_code unwraps the underlying numeric code"_test = [] {
        expect(status_code(Status::OK) == 200);
        expect(status_code(Status::NOT_FOUND) == 404);
        expect(status_code(Status::INTERNAL_SERVER_ERROR) == 500);
    };
};

suite<"method_str"> method_str_suite = [] {
    "method_str maps every known method to its wire name"_test = [] {
        expect(method_str(Method::GET) == "GET");
        expect(method_str(Method::HEAD) == "HEAD");
        expect(method_str(Method::POST) == "POST");
        expect(method_str(Method::PUT) == "PUT");
        expect(method_str(Method::DELETE) == "DELETE");
        expect(method_str(Method::PATCH) == "PATCH");
        expect(method_str(Method::CONNECT) == "CONNECT");
        expect(method_str(Method::OPTIONS) == "OPTIONS");
        expect(method_str(Method::TRACE) == "TRACE");
    };

    "method_str returns empty for UNKNOWN"_test = [] { expect(method_str(Method::UNKNOWN).empty()); };
};

suite<"parse_method"> parse_method_suite = [] {
    "parse_method recognizes every known verb"_test = [] {
        expect(parse_method("GET") == Method::GET);
        expect(parse_method("HEAD") == Method::HEAD);
        expect(parse_method("POST") == Method::POST);
        expect(parse_method("PUT") == Method::PUT);
        expect(parse_method("DELETE") == Method::DELETE);
        expect(parse_method("PATCH") == Method::PATCH);
        expect(parse_method("CONNECT") == Method::CONNECT);
        expect(parse_method("OPTIONS") == Method::OPTIONS);
        expect(parse_method("TRACE") == Method::TRACE);
    };

    "parse_method rejects empty, garbage and near-miss strings"_test = [] {
        expect(parse_method("") == Method::UNKNOWN);
        expect(parse_method("get") == Method::UNKNOWN);
        expect(parse_method("PUTT") == Method::UNKNOWN);
        expect(parse_method("POS") == Method::UNKNOWN);
    };

    "parse_method and method_str round-trip for every method"_test = [] {
        for (auto method : {Method::GET, Method::HEAD, Method::POST, Method::PUT, Method::DELETE,
                            Method::PATCH, Method::CONNECT, Method::OPTIONS, Method::TRACE}) {
            expect(parse_method(method_str(method)) == method);
        }
    };
};

suite<"tokenize"> tokenize_suite = [] {
    "tokenize resolves pseudo-headers and common field names"_test = [] {
        expect(tokenize(":authority") == Token::AUTHORITY);
        expect(tokenize(":method") == Token::METHOD);
        expect(tokenize(":path") == Token::PATH);
        expect(tokenize(":scheme") == Token::SCHEME);
        expect(tokenize(":status") == Token::STATUS);
        expect(tokenize("content-type") == Token::CONTENT_TYPE);
        expect(tokenize("content-length") == Token::CONTENT_LENGTH);
        expect(tokenize("cookie") == Token::COOKIE);
        expect(tokenize("set-cookie") == Token::SET_COOKIE);
        expect(tokenize("access-control-allow-credentials") ==
               Token::ACCESS_CONTROL_ALLOW_CREDENTIALS);
    };

    "tokenize returns nullopt for names it doesn't recognize, even at a known length"_test = [] {
        expect(tokenize("") == std::nullopt);
        expect(tokenize("not-a-real-header") == std::nullopt);
        // Same length bucket as "host" (4) but not a real match.
        expect(tokenize("zzzz") == std::nullopt);
    };

    "tokenize and token_to_string round-trip for every named token"_test = [] {
        for (auto token : {Token::AUTHORITY, Token::METHOD, Token::PATH, Token::SCHEME,
                           Token::STATUS, Token::CONTENT_TYPE, Token::CONTENT_LENGTH,
                           Token::COOKIE, Token::SET_COOKIE, Token::USER_AGENT, Token::HOST,
                           Token::VIA, Token::AGE}) {
            expect(tokenize(token_to_string(token)) == token);
        }
    };
};

suite<"token_to_string"> token_to_string_suite = [] {
    "token_to_string renders the wire-format header name"_test = [] {
        expect(token_to_string(Token::CONTENT_TYPE) == "content-type");
        expect(token_to_string(Token::PATH) == ":path");
        expect(token_to_string(Token::NONE).empty());
    };
};

} // namespace interfaces::io::types::tests
#endif
