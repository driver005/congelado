export module core_otel:http;

import std;
import interfaces;
import :span;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

namespace core::otel::detail {

/**
 * @brief Encodes one nibble (0-15) as its lowercase hex digit character.
 */
inline char to_hex_digit(std::uint8_t nibble) noexcept {
    return nibble < 10 ? static_cast<char>('0' + nibble) : static_cast<char>('a' + (nibble - 10));
}

/**
 * @brief Hex-encodes a fixed-size byte array, lowercase, no separators — the exact shape a W3C
 * `traceparent` field expects for its trace-id/span-id/flags segments.
 */
template <std::size_t N>
std::string to_hex(const std::array<std::byte, N> &bytes) {
    std::string out;
    out.reserve(N * 2);
    for (auto byte : bytes) {
        auto value = std::to_integer<std::uint8_t>(byte);
        out.push_back(to_hex_digit(static_cast<std::uint8_t>(value >> 4)));
        out.push_back(to_hex_digit(static_cast<std::uint8_t>(value & 0x0FU)));
    }
    return out;
}

/**
 * @brief Decodes one hex digit character, case-insensitive.
 * @return the digit's value (0-15), or `std::nullopt` if `c` isn't a valid hex digit.
 */
inline std::optional<std::uint8_t> from_hex_digit(char c) noexcept {
    if (c >= '0' && c <= '9') {
        return static_cast<std::uint8_t>(c - '0');
    }
    if (c >= 'a' && c <= 'f') {
        return static_cast<std::uint8_t>(c - 'a' + 10);
    }
    if (c >= 'A' && c <= 'F') {
        return static_cast<std::uint8_t>(c - 'A' + 10);
    }
    return std::nullopt;
}

/**
 * @brief Decodes a hex string into a fixed-size byte array.
 * @return the decoded bytes, or `std::nullopt` if `hex` isn't exactly `N * 2` valid hex digits.
 */
template <std::size_t N>
std::optional<std::array<std::byte, N>> from_hex(std::string_view hex) {
    if (hex.size() != N * 2) {
        return std::nullopt;
    }
    std::array<std::byte, N> out{};
    for (std::size_t i = 0; i < N; ++i) {
        auto hi = from_hex_digit(hex[i * 2]);
        auto lo = from_hex_digit(hex[(i * 2) + 1]);
        if (!hi.has_value() || !lo.has_value()) {
            return std::nullopt;
        }
        out[i] = static_cast<std::byte>(static_cast<std::uint8_t>((*hi << 4) | *lo));
    }
    return out;
}

} // namespace core::otel::detail

