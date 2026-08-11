module;
#include <rfl/from_generic.hpp>
#include <rfl/json.hpp>
#include <rfl/to_generic.hpp>

export module serde;

export import :core;
export import :converter;
export import :cache;

import interfaces;

export namespace serde {

/// @brief The dynamic reflected-value type every format plugin's encode/decode crosses the
/// ABI as — everything outside `serde` (and the two format plugins, which implement the ABI
/// contract itself) should reach `rfl::Generic` through this alias rather than including any
/// `<rfl/...>` header directly; `serde` is the only place that's supposed to know rfl exists.
using Value = rfl::Generic;

/**
 * @brief Holds every registered wire-format plugin (JSON, TOML, ...) for one process, keyed by
 * content-type — mirrors `core::logger::LoggerRegistry` exactly. Instance-owned (not a static
 * singleton) — exactly one lives inside `congelado::heart::AppContext` (the engine) or a local
 * variable in the worker's `main()`, and `set_active()` points the ambient `Ser::serialize`/
 * `deserialize` facade (dozens of call sites across engine handlers, deliberately kept
 * untouched) at it. Only `s_active` — a single pointer, not the format data itself — is
 * process-global. A format only becomes usable once its plugin actually registers here;
 * nothing is compiled in by default.
 */
class SerdeFormatRegistry {
  public:
    /**
     * @brief Registers a loaded format plugin. No-op if `format` is null.
     * @param format the format instance to add.
     */
    void add_format(std::shared_ptr<interfaces::ISerdeFormat> format) {
        if (format) {
            m_formats.push_back(std::move(format));
        }
    }

    /**
     * @brief Looks up a registered format by content-type.
     * @param content_type the wire content-type to match (e.g. `"application/json"`).
     * @return the matching format, or `nullptr` if no plugin registered for it — i.e. the
     * format plugin simply isn't loaded.
     */
    [[nodiscard]] interfaces::ISerdeFormat *find(std::string_view content_type) const noexcept {
        for (auto &format : m_formats) {
            if (format->content_type() == content_type) {
                return format.get();
            }
        }
        return nullptr;
    }

    /// @brief Gets every registered format. @return all registered formats, in registration order.
    [[nodiscard]] const std::vector<std::shared_ptr<interfaces::ISerdeFormat>> &
    get_formats() const noexcept {
        return m_formats;
    }

    /**
     * @brief Points the ambient `Ser` facade at this instance — call once, right after
     * constructing the process's one `SerdeFormatRegistry`, before any `Ser::serialize`/
     * `deserialize` call.
     * @param registry the instance to make active, or `nullptr` to clear it.
     */
    static void set_active(SerdeFormatRegistry *registry) noexcept { s_active = registry; }

    /**
     * @brief Gets the currently active registry, if one was set.
     * @return the active `SerdeFormatRegistry`, or `nullptr` if `set_active()` was never called.
     */
    [[nodiscard]] static SerdeFormatRegistry *get_active() noexcept { return s_active; }

  private:
    std::vector<std::shared_ptr<interfaces::ISerdeFormat>> m_formats;
    static inline SerdeFormatRegistry *s_active{nullptr};
};

// ─── SerBase / Ser ────────────────────────────────────────────────────────────
//
// Runtime wire-format dispatch. Reads the Accept / Content-Type header value and dispatches
// to whatever format plugin is registered in SerdeFormatRegistry for it — no plugin loaded
// for that content-type means the format genuinely isn't available, no silent fallback.
//
// To add a new wire format: ship a plugin implementing interfaces::ISerdeFormat; nothing to
// change here.

class Ser {
    /**
     * @brief Converts a plain string into a byte vector — the wire types this class hands
     * back everywhere, since that's what the transport layer actually wants.
     * @param str the string to convert.
     * @return `str`'s bytes, one std::byte per char, same order.
     */
    [[nodiscard]] static std::vector<std::byte> to_bytes(std::string_view str) {
        std::vector<std::byte> bytes(str.size());
        std::ranges::transform(str, bytes.begin(),
                               [](char character) noexcept { return std::byte(character); });
        return bytes;
    }

