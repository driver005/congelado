export module openapi_generator_plugin:schema_model;

import std;
import serde;
import :document;

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