export namespace core::otel {

/**
 * @brief Formats a `SpanContext` as a W3C `traceparent` header value
 * (`00-{trace_id}-{span_id}-{flags}`), for injecting into an outbound request before it's sent.
 * @param ctx the context to format.
 * @return the formatted header value.
 */
inline std::string format_traceparent(const interfaces::SpanContext &ctx) {
    return std::format("00-{}-{}-{:02x}", detail::to_hex(ctx.trace_id), detail::to_hex(ctx.span_id),
                       ctx.sampled ? 1 : 0);
}

/**
 * @brief Parses a W3C `traceparent` header value into a `SpanContext`.
 * @note The parsed span id becomes the *parent* span id for whatever child span gets started
 * from the returned context (via `start_span(name, kind, parent)`) — it is not, itself, this
 * context's own span id; there is no "own span id" yet until a new span is actually started.
 * @param header the raw header value (e.g. from `IRequest::find_header("traceparent")`).
 * @return the parsed context, or `std::nullopt` if `header` isn't a valid `traceparent` value.
 */
inline std::optional<interfaces::SpanContext> parse_traceparent(std::string_view header) {
    // version(2) '-' trace-id(32) '-' span-id(16) '-' flags(2) == 55 chars for version "00".
    constexpr std::size_t EXPECTED_LEN = 55;
    if (header.size() < EXPECTED_LEN || header.substr(0, 2) != "00" || header[2] != '-' ||
        header[35] != '-' || header[52] != '-') {
        return std::nullopt;
    }
    auto trace_id = detail::from_hex<16>(header.substr(3, 32));
    auto span_id = detail::from_hex<8>(header.substr(36, 16));
    auto flags = detail::from_hex<1>(header.substr(53, 2));
    if (!trace_id.has_value() || !span_id.has_value() || !flags.has_value()) {
        return std::nullopt;
    }
    interfaces::SpanContext ctx;
    ctx.trace_id = *trace_id;
    ctx.span_id = *span_id;
    ctx.sampled = (std::to_integer<std::uint8_t>((*flags)[0]) & 0x1U) != 0;
    return ctx;
}

// Automatic per-request SERVER span creation lives in `core::router::RouteHandler::match()`
// (the actual dispatch choke point every registered route funnels through) rather than as a
// wrapper function here — same "just works, no per-route boilerplate" deal as OpenAPI metadata
// capture, just at dispatch time instead of registration time. `format_traceparent`/
// `parse_traceparent` above are the pieces that dispatch logic actually needs from this file.
//
// @note Span status there is judged by whether the handler threw, not by the response's actual
// status code: `interfaces::io::IResponse::get_status()`/`is_success()` both default to
// `std::abort()` in the base interface, and the concrete HTTP/2 response type
// (`include/io/layer/http2/res.cppm`, off-limits to modify here) only overrides the setter
// (`set_status()`), never either getter — reading a response's status back out isn't
// implemented anywhere in this codebase yet. Calling either getter would abort the process on
// every single request, so dispatch deliberately doesn't — a real gap, but one to close in the
// http2 response layer itself, not by working around it here.

} // namespace core::otel

#ifdef CONGELADO_TEST
namespace core::otel::tests {
using namespace boost::ut;

suite<"otel::traceparent"> traceparent_suite = [] {
    "an all-zero unsampled context formats to the expected traceparent string"_test = [] {
        interfaces::SpanContext ctx;
        ctx.sampled = false;

        auto header = format_traceparent(ctx);

        expect(header == "00-00000000000000000000000000000000-0000000000000000-00");
    };

    "a sampled context sets the trailing flags byte to 01"_test = [] {
        interfaces::SpanContext ctx;
        ctx.sampled = true;

        auto header = format_traceparent(ctx);

        expect(header.ends_with("-01"));
    };

    "format then parse round-trips trace id, span id, and sampled flag"_test = [] {
        interfaces::SpanContext ctx;
        for (std::size_t i = 0; i < ctx.trace_id.size(); ++i) {
            ctx.trace_id[i] = static_cast<std::byte>(i + 1);
        }
        for (std::size_t i = 0; i < ctx.span_id.size(); ++i) {
            ctx.span_id[i] = static_cast<std::byte>(0xA0 + i);
        }
        ctx.sampled = true;

        auto parsed = parse_traceparent(format_traceparent(ctx));

        expect(parsed.has_value());
        expect(std::ranges::equal(parsed->trace_id, ctx.trace_id));
        expect(std::ranges::equal(parsed->span_id, ctx.span_id));
        expect(parsed->sampled == ctx.sampled);
    };

    "parse rejects a header that's too short"_test = [] {
        expect(not parse_traceparent("00-abcd").has_value());
    };

    "parse rejects an unsupported version prefix"_test = [] {
        expect(not parse_traceparent(
                       "01-00000000000000000000000000000000-0000000000000000-00")
                       .has_value());
    };

    "parse rejects a header with misplaced separators"_test = [] {
        expect(not parse_traceparent(
                       "00x00000000000000000000000000000000-0000000000000000-00")
                       .has_value());
    };

    "parse rejects invalid hex digits"_test = [] {
        expect(not parse_traceparent(
                       "00-zz000000000000000000000000000000-0000000000000000-00")
                       .has_value());
    };
};

} // namespace core::otel::tests
#endif