  public:
    /**
     * @brief Serializes a single value, dispatching to whichever format plugin is registered
     * in `SerdeFormatRegistry` for `accept`.
     * @note Any `T` works here, not just `ISerializable` ones — `rfl::to_generic` reduces it
     * to an `rfl::Generic` first (using the `Reflector<T>` specialization `converter.cppm`
     * already provides for `ISerializable` types, or rfl's own native reflection otherwise),
     * and only that generic value crosses into the plugin's `encode`.
     * @tparam T the type being encoded.
     * @param accept the requested content-type (e.g. an Accept header value) to match
     * against a registered format's `content_type`.
     * @param value the instance to serialize.
     * @warning No silent JSON fallback — if no format plugin is registered for `accept`, this
     * returns an error payload instead of guessing. That's the whole point: a format that
     * isn't loaded isn't available.
     * @return the encoded bytes, or an error payload if no format is registered for `accept`
     * or the registered format's `encode` itself fails.
     */
    template <typename T>
    [[nodiscard]] static std::vector<std::byte> serialize(std::string_view accept, const T &value) {
        auto *registry = SerdeFormatRegistry::get_active();
        auto *format = registry != nullptr ? registry->find(accept) : nullptr;
        if (format == nullptr) {
            return serialize_error(accept, std::format("no format plugin loaded for '{}'", accept));
        }
        auto encoded = format->encode(rfl::to_generic(value));
        if (!encoded) {
            return serialize_error(accept, encoded.error());
        }
        return to_bytes(*encoded);
    }

    /**
     * @brief Deserializes raw bytes into T, dispatching to whichever format plugin is
     * registered in `SerdeFormatRegistry` for `content_type`.
     * @tparam T the type being decoded into.
     * @param content_type the wire content-type (e.g. a Content-Type header value) used to
     * pick which registered format's `decode` runs.
     * @param data the raw encoded bytes/text to decode.
     * @warning No format registered for `content_type` (i.e. the plugin isn't loaded) returns
     * a clean error, same as an unsupported content-type always did — no silent fallback.
     * @return the decoded T, or an error message if no format is registered for
     * `content_type`, the registered format's `decode` fails, or the decoded generic value
     * doesn't match T's shape.
     */
    template <typename T>
    [[nodiscard]] static std::expected<T, std::string> deserialize(std::string_view content_type,
                                                                   std::string_view data) {
        auto *registry = SerdeFormatRegistry::get_active();
        auto *format = registry != nullptr ? registry->find(content_type) : nullptr;
        if (format == nullptr) {
            return std::unexpected{
                std::format("no format plugin loaded for '{}'", content_type)};
        }
        auto decoded = format->decode(data);
        if (!decoded) {
            return std::unexpected{decoded.error()};
        }
        auto result = rfl::from_generic<T>(*decoded);
        if (!result) {
            return std::unexpected{std::string{result.error().what()}};
        }
        return *result;
    }

    /**
     * @brief Decodes raw bytes into the dynamic `rfl::Generic` tree itself, dispatching to
     * whichever format plugin is registered for `content_type` — skips the `rfl::from_generic
     * <T>` step `deserialize<T>()` does, for callers that want to navigate an unknown-shape
     * document (e.g. walking an OpenAPI spec) rather than decode into a known reflected type.
     * @param content_type the wire content-type used to pick which registered format's
     * `decode` runs.
     * @param data the raw encoded bytes/text to decode.
     * @warning Same no-silent-fallback rule as `deserialize<T>()` — no format registered for
     * `content_type` is a clean error, not a guess.
     * @return the decoded `rfl::Generic` tree, or an error message if no format is registered
     * for `content_type` or the registered format's `decode` fails.
     */
    [[nodiscard]] static std::expected<Value, std::string>
    decode_generic(std::string_view content_type, std::string_view data) {
        auto *registry = SerdeFormatRegistry::get_active();
        auto *format = registry != nullptr ? registry->find(content_type) : nullptr;
        if (format == nullptr) {
            return std::unexpected{
                std::format("no format plugin loaded for '{}'", content_type)};
        }
        return format->decode(data);
    }

