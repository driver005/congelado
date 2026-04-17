export module io_shared:http_types;

import std;

export namespace io::shared::http {

// HTTP method as a typed enum — method_raw holds the original string for
// extension methods (PATCH, custom verbs, etc.) that don't map to the enum.
enum class HttpMethod { GET, HEAD, POST, PUT, DELETE, CONNECT, OPTIONS, TRACE, PATCH, UNKNOWN };

[[nodiscard]] inline HttpMethod parse_method(std::string_view s) noexcept {
    if (s == "GET")
        return HttpMethod::GET;
    if (s == "HEAD")
        return HttpMethod::HEAD;
    if (s == "POST")
        return HttpMethod::POST;
    if (s == "PUT")
        return HttpMethod::PUT;
    if (s == "DELETE")
        return HttpMethod::DELETE;
    if (s == "CONNECT")
        return HttpMethod::CONNECT;
    if (s == "OPTIONS")
        return HttpMethod::OPTIONS;
    if (s == "TRACE")
        return HttpMethod::TRACE;
    if (s == "PATCH")
        return HttpMethod::PATCH;
    return HttpMethod::UNKNOWN;
}

enum class Token : std::uint32_t {
    None = 0,
    Authority = 1,
    Method = 2,
    Path = 3,
    Scheme = 4,
    Status = 5,
    AcceptCharset = 6,
    AcceptEncoding = 7,
    AcceptLanguage = 8,
    AcceptRanges = 9,
    Accept = 10,
    AccessControlAllowOrigin = 11,
    Age = 12,
    Allow = 13,
    Authorization = 14,
    CacheControl = 15,
    ContentDisposition = 16,
    ContentEncoding = 17,
    ContentLanguage = 18,
    ContentLength = 19,
    ContentLocation = 20,
    ContentRange = 21,
    ContentType = 22,
    Cookie = 23,
    Date = 24,
    ETag = 25,
    Expect = 26,
    Expires = 27,
    From = 28,
    Host = 29,
    IfMatch = 30,
    IfModifiedSince = 31,
    IfNoneMatch = 32,
    IfRange = 33,
    IfUnmodifiedSince = 34,
    LastModified = 35,
    Link = 36,
    Location = 37,
    MaxForwards = 38,
    ProxyAuthenticate = 39,
    ProxyAuthorization = 40,
    Range = 41,
    Referer = 42,
    Refresh = 43,
    RetryAfter = 44,
    Server = 45,
    SetCookie = 46,
    StrictTransportSecurity = 47,
    TransferEncoding = 48,
    UserAgent = 49,
    Vary = 50,
    Via = 51,
    WwwAuthenticate = 52,
    AccessControlAllowCredentials = 53,
    AccessControlAllowHeaders = 54,
    AccessControlAllowMethods = 55,
    AccessControlExposeHeaders = 56,
    AccessControlRequestHeaders = 57,
    AccessControlRequestMethod = 58,
    AltSvc = 59,
    ContentSecurityPolicy = 60,
    EarlyData = 61,
    ExpectCt = 62,
    Forwarded = 63,
    Origin = 64,
    Purpose = 65,
    TimingAllowOrigin = 66,
    UpgradeInsecureRequests = 67,
    XContentTypeOptions = 68,
    XForwardedFor = 69,
    XFrameOptions = 70,
    XXssProtection = 71,
    Custom = 72
};

[[nodiscard]] constexpr Token tokenize(std::string_view name) noexcept {
    switch (name.length()) {
    case 3:
        if (name == "age")
            return Token::Age;
        if (name == "via")
            return Token::Via;
        break;
    case 4:
        if (name == "date")
            return Token::Date;
        if (name == "etag")
            return Token::ETag;
        if (name == "from")
            return Token::From;
        if (name == "host")
            return Token::Host;
        if (name == "link")
            return Token::Link;
        if (name == "vary")
            return Token::Vary;
        break;
    case 5:
        if (name == ":path")
            return Token::Path;
        if (name == "allow")
            return Token::Allow;
        if (name == "range")
            return Token::Range;
        break;
    case 6:
        if (name == "accept")
            return Token::Accept;
        if (name == "cookie")
            return Token::Cookie;
        if (name == "expect")
            return Token::Expect;
        if (name == "origin")
            return Token::Origin;
        if (name == "server")
            return Token::Server;
        break;
    case 7:
        if (name == ":method")
            return Token::Method;
        if (name == ":scheme")
            return Token::Scheme;
        if (name == ":status")
            return Token::Status;
        if (name == "alt-svc")
            return Token::AltSvc;
        if (name == "expires")
            return Token::Expires;
        if (name == "purpose")
            return Token::Purpose;
        if (name == "referer")
            return Token::Referer;
        if (name == "refresh")
            return Token::Refresh;
        break;
    case 8:
        if (name == "if-match")
            return Token::IfMatch;
        if (name == "if-range")
            return Token::IfRange;
        if (name == "location")
            return Token::Location;
        break;
    case 9:
        if (name == "expect-ct")
            return Token::ExpectCt;
        if (name == "forwarded")
            return Token::Forwarded;
        break;
    case 10:
        if (name == ":authority")
            return Token::Authority;
        if (name == "early-data")
            return Token::EarlyData;
        if (name == "set-cookie")
            return Token::SetCookie;
        if (name == "user-agent")
            return Token::UserAgent;
        break;
    case 11:
        if (name == "retry-after")
            return Token::RetryAfter;
        break;
    case 12:
        if (name == "accept-ranges")
            return Token::AcceptRanges;
        if (name == "content-type")
            return Token::ContentType;
        if (name == "max-forwards")
            return Token::MaxForwards;
        break;
    case 13:
        if (name == "authorization")
            return Token::Authorization;
        if (name == "cache-control")
            return Token::CacheControl;
        if (name == "content-range")
            return Token::ContentRange;
        if (name == "if-none-match")
            return Token::IfNoneMatch;
        if (name == "last-modified")
            return Token::LastModified;
        break;
    case 14:
        if (name == "accept-charset")
            return Token::AcceptCharset;
        if (name == "content-length")
            return Token::ContentLength;
        break;
    case 15:
        if (name == "accept-encoding")
            return Token::AcceptEncoding;
        if (name == "accept-language")
            return Token::AcceptLanguage;
        break;
    case 16:
        if (name == "content-encoding")
            return Token::ContentEncoding;
        if (name == "content-language")
            return Token::ContentLanguage;
        if (name == "content-location")
            return Token::ContentLocation;
        if (name == "www-authenticate")
            return Token::WwwAuthenticate;
        break;
    case 17:
        if (name == "if-modified-since")
            return Token::IfModifiedSince;
        if (name == "transfer-encoding")
            return Token::TransferEncoding;
        break;
    case 18:
        if (name == "proxy-authenticate")
            return Token::ProxyAuthenticate;
        if (name == "x-xss-protection")
            return Token::XXssProtection;
        break;
    case 19:
        if (name == "content-disposition")
            return Token::ContentDisposition;
        if (name == "if-unmodified-since")
            return Token::IfUnmodifiedSince;
        if (name == "proxy-authorization")
            return Token::ProxyAuthorization;
        if (name == "timing-allow-origin")
            return Token::TimingAllowOrigin;
        if (name == "x-frame-options")
            return Token::XFrameOptions;
        break;
    case 20:
        if (name == "x-forwarded-for")
            return Token::XForwardedFor;
        break;
    case 22:
        if (name == "x-content-type-options")
            return Token::XContentTypeOptions;
        break;
    case 23:
        if (name == "content-security-policy")
            return Token::ContentSecurityPolicy;
        break;
    case 25:
        if (name == "strict-transport-security")
            return Token::StrictTransportSecurity;
        if (name == "upgrade-insecure-requests")
            return Token::UpgradeInsecureRequests;
        break;
    case 27:
        if (name == "access-control-allow-origin")
            return Token::AccessControlAllowOrigin;
        break;
    case 28:
        if (name == "access-control-allow-headers")
            return Token::AccessControlAllowHeaders;
        if (name == "access-control-allow-methods")
            return Token::AccessControlAllowMethods;
        break;
    case 29:
        if (name == "access-control-expose-headers")
            return Token::AccessControlExposeHeaders;
        if (name == "access-control-request-method")
            return Token::AccessControlRequestMethod;
        break;
    case 30:
        if (name == "access-control-request-headers")
            return Token::AccessControlRequestHeaders;
        break;
    case 32:
        if (name == "access-control-allow-credentials")
            return Token::AccessControlAllowCredentials;
        break;
    }
    return Token::Custom;
}

constexpr std::string_view token_to_string(const Token &tk) noexcept {
    switch (tk) {
    case Token::Authority:
        return ":authority";
    case Token::Method:
        return ":method";
    case Token::Path:
        return ":path";
    case Token::Scheme:
        return ":scheme";
    case Token::Status:
        return ":status";
    case Token::AcceptCharset:
        return "accept-charset";
    case Token::AcceptEncoding:
        return "accept-encoding";
    case Token::AcceptLanguage:
        return "accept-language";
    case Token::AcceptRanges:
        return "accept-ranges";
    case Token::Accept:
        return "accept";
    case Token::AccessControlAllowOrigin:
        return "access-control-allow-origin";
    case Token::Age:
        return "age";
    case Token::Allow:
        return "allow";
    case Token::Authorization:
        return "authorization";
    case Token::CacheControl:
        return "cache-control";
    case Token::ContentDisposition:
        return "content-disposition";
    case Token::ContentEncoding:
        return "content-encoding";
    case Token::ContentLanguage:
        return "content-language";
    case Token::ContentLength:
        return "content-length";
    case Token::ContentLocation:
        return "content-location";
    case Token::ContentRange:
        return "content-range";
    case Token::ContentType:
        return "content-type";
    case Token::Cookie:
        return "cookie";
    case Token::Date:
        return "date";
    case Token::ETag:
        return "etag";
    case Token::Expect:
        return "expect";
    case Token::Expires:
        return "expires";
    case Token::From:
        return "from";
    case Token::Host:
        return "host";
    case Token::IfMatch:
        return "if-match";
    case Token::IfModifiedSince:
        return "if-modified-since";
    case Token::IfNoneMatch:
        return "if-none-match";
    case Token::IfRange:
        return "if-range";
    case Token::IfUnmodifiedSince:
        return "if-unmodified-since";
    case Token::LastModified:
        return "last-modified";
    case Token::Link:
        return "link";
    case Token::Location:
        return "location";
    case Token::MaxForwards:
        return "max-forwards";
    case Token::ProxyAuthenticate:
        return "proxy-authenticate";
    case Token::ProxyAuthorization:
        return "proxy-authorization";
    case Token::Range:
        return "range";
    case Token::Referer:
        return "referer";
    case Token::Refresh:
        return "refresh";
    case Token::RetryAfter:
        return "retry-after";
    case Token::Server:
        return "server";
    case Token::SetCookie:
        return "set-cookie";
    case Token::StrictTransportSecurity:
        return "strict-transport-security";
    case Token::TransferEncoding:
        return "transfer-encoding";
    case Token::UserAgent:
        return "user-agent";
    case Token::Vary:
        return "vary";
    case Token::Via:
        return "via";
    case Token::WwwAuthenticate:
        return "www-authenticate";
    case Token::AccessControlAllowCredentials:
        return "access-control-allow-credentials";
    case Token::AccessControlAllowHeaders:
        return "access-control-allow-headers";
    case Token::AccessControlAllowMethods:
        return "access-control-allow-methods";
    case Token::AccessControlExposeHeaders:
        return "access-control-expose-headers";
    case Token::AccessControlRequestHeaders:
        return "access-control-request-headers";
    case Token::AccessControlRequestMethod:
        return "access-control-request-method";
    case Token::AltSvc:
        return "alt-svc";
    case Token::ContentSecurityPolicy:
        return "content-security-policy";
    case Token::EarlyData:
        return "early-data";
    case Token::ExpectCt:
        return "expect-ct";
    case Token::Forwarded:
        return "forwarded";
    case Token::Origin:
        return "origin";
    case Token::Purpose:
        return "purpose";
    case Token::TimingAllowOrigin:
        return "timing-allow-origin";
    case Token::UpgradeInsecureRequests:
        return "upgrade-insecure-requests";
    case Token::XContentTypeOptions:
        return "x-content-type-options";
    case Token::XForwardedFor:
        return "x-forwarded-for";
    case Token::XFrameOptions:
        return "x-frame-options";
    case Token::XXssProtection:
        return "x-xss-protection";
    case Token::None:
    case Token::Custom:
        return "";
    }
}

} // namespace io::shared::http
