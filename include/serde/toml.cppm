module;
#include <rfl/toml.hpp>
#include <toml++/toml.hpp>

export module serde:toml;

import :converter;
import std;

export namespace serde {

class Toml {
  public:
    static constexpr std::string_view content_type = "application/toml";

    template <ISerializable T>
    [[nodiscard]] static std::string encode(const T &value) {
        return rfl::toml::write(value);
    }

    template <ISerializable T>
    [[nodiscard]] static std::expected<T, std::string> decode(std::string_view data) {
        toml::table tbl;
        try {
            tbl = toml::parse(data);
        } catch (const toml::parse_error &ex) {
            return std::unexpected{std::string{ex.description()}};
        }
        T obj{};
        if (auto result = TomlParser::from_toml_impl(tbl, obj); !result)
            return std::unexpected{result.error()};
        return obj;
    }
};

} // namespace serde

export namespace model {

class Toml {
  public:
    template <serde::ISerializable T>
    [[nodiscard]] static std::expected<void, std::string> from_toml(const toml::table &t,
                                                                     T &obj) {
        return serde::TomlParser::from_toml_impl(t, obj);
    }
};

} // namespace model
