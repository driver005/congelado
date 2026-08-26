export module workflow_engine:schema;

import std;
import serde;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace engine {

/**
 * @brief Loose JSON-schema checking for TaskDef's `input_schema`/`output_schema` — deliberately
 * NOT full JSON-schema semantics (no type/format/pattern/nested-object validation). The one
 * thing that DOES translate cleanly against a dynamic `serde::Value` input is "does this
 * top-level key exist at all," so that's the whole check. Reaching for a real JSON-schema
 * validator library is the natural next step if this ever needs more than that — flagged, not
 * silently pretended to be complete.
 */
class SchemaValidator
{
public:
    /**
     * @brief Checks that every name in `schema_json`'s top-level `"required": [...]` array is
     * present as a key in `data`.
     * @param schema_json a JSON-schema document (only its `required` array is read — every
     * other keyword, including `properties`/`type`, is ignored).
     * @param data the dynamic input value to check field presence against (must be an object).
     * @return an empty expected if every required key is present (or `schema_json` has no
     * `required` array at all), otherwise an unexpected naming the first missing key.
     */
    [[nodiscard]] static std::expected<void, std::string>
    validate(std::string_view schema_json, const serde::Value& data)
    {
        auto required = extract_required(schema_json);
        auto object = data.to_object();
        for (const auto& key: required) {
            if (!object || object->count(key) == 0) {
                return std::unexpected{std::format("missing required field '{}'", key)};
            }
        }
        return {};
    }

private:
    /// @brief Pulls the string literals out of `schema_json`'s top-level `"required":[...]`
    /// array, if one exists.
    [[nodiscard]] static std::vector<std::string> extract_required(std::string_view schema_json)
    {
        std::vector<std::string> names;
        auto key_pos = schema_json.find("\"required\"");
        if (key_pos == std::string_view::npos) {
            return names;
        }
        // BUG: this looks for the next '[' anywhere after the "required" key, not immediately
        // after its ':' — if "required"'s own value isn't an array (e.g. a string, or absent
        // entirely because of a typo), this happily latches onto the FIRST unrelated array that
        // appears later in the document (e.g. some other field's array value) and treats its
        // string entries as required field names instead. schema_json is TaskDef::input_schema,
        // attacker-settable unauthenticated via POST/PUT /api/v1/tasks (same reachability as
        // the routes.cppm/lua_eval.cppm SECURITY findings), so a crafted schema can make
        // unrelated fields spuriously "required" (denying otherwise-valid task input) or, if no
        // later array exists at all, silently fall back to "no required fields" instead of
        // erroring.
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
    [[nodiscard]] static std::size_t matching_bracket(std::string_view text, std::size_t open)
    {
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

#ifdef CONGELADO_TEST
namespace engine::schema_validator_tests {
using namespace boost::ut;

/// @brief Builds an object serde::Value from a flat string map — the shape SchemaValidator's
/// `data` param always has when driven off Orchestrator::to_value_input().
[[nodiscard]] serde::Value make_object(const std::unordered_map<std::string, std::string>& fields)
{
    serde::Value::Object object;
    for (const auto& [key, value]: fields) {
        object.emplace(key, serde::Value{value});
    }
    return serde::Value{std::move(object)};
}

suite<"SchemaValidator::validate happy paths"> schema_validator_happy_suite = [] {
    "a schema with no 'required' array passes regardless of data"_test = [] {
        auto result = SchemaValidator::validate(R"({"type":"object"})", make_object({}));
        expect(bool(result));
    };

    "every required key present passes"_test = [] {
        auto schema = R"({"required":["order_id","amount"]})";
        auto result =
            SchemaValidator::validate(schema, make_object({{"order_id", "1"}, {"amount", "5"}}));
        expect(bool(result));
    };

    "extra keys beyond 'required' are fine, only presence of required ones matters"_test = [] {
        auto schema = R"({"required":["order_id"]})";
        auto result =
            SchemaValidator::validate(schema, make_object({{"order_id", "1"}, {"unrelated", "x"}}));
        expect(bool(result));
    };
};

suite<"SchemaValidator::validate rejects missing fields"> schema_validator_missing_suite = [] {
    "a missing required key fails, naming that key"_test = [] {
        auto schema = R"({"required":["order_id","amount"]})";
        auto result = SchemaValidator::validate(schema, make_object({{"order_id", "1"}}));
        expect(!result.has_value()) << fatal;
        expect(result.error() == "missing required field 'amount'");
    };

    "the first missing key wins, in required-array order"_test = [] {
        auto schema = R"({"required":["first","second"]})";
        auto result = SchemaValidator::validate(schema, make_object({}));
        expect(!result.has_value()) << fatal;
        expect(result.error() == "missing required field 'first'");
    };

    "non-object data with a non-empty required list fails on the first name"_test = [] {
        auto schema = R"({"required":["order_id"]})";
        auto result = SchemaValidator::validate(schema, serde::Value{std::string{"not an object"}});
        expect(!result.has_value()) << fatal;
        expect(result.error() == "missing required field 'order_id'");
    };

    // Loose-check gap, matches this class's own documented scope (top-level key presence only,
    // not real JSON-schema semantics): non-object data with an EMPTY required list still
    // passes, since there's nothing to check presence of — validate() never confirms `data` is
    // even an object shape at all when required is empty.
    "non-object data with an empty required list still passes"_test = [] {
        auto result =
            SchemaValidator::validate(R"({"required":[]})", serde::Value{std::int64_t{42}});
        expect(bool(result));
    };
};

suite<"SchemaValidator hand-rolled bracket scan"> schema_validator_scan_suite = [] {
    "an escaped quote inside a required name parses without truncating"_test = [] {
        auto schema = R"({"required":["a\"b"]})";
        auto result = SchemaValidator::validate(schema, make_object({}));
        expect(!result.has_value()) << fatal;
        expect(result.error() == R"(missing required field 'a"b')");
    };

    "literal bracket characters inside a quoted required name don't confuse the scan"_test = [] {
        auto schema = R"({"required":["weird[0]name"]})";
        auto result = SchemaValidator::validate(schema, make_object({}));
        expect(!result.has_value()) << fatal;
        expect(result.error() == "missing required field 'weird[0]name'");
    };

    "an unterminated required array is treated as no required fields, no crash"_test = [] {
        auto schema = R"({"required":["order_id")";
        auto result = SchemaValidator::validate(schema, make_object({}));
        expect(bool(result));
    };

    "malformed JSON with no closing brace at all still doesn't crash"_test = [] {
        auto result = SchemaValidator::validate(R"({"required": [)", make_object({}));
        expect(bool(result));
    };

    // BUG pin (see the `// BUG:` comment on extract_required's `find('[', key_pos)` line
    // above): "required" here is a plain string, not an array, so there's genuinely nothing
    // required — but the scan latches onto the unrelated "other_field" array later in the
    // document and treats ITS entries as required field names instead.
    "BUG: a non-array 'required' value spuriously adopts a later unrelated array's entries"_test =
        [] {
            auto schema = R"({"required": "nope", "other_field": ["x", "y"]})";
            auto result = SchemaValidator::validate(schema, make_object({}));
            expect(!result.has_value()) << fatal;
            expect(result.error() == "missing required field 'x'");

            // Providing the spuriously-adopted names satisfies the (bogus) check.
            auto satisfied =
                SchemaValidator::validate(schema, make_object({{"x", "1"}, {"y", "2"}}));
            expect(bool(satisfied));
        };
};

} // namespace engine::schema_validator_tests
#endif
