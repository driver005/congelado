export module io_shared:http_types;

import std;

export namespace io::shared::http {

// // HTTP method as a typed enum — method_raw holds the original string for
// // extension methods (PATCH, custom verbs, etc.) that don't map to the enum.
// enum class HttpMethod : std::uint8_t { GET, HEAD, POST, PUT, DELETE, CONNECT, OPTIONS, TRACE,
// PATCH, UNKNOWN };
//
// [[nodiscard]] constexpr std::string_view method_str(HttpMethod method) noexcept {
//     switch (method) {
//     case HttpMethod::GET:
//         return "GET";
//     case HttpMethod::HEAD:
//         return "HEAD";
//     case HttpMethod::POST:
//         return "POST";
//     case HttpMethod::PUT:
//         return "PUT";
//     case HttpMethod::DELETE:
//         return "DELETE";
//     case HttpMethod::CONNECT:
//         return "CONNECT";
//     case HttpMethod::OPTIONS:
//         return "OPTIONS";
//     case HttpMethod::TRACE:
//         return "TRACE";
//     case HttpMethod::PATCH:
//         return "PATCH";
//     default:
//         return "";
//     }
// }
//
// [[nodiscard]] inline HttpMethod parse_method(std::string_view view) noexcept {
//     if (view.empty()) [[unlikely]] {
//         return HttpMethod::UNKNOWN;
//     }
//
//     switch (view[0]) {
//     case 'G':
//         if (view == "GET") {
//             return HttpMethod::GET;
//         }
//         break;
//     case 'H':
//         if (view == "HEAD") {
//             return HttpMethod::HEAD;
//         }
//         break;
//     case 'P':
//         switch (view.size()) {
//         case 4:
//             if (view == "POST") {
//                 return HttpMethod::POST;
//             }
//             break;
//         case 3:
//             if (view == "PUT") {
//                 return HttpMethod::PUT;
//             }
//             break;
//         case 5:
//             if (view == "PATCH") {
//                 return HttpMethod::PATCH;
//             }
//             break;
//         default:
//             break;
//         }
//         break;
//     case 'D':
//         if (view == "DELETE") {
//             return HttpMethod::DELETE;
//         }
//         break;
//     case 'C':
//         if (view == "CONNECT") {
//             return HttpMethod::CONNECT;
//         }
//         break;
//     case 'O':
//         if (view == "OPTIONS") {
//             return HttpMethod::OPTIONS;
//         }
//         break;
//     case 'T':
//         if (view == "TRACE") {
//             return HttpMethod::TRACE;
//         }
//         break;
//     default:
//         break;
//     }
//
//     return HttpMethod::UNKNOWN;
// }

// enum class Token : std::uint32_t {
//     NONE = 0,
//     AUTHORITY = 1,
//     METHOD = 2,
//     PATH = 3,
//     SCHEME = 4,
//     STATUS = 5,
//     ACCEPT_CHARSET = 6,
//     ACCEPT_ENCODING = 7,
//     ACCEPT_LANGUAGE = 8,
//     ACCEPT_RANGES = 9,
//     ACCEPT = 10,
//     ACCESS_CONTROL_ALLOW_ORIGIN = 11,
//     AGE = 12,
//     ALLOW = 13,
//     AUTHORIZATION = 14,
//     CACHE_CONTROL = 15,
//     CONTENT_DISPOSITION = 16,
//     CONTENT_ENCODING = 17,
//     CONTENT_LANGUAGE = 18,
//     CONTENT_LENGTH = 19,
//     CONTENT_LOCATION = 20,
//     CONTENT_RANGE = 21,
//     CONTENT_TYPE = 22,
//     COOKIE = 23,
//     DATE = 24,
//     E_TAG = 25,
//     EXPECT = 26,
//     EXPIRES = 27,
//     FROM = 28,
//     HOST = 29,
//     IF_MATCH = 30,
//     IF_MODIFIED_SINCE = 31,
//     IF_NONE_MATCH = 32,
//     IF_RANGE = 33,
//     IF_UNMODIFIED_SINCE = 34,
//     LAST_MODIFIED = 35,
//     LINK = 36,
//     LOCATION = 37,
//     MAX_FORWARDS = 38,
//     PROXY_AUTHENTICATE = 39,
//     PROXY_AUTHORIZATION = 40,
//     RANGE = 41,
//     REFERER = 42,
//     REFRESH = 43,
//     RETRY_AFTER = 44,
//     SERVER = 45,
//     SET_COOKIE = 46,
//     STRICT_TRANSPORT_SECURITY = 47,
//     TRANSFER_ENCODING = 48,
//     USER_AGENT = 49,
//     VARY = 50,
//     VIA = 51,
//     WWW_AUTHENTICATE = 52,
//     ACCESS_CONTROL_ALLOW_CREDENTIALS = 53,
//     ACCESS_CONTROL_ALLOW_HEADERS = 54,
//     ACCESS_CONTROL_ALLOW_METHODS = 55,
//     ACCESS_CONTROL_EXPOSE_HEADERS = 56,
//     ACCESS_CONTROL_REQUEST_HEADERS = 57,
//     ACCESS_CONTROL_REQUEST_METHOD = 58,
//     ALT_SVC = 59,
//     CONTENT_SECURITY_POLICY = 60,
//     EARLY_DATA = 61,
//     EXPECT_CT = 62,
//     FORWARDED = 63,
//     ORIGIN = 64,
//     PURPOSE = 65,
//     TIMING_ALLOW_ORIGIN = 66,
//     UPGRADE_INSECURE_REQUESTS = 67,
//     X_CONTENT_TYPE_OPTIONS = 68,
//     X_FORWARDED_FOR = 69,
//     X_FRAME_OPTIONS = 70,
//     X_XSS_PROTECTION = 71,
//     CUSTOM = 72
// };
//
// [[nodiscard]] constexpr Token tokenize(std::string_view name) noexcept {
//     switch (name.length()) {
//     case 3:
//         if (name == "age") {
//             return Token::AGE;
//         }
//         if (name == "via") {
//             return Token::VIA;
//         }
//         break;
//     case 4:
//         if (name == "date") {
//             return Token::DATE;
//         }
//         if (name == "etag") {
//             return Token::E_TAG;
//         }
//         if (name == "from") {
//             return Token::FROM;
//         }
//         if (name == "host") {
//             return Token::HOST;
//         }
//         if (name == "link") {
//             return Token::LINK;
//         }
//         if (name == "vary") {
//             return Token::VARY;
//         }
//         break;
//     case 5:
//         if (name == ":path") {
//             return Token::PATH;
//         }
//         if (name == "allow") {
//             return Token::ALLOW;
//         }
//         if (name == "range") {
//             return Token::RANGE;
//         }
//         break;
//     case 6:
//         if (name == "accept") {
//             return Token::ACCEPT;
//         }
//         if (name == "cookie") {
//             return Token::COOKIE;
//         }
//         if (name == "expect") {
//             return Token::EXPECT;
//         }
//         if (name == "origin") {
//             return Token::ORIGIN;
//         }
//         if (name == "server") {
//             return Token::SERVER;
//         }
//         break;
//     case 7:
//         if (name == ":method") {
//             return Token::METHOD;
//         }
//         if (name == ":scheme") {
//             return Token::SCHEME;
//         }
//         if (name == ":status") {
//             return Token::STATUS;
//         }
//         if (name == "alt-svc") {
//             return Token::ALT_SVC;
//         }
//         if (name == "expires") {
//             return Token::EXPIRES;
//         }
//         if (name == "purpose") {
//             return Token::PURPOSE;
//         }
//         if (name == "referer") {
//             return Token::REFERER;
//         }
//         if (name == "refresh") {
//             return Token::REFRESH;
//         }
//         break;
//     case 8:
//         if (name == "if-match") {
//             return Token::IF_MATCH;
//         }
//         if (name == "if-range") {
//             return Token::IF_RANGE;
//         }
//         if (name == "location") {
//             return Token::LOCATION;
//         }
//         break;
//     case 9:
//         if (name == "expect-ct") {
//             return Token::EXPECT_CT;
//         }
//         if (name == "forwarded") {
//             return Token::FORWARDED;
//         }
//         break;
//     case 10:
//         if (name == ":authority") {
//             return Token::AUTHORITY;
//         }
//         if (name == "early-data") {
//             return Token::EARLY_DATA;
//         }
//         if (name == "set-cookie") {
//             return Token::SET_COOKIE;
//         }
//         if (name == "user-agent") {
//             return Token::USER_AGENT;
//         }
//         break;
//     case 11:
//         if (name == "retry-after") {
//             return Token::RETRY_AFTER;
//         }
//         break;
//     case 12:
//         if (name == "accept-ranges") {
//             return Token::ACCEPT_RANGES;
//         }
//         if (name == "content-type") {
//             return Token::CONTENT_TYPE;
//         }
//         if (name == "max-forwards") {
//             return Token::MAX_FORWARDS;
//         }
//         break;
//     case 13:
//         if (name == "authorization") {
//             return Token::AUTHORIZATION;
//         }
//         if (name == "cache-control") {
//             return Token::CACHE_CONTROL;
//         }
//         if (name == "content-range") {
//             return Token::CONTENT_RANGE;
//         }
//         if (name == "if-none-match") {
//             return Token::IF_NONE_MATCH;
//         }
//         if (name == "last-modified") {
//             return Token::LAST_MODIFIED;
//         }
//         break;
//     case 14:
//         if (name == "accept-charset") {
//             return Token::ACCEPT_CHARSET;
//         }
//         if (name == "content-length") {
//             return Token::CONTENT_LENGTH;
//         }
//         break;
//     case 15:
//         if (name == "accept-encoding") {
//             return Token::ACCEPT_ENCODING;
//         }
//         if (name == "accept-language") {
//             return Token::ACCEPT_LANGUAGE;
//         }
//         break;
//     case 16:
//         if (name == "content-encoding") {
//             return Token::CONTENT_ENCODING;
//         }
//         if (name == "content-language") {
//             return Token::CONTENT_LANGUAGE;
//         }
//         if (name == "content-location") {
//             return Token::CONTENT_LOCATION;
//         }
//         if (name == "www-authenticate") {
//             return Token::WWW_AUTHENTICATE;
//         }
//         break;
//     case 17:
//         if (name == "if-modified-since") {
//             return Token::IF_MODIFIED_SINCE;
//         }
//         if (name == "transfer-encoding") {
//             return Token::TRANSFER_ENCODING;
//         }
//         break;
//     case 18:
//         if (name == "proxy-authenticate") {
//             return Token::PROXY_AUTHENTICATE;
//         }
//         if (name == "x-xss-protection") {
//             return Token::X_XSS_PROTECTION;
//         }
//         break;
//     case 19:
//         if (name == "content-disposition") {
//             return Token::CONTENT_DISPOSITION;
//         }
//         if (name == "if-unmodified-since") {
//             return Token::IF_UNMODIFIED_SINCE;
//         }
//         if (name == "proxy-authorization") {
//             return Token::PROXY_AUTHORIZATION;
//         }
//         if (name == "timing-allow-origin") {
//             return Token::TIMING_ALLOW_ORIGIN;
//         }
//         if (name == "x-frame-options") {
//             return Token::X_FRAME_OPTIONS;
//         }
//         break;
//     case 20:
//         if (name == "x-forwarded-for") {
//             return Token::X_FORWARDED_FOR;
//         }
//         break;
//     case 22:
//         if (name == "x-content-type-options") {
//             return Token::X_CONTENT_TYPE_OPTIONS;
//         }
//         break;
//     case 23:
//         if (name == "content-security-policy") {
//             return Token::CONTENT_SECURITY_POLICY;
//         }
//         break;
//     case 25:
//         if (name == "strict-transport-security") {
//             return Token::STRICT_TRANSPORT_SECURITY;
//         }
//         if (name == "upgrade-insecure-requests") {
//             return Token::UPGRADE_INSECURE_REQUESTS;
//         }
//         break;
//     case 27:
//         if (name == "access-control-allow-origin") {
//             return Token::ACCESS_CONTROL_ALLOW_ORIGIN;
//         }
//         break;
//     case 28:
//         if (name == "access-control-allow-headers") {
//             return Token::ACCESS_CONTROL_ALLOW_HEADERS;
//         }
//         if (name == "access-control-allow-methods") {
//             return Token::ACCESS_CONTROL_ALLOW_METHODS;
//         }
//         break;
//     case 29:
//         if (name == "access-control-expose-headers") {
//             return Token::ACCESS_CONTROL_EXPOSE_HEADERS;
//         }
//         if (name == "access-control-request-method") {
//             return Token::ACCESS_CONTROL_REQUEST_METHOD;
//         }
//         break;
//     case 30:
//         if (name == "access-control-request-headers") {
//             return Token::ACCESS_CONTROL_REQUEST_HEADERS;
//         }
//         break;
//     case 32:
//         if (name == "access-control-allow-credentials") {
//             return Token::ACCESS_CONTROL_ALLOW_CREDENTIALS;
//         }
//         break;
//     default:
//         break;
//     }
//     return Token::CUSTOM;
// }
//
// constexpr std::string_view token_to_string(const Token &tkst) noexcept {
//     switch (tkst) {
//     case Token::AUTHORITY:
//         return ":authority";
//     case Token::METHOD:
//         return ":method";
//     case Token::PATH:
//         return ":path";
//     case Token::SCHEME:
//         return ":scheme";
//     case Token::STATUS:
//         return ":status";
//     case Token::ACCEPT_CHARSET:
//         return "accept-charset";
//     case Token::ACCEPT_ENCODING:
//         return "accept-encoding";
//     case Token::ACCEPT_LANGUAGE:
//         return "accept-language";
//     case Token::ACCEPT_RANGES:
//         return "accept-ranges";
//     case Token::ACCEPT:
//         return "accept";
//     case Token::ACCESS_CONTROL_ALLOW_ORIGIN:
//         return "access-control-allow-origin";
//     case Token::AGE:
//         return "age";
//     case Token::ALLOW:
//         return "allow";
//     case Token::AUTHORIZATION:
//         return "authorization";
//     case Token::CACHE_CONTROL:
//         return "cache-control";
//     case Token::CONTENT_DISPOSITION:
//         return "content-disposition";
//     case Token::CONTENT_ENCODING:
//         return "content-encoding";
//     case Token::CONTENT_LANGUAGE:
//         return "content-language";
//     case Token::CONTENT_LENGTH:
//         return "content-length";
//     case Token::CONTENT_LOCATION:
//         return "content-location";
//     case Token::CONTENT_RANGE:
//         return "content-range";
//     case Token::CONTENT_TYPE:
//         return "content-type";
//     case Token::COOKIE:
//         return "cookie";
//     case Token::DATE:
//         return "date";
//     case Token::E_TAG:
//         return "etag";
//     case Token::EXPECT:
//         return "expect";
//     case Token::EXPIRES:
//         return "expires";
//     case Token::FROM:
//         return "from";
//     case Token::HOST:
//         return "host";
//     case Token::IF_MATCH:
//         return "if-match";
//     case Token::IF_MODIFIED_SINCE:
//         return "if-modified-since";
//     case Token::IF_NONE_MATCH:
//         return "if-none-match";
//     case Token::IF_RANGE:
//         return "if-range";
//     case Token::IF_UNMODIFIED_SINCE:
//         return "if-unmodified-since";
//     case Token::LAST_MODIFIED:
//         return "last-modified";
//     case Token::LINK:
//         return "link";
//     case Token::LOCATION:
//         return "location";
//     case Token::MAX_FORWARDS:
//         return "max-forwards";
//     case Token::PROXY_AUTHENTICATE:
//         return "proxy-authenticate";
//     case Token::PROXY_AUTHORIZATION:
//         return "proxy-authorization";
//     case Token::RANGE:
//         return "range";
//     case Token::REFERER:
//         return "referer";
//     case Token::REFRESH:
//         return "refresh";
//     case Token::RETRY_AFTER:
//         return "retry-after";
//     case Token::SERVER:
//         return "server";
//     case Token::SET_COOKIE:
//         return "set-cookie";
//     case Token::STRICT_TRANSPORT_SECURITY:
//         return "strict-transport-security";
//     case Token::TRANSFER_ENCODING:
//         return "transfer-encoding";
//     case Token::USER_AGENT:
//         return "user-agent";
//     case Token::VARY:
//         return "vary";
//     case Token::VIA:
//         return "via";
//     case Token::WWW_AUTHENTICATE:
//         return "www-authenticate";
//     case Token::ACCESS_CONTROL_ALLOW_CREDENTIALS:
//         return "access-control-allow-credentials";
//     case Token::ACCESS_CONTROL_ALLOW_HEADERS:
//         return "access-control-allow-headers";
//     case Token::ACCESS_CONTROL_ALLOW_METHODS:
//         return "access-control-allow-methods";
//     case Token::ACCESS_CONTROL_EXPOSE_HEADERS:
//         return "access-control-expose-headers";
//     case Token::ACCESS_CONTROL_REQUEST_HEADERS:
//         return "access-control-request-headers";
//     case Token::ACCESS_CONTROL_REQUEST_METHOD:
//         return "access-control-request-method";
//     case Token::ALT_SVC:
//         return "alt-svc";
//     case Token::CONTENT_SECURITY_POLICY:
//         return "content-security-policy";
//     case Token::EARLY_DATA:
//         return "early-data";
//     case Token::EXPECT_CT:
//         return "expect-ct";
//     case Token::FORWARDED:
//         return "forwarded";
//     case Token::ORIGIN:
//         return "origin";
//     case Token::PURPOSE:
//         return "purpose";
//     case Token::TIMING_ALLOW_ORIGIN:
//         return "timing-allow-origin";
//     case Token::UPGRADE_INSECURE_REQUESTS:
//         return "upgrade-insecure-requests";
//     case Token::X_CONTENT_TYPE_OPTIONS:
//         return "x-content-type-options";
//     case Token::X_FORWARDED_FOR:
//         return "x-forwarded-for";
//     case Token::X_FRAME_OPTIONS:
//         return "x-frame-options";
//     case Token::X_XSS_PROTECTION:
//         return "x-xss-protection";
//     case Token::NONE:
//     case Token::CUSTOM:
//         return "";
//     }
// }

} // namespace io::shared::http
