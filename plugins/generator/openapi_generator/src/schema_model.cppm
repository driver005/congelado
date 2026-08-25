module;
#ifdef CONGELADO_TEST
// Test-only: SchemaType::parse() consumes a serde::Value (== rfl::Generic) tree, but this file
// deliberately never touches rfl headers directly in production (see document.cppm's own note on
// that boundary) — no JSON format plugin is registered in this isolated test target though, so
// serde::Ser::decode_generic() can't produce a Value to parse either. Building fixtures directly
// via rfl::Generic (same pattern plugins/serde/json/bin/json_plugin.cc's own tests use) is the
// only way to exercise parse() here; guarded so production builds never see this include.
#include <rfl/Generic.hpp>
#endif

export module openapi_generator_plugin:schema_model;

import std;
import serde;
import :document;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace congelado::client {

enum class SchemaKind : std::uint8_t { OBJECT, ARRAY, STRING, INTEGER, NUMBER, BOOLEAN, REF };

class SchemaType {
  public:
    /**
     * @brief Spins up an empty schema, String-kind by default till properties/items actually
     * show up — clean slate, no motion yet.
     */
    SchemaType() = default;

    /**
     * @brief Chews through an OpenAPI schema element and builds this SchemaType out of it —
     * handles `$ref`, `nullable`, and the primitive/object/array `type` values, recursing into
     * nested `properties` (for Object) or `items` (for Array) so the whole tree gets built in
     * one pass. No cap, this is the parser doing all the heavy lifting.
     * @param element the JSON schema node to parse.
     * @return nothing on success (W), or an error string if `$ref` is absent and `type` is
     * missing, not a string, or unrecognized, or (for Array) `items` is missing — also bubbles
     * up any error from parsing a nested property/item.
     */
    [[nodiscard]] std::expected<void, std::string> parse(const serde::Value &element) {
        if (auto nullable = Document::at(element, {"nullable"})) {
            if (auto value = nullable->to_bool()) {
                set_nullable(*value);
            }
        }

        if (auto reference = Document::at(element, {"$ref"})) {
            if (auto value = reference->to_string(); value && !value->empty()) {
                set_kind(SchemaKind::REF);
                set_ref(get_ref_name(*value));
                return {};
            }
        }

        auto type_value = Document::at(element, {"type"});
        if (!type_value) {
            return std::unexpected{"schema missing 'type'"};
        }
        auto type = type_value->to_string();
        if (!type) {
            return std::unexpected{"schema 'type' must be a string"};
        }

        auto kind = resolve_kind(*type);
        if (!kind) {
            return std::unexpected{kind.error()};
        }
        set_kind(*kind);

        if (*kind == SchemaKind::OBJECT) {
            if (auto properties = Document::at(element, {"properties"})) {
                if (auto object = properties->to_object()) {
                    for (auto &[name, value] : *object) {
                        SchemaType child;
                        if (auto result = child.parse(value); !result) {
                            return std::unexpected{result.error()};
                        }
                        add_property(name, std::move(child));
                    }
                }
            }
        } else if (*kind == SchemaKind::ARRAY) {
            auto items = Document::at(element, {"items"});
            if (!items) {
                return std::unexpected{"array schema missing 'items'"};
            }
            SchemaType child;
            if (auto result = child.parse(*items); !result) {
                return std::unexpected{result.error()};
            }
            set_items(std::move(child));
        }

        return {};
    }

    /**
     * @brief Tacks a named child property onto this schema — only actually means anything on an
     * Object-kind schema, everywhere else it's just extra baggage nobody reads.
     * @param name property name.
     * @param type the property's own parsed schema.
     */
    void add_property(std::string name, SchemaType type) {
        m_properties.emplace(std::move(name), std::move(type));
    }

    /** @brief Sets which schema kind this node reps. @param value the kind. */
    void set_kind(SchemaKind value) noexcept { m_kind = value; }
    /**
     * @brief Sets the referenced schema name — only relevant when this node's a Ref, bet.
     * @param value bare schema name.
     */
    void set_ref(std::string value) { m_ref = std::move(value); }
    /** @brief Flips whether this schema lets null ride or not. @param value nullable flag. */
    void set_nullable(bool value) noexcept { m_nullable = value; }
    /**
     * @brief Sets the element schema for an Array-kind schema — this is what makes the array
     * actually typed instead of just a vibe.
     * @param type the array element's parsed schema; stored behind a shared_ptr.
     */
    void set_items(SchemaType type) { m_items = std::make_shared<SchemaType>(std::move(type)); }

    /** @brief Gets which schema kind this node reps. @return the kind. */
    [[nodiscard]] SchemaKind get_kind() const noexcept { return m_kind; }
    /** @brief Gets the referenced schema name. @return bare schema name, empty if this ain't a Ref. */
    [[nodiscard]] const std::string &get_ref() const noexcept { return m_ref; }
    /** @brief Gets the nullable flag. @return is null on the table for this schema, or not. */
    [[nodiscard]] bool get_nullable() const noexcept { return m_nullable; }
    /**
     * @brief Gets the named child properties — only actually populated for an Object-kind
     * schema, everything else just hands back an empty map.
     * @return every property, keyed by name.
     */
    [[nodiscard]] const std::unordered_map<std::string, SchemaType> &
    get_properties() const noexcept {
        return m_properties;
    }
    /**
     * @brief Gets the array element schema.
     * @warning Call this on anything but Array-kind and it's straight cooked — `m_items` only
     * ever gets set by set_items()/parse() for Array, so every other kind means a null shared_ptr
     * dereference. UB with no safety net. Don't be that guy.
     * @return the array element's schema.
     */
    [[nodiscard]] const SchemaType &get_items() const noexcept { return *m_items; }

  private:
    // Bare schema name from a "#/components/schemas/<Name>" JSON Pointer.
    /**
     * @brief Rips the bare schema name out of a "#/components/schemas/<Name>" JSON Pointer —
     * no need for the whole path, just the part that matters.
     * @param pointer the `$ref` pointer string.
     * @return everything after the last '/', or the whole string if there's no '/'.
     */
    [[nodiscard]] static std::string get_ref_name(std::string_view pointer) {
        auto pos = pointer.rfind('/');
        return pos == std::string_view::npos ? std::string{pointer}
                                             : std::string{pointer.substr(pos + 1)};
    }

    /**
     * @brief Maps an OpenAPI `type` string over to the matching SchemaKind — the whole enum
     * lives or dies on this lookup landing right.
     * @param type the schema's `type` value (e.g. "object", "integer").
     * @return the matching SchemaKind, or an L in string form if `type` isn't a name we recognize.
     */
    [[nodiscard]] static std::expected<SchemaKind, std::string> resolve_kind(std::string_view type) {
        if (type == "object") {
            return SchemaKind::OBJECT;
        }
        if (type == "array") {
            return SchemaKind::ARRAY;
        }
        if (type == "string") {
            return SchemaKind::STRING;
        }
        if (type == "integer") {
            return SchemaKind::INTEGER;
        }
        if (type == "number") {
            return SchemaKind::NUMBER;
        }
        if (type == "boolean") {
            return SchemaKind::BOOLEAN;
        }
        return std::unexpected{std::format("unknown schema type '{}'", type)};
    }

    SchemaKind m_kind{SchemaKind::STRING};
    std::string m_ref;
    bool m_nullable{false};
    std::unordered_map<std::string, SchemaType> m_properties;
    std::shared_ptr<SchemaType> m_items;
};

} // namespace congelado::client

