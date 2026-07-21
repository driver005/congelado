module;
#include <rfl/toml.hpp>
#include <toml++/toml.hpp>

export module serde:toml;

import :converter;
import std;

export namespace serde {

class Toml {
  public:
    // FIXME(clang-tidy): readability-identifier-naming — must stay lowercase `content_type` to
    // satisfy the IAnyFormat concept (serde/core.cppm) shared with Json; renaming needs a
    // coordinated cross-file change, out of scope here.
    static constexpr std::string_view content_type = "application/toml";  // NOLINT(readability-identifier-naming) — must match IAnyFormat concept's content_type requirement shared with Json

    /**
     * @brief Encodes an ISerializable value to a TOML string via rfl's reflection (this
     * overload is what satisfies IFormat<Toml, T>).
     * @tparam T the serializable type being encoded.
     * @param value the instance to encode.
     * @return the TOML-encoded string.
     */
    template <ISerializable T>
    [[nodiscard]] static std::string encode(const T &value) {
        return rfl::toml::write(value);
    }

    /**
     * @brief Encodes a non-ISerializable value to a TOML string — same fallback motion as
     * Json::encode's twin overload, straight to rfl's own reflection for plain types.
     * @tparam T the type being encoded, constrained to NOT satisfy ISerializable.
     * @param value the instance to encode.
     * @return the TOML-encoded string.
     */
    template <typename T>
        requires(!ISerializable<T>)
    [[nodiscard]] static std::string encode(const T &value) {
        return rfl::toml::write(value);
    }

    /**
     * @brief Decodes a TOML document into an ISerializable T via toml++'s parser, then walks
     * T's fields through TomlParser::from_toml_impl — unlike Json's on-demand path, this one
     * builds a full `toml::table` up front before any field extraction happens.
     * @tparam T the serializable type being decoded into.
     * @param data the raw TOML text to parse.
     * @return the decoded T, or an error message if `data` isn't valid TOML (parse_error's
     * description) or a required field is missing/mistyped for T — no partial object handed
     * back on an L, bet.
     */
    template <ISerializable T>
    [[nodiscard]] static std::expected<T, std::string> decode(std::string_view data) {
        // toml++ throws on a parse failure, unlike the rest of this codebase's expected-based
        // error handling — catch it here and translate to the usual unexpected/string shape.
        toml::table parsed_table;
        try {
            parsed_table = toml::parse(data);
        } catch (const toml::parse_error &parse_error) {
            return std::unexpected{std::string{parse_error.description()}};
        }
        // Table parsed clean — now walk T's fields out of it, bailing on the first missing or
        // mistyped one, no partial object on an L.
        T result{};
        if (auto outcome = TomlParser::from_toml_impl(parsed_table, result); !outcome) {
            return std::unexpected{outcome.error()};
        }
        return result;
    }
};

} // namespace serde

export namespace model {

class Toml {
  public:
    /**
     * @brief Thin pass-through to serde::TomlParser::from_toml_impl for callers already
     * outside the `serde` namespace — lets `model`-space code populate an object from an
     * already-parsed `toml::table` without reaching back into `serde` directly.
     * @tparam T the serializable type being populated.
     * @param toml_table the already-parsed TOML table to read fields from.
     * @param obj the object whose fields get set, mutated in place.
     * @return success, or an error message the moment any field fails to extract.
     */
    template <serde::ISerializable T>
    [[nodiscard]] static std::expected<void, std::string> from_toml(const toml::table &toml_table,
                                                                     T &obj) {
        return serde::TomlParser::from_toml_impl(toml_table, obj);
    }
};

} // namespace model
