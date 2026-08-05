export module engine:schema;

import std;

export namespace engine {

/**
 * @brief Loose JSON-schema checking for TaskDef's `input_schema`/`output_schema` — deliberately
 * NOT full JSON-schema semantics (no type/format/pattern/nested-object validation). This
 * codebase's TaskInstance input/output data is a flat `string -> string` map (see task/instance.
 * cppm's own docs on why), so there's no meaningful way to check a nested `properties` shape
 * against it anyway; the one thing that DOES translate cleanly is "does this top-level key
 * exist at all," so that's the whole check. Reaching for a real JSON-schema validator library
 * is the natural next step if this ever needs more than that — flagged, not silently pretended
 * to be complete.
 */
class SchemaValidator {
  public:
    /**
     * @brief Checks that every name in `schema_json`'s top-level `"required": [...]` array is
     * present as a key in `data`.
     * @param schema_json a JSON-schema document (only its `required` array is read — every
     * other keyword, including `properties`/`type`, is ignored).
     * @param data the flat input/output map to check field presence against.
     * @return an empty expected if every required key is present (or `schema_json` has no
     * `required` array at all), otherwise an unexpected naming the first missing key.
     */
    [[nodiscard]] static std::expected<void, std::string>
    validate(std::string_view schema_json, const std::unordered_map<std::string, std::string> &data) {
        auto required = extract_required(schema_json);
        for (auto const &key : required) {
            if (!data.contains(key)) {
                return std::unexpected{std::format("missing required field '{}'", key)};
            }
        }
        return {};
    }

  private:
    /// @brief Pulls the string literals out of `schema_json`'s top-level `"required":[...]`
    /// array, if one exists.
    [[nodiscard]] static std::vector<std::string> extract_required(std::string_view schema_json) {
        std::vector<std::string> names;
        auto key_pos = schema_json.find("\"required\"");
        if (key_pos == std::string_view::npos) {
            return names;
        }
        auto open = schema_json.find('[', key_pos);
        if (open == std::string_view::npos) {
            return names;
        }
        auto close = matching_bracket(schema_json, open);
        if (close == std::string_view::npos) {
            return names;
        }
        bool in_string = false;
        std::string current;
        for (std::size_t index = open + 1; index < close; ++index) {
            char character = schema_json[index];
            if (in_string) {
                if (character == '\\' && index + 1 < close) {
                    current += schema_json[++index];
                } else if (character == '"') {
                    names.push_back(std::move(current));
                    current.clear();
                    in_string = false;
                } else {
                    current += character;
                }
            } else if (character == '"') {
                in_string = true;
            }
        }
        return names;
    }

    /// @brief Finds the index of the `]` matching the `[` at `open`, tracking nesting depth and
    /// skipping over quoted-string contents — same reasoning as any other hand-rolled
    /// balanced-bracket scan in this codebase (e.g. elasticsearch_plugin's `matching_brace()`):
    /// no JSON library is linked here to lean on instead.
    [[nodiscard]] static std::size_t matching_bracket(std::string_view text, std::size_t open) {
        int depth = 0;
        bool in_string = false;
        for (std::size_t index = open; index < text.size(); ++index) {
            char character = text[index];
            if (in_string) {
                if (character == '\\') {
                    ++index;
                } else if (character == '"') {
                    in_string = false;
                }
                continue;
            }
            if (character == '"') {
                in_string = true;
            } else if (character == '[') {
                ++depth;
            } else if (character == ']') {
                --depth;
                if (depth == 0) {
                    return index;
                }
            }
        }
        return std::string_view::npos;
    }
};

} // namespace engine
