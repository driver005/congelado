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

    template <typename T>
        requires(!ISerializable<T>)
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

    template <ISerializable T>
    [[nodiscard]] static std::expected<std::vector<T>, std::string>
    decode_array(std::string_view data) {
        simdjson::ondemand::parser parser;
        simdjson::padded_string padded{data};
        auto doc = parser.iterate(padded);
        simdjson::ondemand::array arr;
        if (auto ec = doc.get_array().get(arr); ec)
            return std::unexpected{std::string{simdjson::error_message(ec)}};
        std::vector<T> result;
        for (auto elem : arr) {
            simdjson::ondemand::value val;
            if (auto ec = elem.get(val); ec)
                return std::unexpected{std::string{simdjson::error_message(ec)}};
            T obj{};
            if (auto ec = simdjson::tag_invoke(simdjson::deserialize_tag{}, val, obj); ec)
                return std::unexpected{std::string{simdjson::error_message(ec)}};
            result.push_back(std::move(obj));
        }
        return result;
    }
};

} // namespace serde
