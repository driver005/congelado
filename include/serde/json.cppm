module;
#include <rfl/json.hpp>
#include <simdjson.h>

export module serde:json;

import :converter;
import std;

export namespace serde {

using Value = rfl::Generic;

// Parses and navigates a dynamic (non-ISerializable) JSON value tree.
class Document {
  public:
    /**
     * @brief Parses a raw JSON string into a dynamic Value tree — no target type needed,
     * this is for navigating JSON whose shape you don't know ahead of time, bet.
     * @param data the raw JSON text to parse.
     * @return the parsed Value tree, or an error message if `data` isn't valid JSON.
     */
    [[nodiscard]] static std::expected<Value, std::string> parse(std::string_view data) {
        // Let rfl do the actual parsing, then flatten its error type down to a plain string.
        auto result = rfl::json::read<Value>(data);
        if (!result) {
            return std::unexpected{result.error().what()};
        }
        return *result;
    }

    /**
     * @brief Loads and parses a JSON file from disk into a dynamic Value tree.
     * @param path the filesystem path to read.
     * @return the parsed Value tree, or an error message (path + underlying parse failure)
     * if the file's missing, unreadable, or isn't valid JSON.
     */
    [[nodiscard]] static std::expected<Value, std::string> load(const std::filesystem::path &path) {
        // Read + parse in one rfl call — on failure, fold the path into the error message so
        // it's obvious which file went sideways.
        auto result = rfl::json::load<Value>(path.string());
        if (!result) {
            return std::unexpected{
                std::format("failed to parse '{}': {}", path.string(), result.error().what())};
        }
        return *result;
    }

    /**
     * @brief Chained object-key lookup — serde::Value has no operator[] chaining of its own,
     * so this is the motion for reaching into nested objects without a null check per hop.
     * @param value the root Value to start the walk from.
     * @param keys the sequence of object keys to descend through, in order.
     * @return the Value found at the end of the key chain, or `nullopt` the moment any hop
     * isn't an object or is missing the next key — bails clean, no exception.
     */
    // Chained object-key lookup — serde::Value has no operator[] chaining of its own.
    [[nodiscard]] static std::optional<Value>
    at(const Value &value, std::initializer_list<std::string_view> keys) {
        Value current = value;
        // Walk the key chain one hop at a time — bail clean the moment the current value isn't
        // an object or the next key just isn't there, no exceptions thrown either way.
        for (auto key : keys) {
            auto object = current.to_object();
            if (!object) {
                return std::nullopt;
            }
            auto found = object->get(std::string{key});
            if (!found) {
                return std::nullopt;
            }
            current = *found;
        }
        return current;
    }
};

class Json {
  public:
    // FIXME(clang-tidy): readability-identifier-naming — must stay lowercase `content_type` to
    // satisfy the IAnyFormat concept (serde/core.cppm) shared with Toml; renaming needs a
    // coordinated cross-file change, out of scope here.
    static constexpr std::string_view content_type = "application/json";  // NOLINT(readability-identifier-naming) — must match IAnyFormat concept's content_type requirement shared with Toml

    /**
     * @brief Encodes an ISerializable value to a JSON string via the FieldConverter/rfl
     * reflection pipeline (this overload is what satisfies IFormat<Json, T>).
     * @tparam T the serializable type being encoded.
     * @param value the instance to encode.
     * @return the JSON-encoded string.
     */
    template <ISerializable T>
    [[nodiscard]] static std::string encode(const T &value) {
        return rfl::json::write(value);
    }

    /**
     * @brief Encodes a non-ISerializable value to a JSON string — falls straight through
     * to rfl's own reflection for plain types (primitives, containers, rfl-aware structs)
     * that never opted into this codebase's FieldDesc-based Serializable machinery.
     * @tparam T the type being encoded, constrained to NOT satisfy ISerializable.
     * @param value the instance to encode.
     * @return the JSON-encoded string.
     */
    template <typename T>
        requires(!ISerializable<T>)
    [[nodiscard]] static std::string encode(const T &value) {
        return rfl::json::write(value);
    }

    /**
     * @brief Decodes a single JSON object into an ISerializable T, on-demand style via
     * simdjson — the fast path this codebase leans on hard for hot deserialization.
     * @tparam T the serializable type being decoded into.
     * @param data the raw JSON text to parse (must own its lifetime — simdjson pads it
     * internally via `padded_string`, so no dangling-view worries here).
     * @return the decoded T, or an error message if `data` isn't valid JSON or doesn't
     * satisfy T's field requirements.
     */
    template <ISerializable T>
    [[nodiscard]] static std::expected<T, std::string> decode(std::string_view data) {
        // simdjson needs its own padded buffer, so wrap `data` before handing it to the
        // on-demand parser and pulling the top-level value out.
        simdjson::ondemand::parser parser;
        simdjson::padded_string padded{data};
        auto doc = parser.iterate(padded);
        T obj{};
        simdjson::ondemand::value val = doc.get_value();
        // Hand off to T's tag_invoke overload to walk every reflected field — bail with the
        // simdjson error message the moment any field fails to decode.
        if (auto error_code = simdjson::tag_invoke(simdjson::deserialize_tag{}, val, obj); error_code) {
            return std::unexpected{std::string{simdjson::error_message(error_code)}};
        }
        return obj;
    }

    /**
     * @brief Decodes a JSON array into a vector of ISerializable T, one element at a time —
     * straight linear walk, no cap.
     * @tparam T the serializable element type.
     * @param data the raw JSON text — must be a top-level array or this bails on the first
     * `get_array()` call.
     * @return the decoded elements in array order, or an error message the moment any
     * element fails to parse or doesn't decode as T — no partial results on failure.
     */
    template <ISerializable T>
    [[nodiscard]] static std::expected<std::vector<T>, std::string>
    decode_array(std::string_view data) {
        simdjson::ondemand::parser parser;
        simdjson::padded_string padded{data};
        auto doc = parser.iterate(padded);
        // `data` must be a top-level array — bail immediately if simdjson can't see it as one.
        simdjson::ondemand::array json_array;
        if (auto ec = doc.get_array().get(json_array); ec) {
            return std::unexpected{std::string{simdjson::error_message(ec)}};
        }
        std::vector<T> result;
        // Decode each element in order — first failure (bad element or a T that doesn't
        // satisfy its own fields) bails the whole call, no partial results handed back.
        for (auto element : json_array) {
            simdjson::ondemand::value element_value;
            if (auto ec = element.get(element_value); ec) {
                return std::unexpected{std::string{simdjson::error_message(ec)}};
            }
            T obj{};
            if (auto ec = simdjson::tag_invoke(simdjson::deserialize_tag{}, element_value, obj); ec) {
                return std::unexpected{std::string{simdjson::error_message(ec)}};
            }
            result.push_back(std::move(obj));
        }
        return result;
    }
};

} // namespace serde