export template <>
struct std::formatter<congelado::client::SchemaKind> {
    /**
     * @brief No format-spec support here, lowkey by design — SchemaKind only ever formats as
     * its plain name, so parsing just accepts an empty spec and bails immediately.
     * @param ctx the format parse context.
     * @return iterator to the start of the (expected-empty) format spec.
     */
    static constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

    /**
     * @brief Writes the enumerator's plain name (e.g. "Object", "Array") straight out to the
     * format context — that's the whole trick.
     * @tparam FormatContext the format context type, deduced by `std::format`.
     * @param kind the SchemaKind to format.
     * @param ctx the format context to write into.
     * @return output iterator past the written name.
     */
    template <typename FormatContext>
    auto format(congelado::client::SchemaKind kind, FormatContext &ctx) const {
        using enum congelado::client::SchemaKind;
        std::string_view name;
        switch (kind) {
        case OBJECT: {
            name = "Object";
            break;
        }
        case ARRAY: {
            name = "Array";
            break;
        }
        case STRING: {
            name = "String";
            break;
        }
        case INTEGER: {
            name = "Integer";
            break;
        }
        case NUMBER: {
            name = "Number";
            break;
        }
        case BOOLEAN: {
            name = "Boolean";
            break;
        }
        case REF: {
            name = "Ref";
            break;
        }
        }
        return std::format_to(ctx.out(), "{}", name);
    }
};

