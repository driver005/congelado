export module openapi_generator_plugin:document;

import std;
import serde;

// Navigates a parsed OpenAPI document as a dynamic value tree. The actual JSON bytes-to-
// serde::Value step goes through serde::Ser::decode_generic (the same JSON format plugin every
// other consumer dispatches through — build.cc bootstraps it before this ever runs), so this
// class no longer duplicates plugins/json/json_plugin.cc's own rfl::json::read call; it just
// adds the file-read + chained-key-lookup motions the codegen tool actually needs on top. Only
// `serde` itself (plus the two format plugins, which implement the ABI contract) touches rfl
// headers directly — this file reaches the reflected-value type through serde::Value instead.

export namespace congelado::client {

class Document {
  public:
    /**
     * @brief Parses a raw JSON string into a dynamic value tree via the registered JSON format
     * plugin — no target type needed, this is for navigating JSON whose shape you don't know
     * ahead of time.
     * @param data the raw JSON text to parse.
     * @return the parsed value tree, or an error message if `data` isn't valid JSON or no JSON
     * format plugin is loaded.
     */
    [[nodiscard]] static std::expected<serde::Value, std::string> parse(std::string_view data) {
        return serde::Ser::decode_generic("application/json", data);
    }

    /**
     * @brief Loads and parses a JSON file from disk into a dynamic value tree.
     * @param path the filesystem path to read.
     * @return the parsed value tree, or an error message (path + underlying parse failure) if
     * the file's missing, unreadable, or isn't valid JSON.
     */
    [[nodiscard]] static std::expected<serde::Value, std::string>
    load(const std::filesystem::path &path) {
        std::ifstream file{path};
        if (!file) {
            return std::unexpected{std::format("failed to open '{}'", path.string())};
        }
        std::string contents{std::istreambuf_iterator<char>{file}, std::istreambuf_iterator<char>{}};
        auto result = parse(contents);
        if (!result) {
            return std::unexpected{std::format("failed to parse '{}': {}", path.string(), result.error())};
        }
        return *result;
    }

    /**
     * @brief Chained object-key lookup — no operator[] chaining of its own, so this is the
     * motion for reaching into nested objects without a null check per hop.
     * @param value the root value to start the walk from.
     * @param keys the sequence of object keys to descend through, in order.
     * @return the value found at the end of the key chain, or `nullopt` the moment any hop
     * isn't an object or is missing the next key — bails clean, no exception.
     */
    [[nodiscard]] static std::optional<serde::Value>
    at(const serde::Value &value, std::initializer_list<std::string_view> keys) {
        serde::Value current = value;
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

} // namespace congelado::client