    /**
     * @brief JSON-encodes `value` directly via `rfl::json::write` — always JSON, no registry
     * lookup, no `accept` parameter. For callers that need JSON specifically as an internal
     * storage/embedding format (e.g. `Cache::cache_value`, `connector::Sql`'s
     * `json_populate_record` payloads) rather than a content-negotiated wire format; those
     * callers shouldn't need their own `#include <rfl/json.hpp>` just for this one call.
     * @tparam T the type being encoded.
     * @param value the instance to encode.
     * @return the JSON-encoded string.
     */
    template <typename T>
    [[nodiscard]] static std::string encode_json(const T &value) {
        return rfl::json::write(value);
    }

    /**
     * @brief Wraps an error message as a minimal `{"error":"..."}` JSON payload — always
     * JSON, `accept` is ignored here too. Hand-formatted, not routed through any format
     * plugin — has to work even when nothing is loaded, since it's what reports that.
     * @param message the error text to embed.
     * @warning `message` is dropped straight into the JSON string with no escaping — a
     * message containing a `"` or control character produces malformed JSON. Only feed this
     * trusted, pre-sanitized text.
     * @return the encoded error payload as bytes.
     */
    [[nodiscard]] static std::vector<std::byte> serialize_error(std::string_view /*accept*/,
                                                                std::string_view message) {
        return to_bytes(std::format(R"({{"error":"{}"}})", message));
    }

    /**
     * @brief Passes `data` straight through to bytes, no encoding applied — for callers
     * that already have wire-ready text and just need the byte-vector shape.
     * @param data the pre-encoded text to pass through untouched.
     * @return `data`'s bytes, unchanged.
     */
    [[nodiscard]] static std::vector<std::byte> serialize_raw(std::string_view /*accept*/,
                                                              std::string_view data) {
        return to_bytes(data);
    }
};

/**
 * @brief Global content-negotiation middleware — validates a request's content types against the
 * registered serde formats before it ever reaches a handler, so a request the server can't
 * actually parse or represent is rejected up front with the right status instead of being run
 * and then handed a success status carrying a `{"error":...}` body it couldn't serialize.
 *
 * Two checks, exact-match against `SerdeFormatRegistry` (no wildcard/fallback — the server only
 * speaks the formats a plugin registered):
 *   - inbound: if the request carries a body, its `Content-Type` must name a registered format,
 *     else `415 Unsupported Media Type`;
 *   - outbound: the `Accept` header must name a registered format, else `406 Not Acceptable`
 *     (an absent/empty Accept, and the wildcard `* / *`, both fail — there is no default).
 * On a miss it replies + calls `send` and does NOT call `next`, which (via the router's global
 * `execute_wrapping`) skips the handler entirely. On a clean match it calls `next` to continue.
 *
 * Must be a plain free function: `interfaces::MiddlewareFn` is a function pointer, so this can't
 * capture — it reads the process-global `SerdeFormatRegistry::get_active()` instead.
 * @param req the inbound request; its `content-type`/`accept` headers and body are read.
 * @param res the response — set to 415/406 with an error body on a rejection.
 * @param next the continuation into the rest of dispatch; called only on the accept path.
 * @param send the response-completion callback; called only on a rejection here.
 */
inline void content_negotiation_middleware(interfaces::io::IRequest &req,
                                           interfaces::io::IResponse &res, interfaces::NextFn &&next,
                                           std::function<void()> send) {
    auto *registry = SerdeFormatRegistry::get_active();

    // inbound body format — only meaningful when there actually is a body to parse
    auto content_type = req.find_header("content-type");
    if (!req.get_body().empty() &&
        (registry == nullptr || registry->find(content_type) == nullptr)) {
        res.set_body(Ser::serialize_error(
            content_type,
            std::format("no serde format registered for Content-Type '{}'", content_type)));
        res.set_status(interfaces::io::types::Status::UNSUPPORTED_MEDIA_TYPE);
        send();
        return;
    }

    // outbound response format
    auto accept = req.find_header("accept");
    if (registry == nullptr || registry->find(accept) == nullptr) {
        res.set_body(Ser::serialize_error(
            accept, std::format("no serde format registered for Accept '{}'", accept)));
        res.set_status(interfaces::io::types::Status::NOT_ACCEPTABLE);
        send();
        return;
    }

    next(req, res, std::move(send));
}

} // namespace serde
