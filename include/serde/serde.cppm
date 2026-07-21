export module serde;

export import :core;
export import :converter;
export import :json;
export import :toml;
export import :cache;
export import :sql;

export namespace serde {

// ─── SerBase / Ser ────────────────────────────────────────────────────────────
//
// Runtime wire-format dispatch. Reads the Accept / Content-Type header value
// and dispatches to a format class satisfying IFormat<F,T>.
//
// To add a new wire format: define a class satisfying IFormat<F,T> and append
// it to the Ser alias at the bottom of this file.

template <IAnyFormat... Fmts>
class SerBase {
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
     * @brief Serializes a single value, picking the format whose `content_type` matches
     * `accept` out of the `Fmts...` pack — falls back to Json::encode if nothing matches,
     * so this never actually fails, bet.
     * @tparam T the serializable type being encoded.
     * @param accept the requested content-type (e.g. an Accept header value) to match
     * against each format's `content_type`.
     * @param value the instance to serialize.
     * @return the encoded bytes, in whichever format matched (or JSON if none did).
     */
    template <ISerializable T>
    [[nodiscard]] static std::vector<std::byte> serialize(std::string_view accept, const T &value) {
        std::string encoded;
        // Fold across Fmts... looking for a content_type match — first hit encodes and short
        // circuits the ||. Nothing matched? Fall back to JSON, bet, this never actually fails.
        bool matched =
            ((Fmts::content_type == accept && (encoded = Fmts::encode(value), true)) || ...);
        if (!matched) {
            encoded = Json::encode(value);
        }
        return to_bytes(encoded);
    }

    /**
     * @brief Serializes a vector of values as a JSON array — always JSON here regardless of
     * `accept`, unlike the single-value overload above. Straight up hardcoded to
     * Json::encode per element, no format-pack dispatch for this shape.
     * @tparam T the serializable element type.
     * @param values the instances to serialize, in order.
     * @warning `accept` is silently ignored — a caller requesting `"application/toml"` for
     * a vector still gets JSON back, no error, no fallback signal. Inconsistent with the
     * single-value overload right above it, which actually honors `accept`. Real footgun if
     * a Toml-only client ever hits a list endpoint.
     * @return the encoded `[elem,elem,...]` JSON array as bytes.
     */
    template <ISerializable T>
    [[nodiscard]] static std::vector<std::byte> serialize(std::string_view /*accept*/,
                                                          const std::vector<T> &values) {
        std::string encoded = "[";
        bool first = true;
        // Walk every element, comma-separating all but the first — straight linear join, no
        // format-pack dispatch here since this overload is hardcoded to JSON regardless of accept.
        for (const auto &value : values) {
            if (!first) {
                encoded += ',';
            }
            encoded += Json::encode(value);
            first = false;
        }
        encoded += ']';
        return to_bytes(encoded);
    }

    /**
     * @brief Serializes a non-ISerializable value, same content-negotiation motion as the
     * ISerializable single-value overload — matches `accept` against the `Fmts...` pack,
     * falls back to JSON if nothing matches.
     * @tparam T the type being encoded, constrained to NOT satisfy ISerializable.
     * @param accept the requested content-type to match against each format's
     * `content_type`.
     * @param value the instance to serialize.
     * @return the encoded bytes, in whichever format matched (or JSON if none did).
     */
    template <typename T>
        requires(!ISerializable<T>)
    [[nodiscard]] static std::vector<std::byte> serialize(std::string_view accept, const T &value) {
        std::string encoded;
        // Same fold-and-fallback motion as the ISerializable overload above — match accept
        // against Fmts..., default to JSON if nothing lands.
        bool matched =
            ((Fmts::content_type == accept && (encoded = Fmts::encode(value), true)) || ...);
        if (!matched) {
            encoded = Json::encode(value);
        }
        return to_bytes(encoded);
    }

    /**
     * @brief Deserializes raw bytes into T, picking the format whose `content_type` matches
     * `content_type` out of the `Fmts...` pack.
     * @tparam T the serializable type being decoded into.
     * @param content_type the wire content-type (e.g. a Content-Type header value) used to
     * pick which format's `decode` runs.
     * @param data the raw encoded bytes/text to decode.
     * @warning Unlike `serialize`, there's no silent JSON fallback here — if `content_type`
     * doesn't match any format in `Fmts...`, this returns the `"unsupported content-type"`
     * error straight up, no cap.
     * @return the decoded T, or an error message on an unsupported content-type or a decode
     * failure from the matched format.
     */
    template <ISerializable T>
    [[nodiscard]] static std::expected<T, std::string> deserialize(std::string_view content_type,
                                                                   std::string_view data) {
        // Default to an error — only a matching Fmts entry overwrites this, so no match at all
        // just leaves the "unsupported content-type" result standing, no JSON fallback like
        // serialize() gets.
        std::expected<T, std::string> result =
            std::unexpected{std::string{"unsupported content-type"}};
        (void)((Fmts::content_type == content_type &&
                (result = Fmts::template decode<T>(data), true)) ||
               ...);
        return result;
    }

    /**
     * @brief Wraps an error message as a minimal `{"error":"..."}` JSON payload — always
     * JSON, `accept` is ignored here too.
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

using Ser = SerBase<Json, Toml>;

} // namespace serde
