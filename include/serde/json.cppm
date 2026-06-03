module;
#include <rfl/json.hpp>
#include <simdjson.h>

export module serde:json;

import :converter;
import std;

export namespace serde {

class Json {
  public:
    static constexpr std::string_view content_type = "application/json";

    template <ISerializable T>
    [[nodiscard]] static std::string encode(const T &value) {
        return rfl::json::write(value);
    }

    template <ISerializable T>
    [[nodiscard]] static std::expected<T, std::string> decode(std::string_view data) {
        simdjson::ondemand::parser parser;
        simdjson::padded_string padded{data};
        auto doc = parser.iterate(padded);
        T obj{};
        simdjson::ondemand::value val = doc.get_value();
        if (auto error_code = simdjson::tag_invoke(simdjson::deserialize_tag{}, val, obj); error_code)
            return std::unexpected{std::string{simdjson::error_message(error_code)}};
        return obj;
    }
};

} // namespace serde