#ifdef CONGELADO_TEST
namespace openapi_gen_schema_model_tests {
using namespace boost::ut;
using congelado::client::SchemaKind;
using congelado::client::SchemaType;

// Builds a schema-shaped serde::Value directly (bypassing JSON text parsing entirely — no
// format plugin is registered in this isolated test target, see the module-preamble note
// above for why). Mirrors the rfl::Generic::Object construction pattern
// plugins/serde/json/bin/json_plugin.cc's own tests already use.
[[nodiscard]] serde::Value primitive_schema(const std::string &type) {
    serde::Value::Object object;
    object.insert(std::string{"type"}, serde::Value{type});
    return serde::Value{object};
}

suite<"SchemaType"> schema_type_suite = [] {
    "default state is String-kind, non-nullable, no ref, no properties"_test = [] {
        SchemaType schema;

        expect(schema.get_kind() == SchemaKind::STRING);
        expect(not schema.get_nullable());
        expect(schema.get_ref().empty());
        expect(schema.get_properties().empty());
    };

    "set_kind/get_kind round-trip for every kind"_test = [] {
        SchemaType schema;
        schema.set_kind(SchemaKind::OBJECT);
        expect(schema.get_kind() == SchemaKind::OBJECT);
        schema.set_kind(SchemaKind::ARRAY);
        expect(schema.get_kind() == SchemaKind::ARRAY);
        schema.set_kind(SchemaKind::REF);
        expect(schema.get_kind() == SchemaKind::REF);
    };

    "set_ref/get_ref round-trip"_test = [] {
        SchemaType schema;
        schema.set_ref("TaskDef");
        expect(schema.get_ref() == "TaskDef");
    };

    "set_nullable/get_nullable round-trip"_test = [] {
        SchemaType schema;
        schema.set_nullable(true);
        expect(schema.get_nullable());
    };

    "add_property populates get_properties, keyed by name"_test = [] {
        SchemaType schema;
        SchemaType child_a;
        child_a.set_kind(SchemaKind::STRING);
        SchemaType child_b;
        child_b.set_kind(SchemaKind::INTEGER);
        schema.add_property("a", child_a);
        schema.add_property("b", child_b);

        expect(schema.get_properties().size() == 2);
        expect(schema.get_properties().contains("a"));
        expect(schema.get_properties().at("a").get_kind() == SchemaKind::STRING);
        expect(schema.get_properties().at("b").get_kind() == SchemaKind::INTEGER);
    };

    "set_items/get_items round-trip"_test = [] {
        SchemaType schema;
        SchemaType items;
        items.set_kind(SchemaKind::STRING);
        schema.set_items(items);

        expect(schema.get_items().get_kind() == SchemaKind::STRING);
    };

    "parse: string type yields String kind, non-nullable"_test = [] {
        SchemaType schema;
        auto result = schema.parse(primitive_schema("string"));

        expect(result.has_value());
        expect(schema.get_kind() == SchemaKind::STRING);
        expect(not schema.get_nullable());
    };

    "parse: integer/number/boolean types map correctly"_test = [] {
        SchemaType int_schema;
        [[maybe_unused]] auto r1 = int_schema.parse(primitive_schema("integer"));
        expect(int_schema.get_kind() == SchemaKind::INTEGER);

        SchemaType num_schema;
        [[maybe_unused]] auto r2 = num_schema.parse(primitive_schema("number"));
        expect(num_schema.get_kind() == SchemaKind::NUMBER);

        SchemaType bool_schema;
        [[maybe_unused]] auto r3 = bool_schema.parse(primitive_schema("boolean"));
        expect(bool_schema.get_kind() == SchemaKind::BOOLEAN);
    };

    "parse: object with properties recurses into each one"_test = [] {
        serde::Value::Object name_prop;
        name_prop.insert(std::string{"type"}, serde::Value{std::string{"string"}});
        serde::Value::Object age_prop;
        age_prop.insert(std::string{"type"}, serde::Value{std::string{"integer"}});
        serde::Value::Object properties;
        properties.insert(std::string{"name"}, serde::Value{name_prop});
        properties.insert(std::string{"age"}, serde::Value{age_prop});
        serde::Value::Object object;
        object.insert(std::string{"type"}, serde::Value{std::string{"object"}});
        object.insert(std::string{"properties"}, serde::Value{properties});

        SchemaType schema;
        auto result = schema.parse(serde::Value{object});

        expect(result.has_value()) << fatal;
        expect(schema.get_kind() == SchemaKind::OBJECT);
        expect(schema.get_properties().size() == 2);
        expect(schema.get_properties().at("name").get_kind() == SchemaKind::STRING);
        expect(schema.get_properties().at("age").get_kind() == SchemaKind::INTEGER);
    };

    "parse: object with no properties key yields an empty property map"_test = [] {
        SchemaType schema;
        auto result = schema.parse(primitive_schema("object"));

        expect(result.has_value());
        expect(schema.get_kind() == SchemaKind::OBJECT);
        expect(schema.get_properties().empty());
    };

    "parse: array with items recurses into the element schema"_test = [] {
        serde::Value::Object items;
        items.insert(std::string{"type"}, serde::Value{std::string{"string"}});
        serde::Value::Object array_object;
        array_object.insert(std::string{"type"}, serde::Value{std::string{"array"}});
        array_object.insert(std::string{"items"}, serde::Value{items});

        SchemaType schema;
        auto result = schema.parse(serde::Value{array_object});

        expect(result.has_value()) << fatal;
        expect(schema.get_kind() == SchemaKind::ARRAY);
        expect(schema.get_items().get_kind() == SchemaKind::STRING);
    };

    "parse: nullable:true is honored regardless of type"_test = [] {
        serde::Value::Object object;
        object.insert(std::string{"type"}, serde::Value{std::string{"string"}});
        object.insert(std::string{"nullable"}, serde::Value{true});

        SchemaType schema;
        [[maybe_unused]] auto result = schema.parse(serde::Value{object});

        expect(schema.get_nullable());
    };

    "parse: $ref with a components-schemas pointer extracts the bare name"_test = [] {
        serde::Value::Object object;
        object.insert(std::string{"$ref"}, serde::Value{std::string{"#/components/schemas/TaskDef"}});

        SchemaType schema;
        auto result = schema.parse(serde::Value{object});

        expect(result.has_value()) << fatal;
        expect(schema.get_kind() == SchemaKind::REF);
        expect(schema.get_ref() == "TaskDef");
    };

    "parse: $ref with no slash uses the whole string as the name"_test = [] {
        serde::Value::Object object;
        object.insert(std::string{"$ref"}, serde::Value{std::string{"TaskDef"}});

        SchemaType schema;
        [[maybe_unused]] auto result = schema.parse(serde::Value{object});

        expect(schema.get_kind() == SchemaKind::REF);
        expect(schema.get_ref() == "TaskDef");
    };

    "parse: an empty $ref string is treated as absent, falls through to 'type'"_test = [] {
        serde::Value::Object object;
        object.insert(std::string{"$ref"}, serde::Value{std::string{""}});
        object.insert(std::string{"type"}, serde::Value{std::string{"string"}});

        SchemaType schema;
        auto result = schema.parse(serde::Value{object});

        expect(result.has_value()) << fatal;
        expect(schema.get_kind() == SchemaKind::STRING);
    };

    "parse: missing 'type' (and no $ref) is an error"_test = [] {
        serde::Value::Object object;
        SchemaType schema;

        auto result = schema.parse(serde::Value{object});

        expect(not result.has_value()) << fatal;
        expect(result.error() == "schema missing 'type'");
    };

    "parse: non-string 'type' is an error"_test = [] {
        serde::Value::Object object;
        object.insert(std::string{"type"}, serde::Value{std::int64_t{1}});
        SchemaType schema;

        auto result = schema.parse(serde::Value{object});

        expect(not result.has_value()) << fatal;
        expect(result.error() == "schema 'type' must be a string");
    };

    "parse: unrecognized 'type' string is an error naming it"_test = [] {
        SchemaType schema;
        auto result = schema.parse(primitive_schema("frobnicator"));

        expect(not result.has_value()) << fatal;
        expect(result.error() == "unknown schema type 'frobnicator'");
    };

    "parse: array missing 'items' is an error"_test = [] {
        SchemaType schema;
        auto result = schema.parse(primitive_schema("array"));

        expect(not result.has_value()) << fatal;
        expect(result.error() == "array schema missing 'items'");
    };

    "parse: a nested property's parse failure propagates verbatim"_test = [] {
        serde::Value::Object bad_prop; // no 'type' key
        serde::Value::Object properties;
        properties.insert(std::string{"broken"}, serde::Value{bad_prop});
        serde::Value::Object object;
        object.insert(std::string{"type"}, serde::Value{std::string{"object"}});
        object.insert(std::string{"properties"}, serde::Value{properties});

        SchemaType schema;
        auto result = schema.parse(serde::Value{object});

        expect(not result.has_value()) << fatal;
        expect(result.error() == "schema missing 'type'");
    };

    "parse: an array item's parse failure propagates verbatim"_test = [] {
        serde::Value::Object bad_items; // no 'type' key
        serde::Value::Object object;
        object.insert(std::string{"type"}, serde::Value{std::string{"array"}});
        object.insert(std::string{"items"}, serde::Value{bad_items});

        SchemaType schema;
        auto result = schema.parse(serde::Value{object});

        expect(not result.has_value()) << fatal;
        expect(result.error() == "schema missing 'type'");
    };
};

suite<"SchemaKind formatter"> schema_kind_formatter_suite = [] {
    "formats every enumerator by its plain name"_test = [] {
        expect(std::format("{}", SchemaKind::OBJECT) == "Object");
        expect(std::format("{}", SchemaKind::ARRAY) == "Array");
        expect(std::format("{}", SchemaKind::STRING) == "String");
        expect(std::format("{}", SchemaKind::INTEGER) == "Integer");
        expect(std::format("{}", SchemaKind::NUMBER) == "Number");
        expect(std::format("{}", SchemaKind::BOOLEAN) == "Boolean");
        expect(std::format("{}", SchemaKind::REF) == "Ref");
    };
};

} // namespace openapi_gen_schema_model_tests
#endif
