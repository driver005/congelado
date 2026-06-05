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
    [[nodiscard]] static std::vector<std::byte> to_bytes(std::string_view str) {
        std::vector<std::byte> bytes(str.size());
        std::ranges::transform(str, bytes.begin(), [](char c) noexcept { return std::byte(c); });
        return bytes;
    }

  public:
    template <ISerializable T>
    [[nodiscard]] static std::vector<std::byte> serialize(std::string_view accept, const T &value) {
        std::string encoded;
        if (!((Fmts::content_type == accept && (encoded = Fmts::encode(value), true)) || ...))
            encoded = Json::encode(value);
        return to_bytes(encoded);
    }

    template <ISerializable T>
    [[nodiscard]] static std::vector<std::byte> serialize(std::string_view /*accept*/,
                                                          const std::vector<T> &values) {
        std::string encoded = "[";
        bool first = true;
        for (const auto &v : values) {
            if (!first)
                encoded += ',';
            encoded += Json::encode(v);
            first = false;
        }
        encoded += ']';
        return to_bytes(encoded);
    }

    template <typename T>
        requires(!ISerializable<T>)
    [[nodiscard]] static std::vector<std::byte> serialize(std::string_view accept, const T &value) {
        std::string encoded;
        if (!((Fmts::content_type == accept && (encoded = Fmts::encode(value), true)) || ...))
            encoded = Json::encode(value);
        return to_bytes(encoded);
    }

    template <ISerializable T>
    [[nodiscard]] static std::expected<T, std::string> deserialize(std::string_view content_type,
                                                                   std::string_view data) {
        std::expected<T, std::string> result =
            std::unexpected{std::string{"unsupported content-type"}};
        ((Fmts::content_type == content_type && (result = Fmts::template decode<T>(data), true)) ||
         ...);
        return result;
    }

    [[nodiscard]] static std::vector<std::byte> serialize_error(std::string_view /*accept*/,
                                                                std::string_view message) {
        return to_bytes(std::format(R"({{"error":"{}"}})", message));
    }

    [[nodiscard]] static std::vector<std::byte> serialize_raw(std::string_view /*accept*/,
                                                              std::string_view data) {
        return to_bytes(data);
    }
};

using Ser = SerBase<Json, Toml>;

} // namespace serde
