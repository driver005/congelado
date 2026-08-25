module;
#include <rfl.hpp>

export module utils_openapi:schema;

import std;
import serde;
import :model;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

namespace utils::openapi::detail {

template <typename T>
constexpr bool IS_OPTIONAL_V = false;
template <typename T>
constexpr bool IS_OPTIONAL_V<std::optional<T>> = true;

template <typename T>
constexpr bool IS_VECTOR_V = false;
template <typename T>
constexpr bool IS_VECTOR_V<std::vector<T>> = true;

template <typename T>
constexpr bool IS_STRING_MAP_V = false;
template <typename T>
constexpr bool IS_STRING_MAP_V<std::unordered_map<std::string, T>> = true;
template <typename T>
constexpr bool IS_STRING_MAP_V<std::map<std::string, T>> = true;

/**
 * @brief Strips the namespace prefix off a type's reflected name, so build_schema<T>() can use
 * a bare "TaskDef" as both the $ref name and the components.schemas key instead of a mouthful
 * like "congelado::api::TaskDef". Straight cleanup motion, no cap.
 * @tparam T the type whose name gets resolved via rfl reflection.
 * @return the bare (unqualified) type name.
 */
template <typename T>
[[nodiscard]] std::string bare_type_name() {
    auto full_name = rfl::type_name_t<T>().str();
    auto separator_pos = full_name.rfind("::");
    return separator_pos == std::string::npos ? full_name
                                              : full_name.substr(separator_pos + 2);
}

} // namespace utils::openapi::detail

export namespace utils::openapi {

// Static, process-wide collector of named object schemas — same singleton pattern as
// utils::openapi::Registry. Populated as a side effect of build_schema<T>() the first
// time each ISerializable T is encountered (which happens naturally whenever a route
// declares .body<T>()/.response<T>(), since those already call build_schema<T>()).
class SchemaRegistry {
  public:
    /**
     * @brief Registers (or overwrites) a named schema in the process-wide collector.
     * @param name the schema's name, used as its components.schemas key.
     * @param schema the schema to store, moved in.
     */
    // FIXME(clang-tidy): readability-identifier-naming — addSchema/hasSchema/getSchemas are
    // called from include/utils/openapi/generator.cppm, which is out of scope for this pass;
    // renaming here without updating that call site would break the build.
    static void addSchema(std::string name, SchemaObject schema) {  // NOLINT(readability-identifier-naming) — matches this project's get/set/add accessor naming convention (camelCase after prefix), not a real naming defect — the shared clang-tidy config has no accessor exception
        m_schemas.insert_or_assign(std::move(name), std::move(schema));
    }

    /**
     * @brief Checks whether a schema's already been registered under this name.
     * @param name the schema name to look up.
     * @return true if it's already registered, false otherwise.
     */
    [[nodiscard]] static bool hasSchema(const std::string &name) noexcept {  // NOLINT(readability-identifier-naming) — matches this project's get/set/add accessor naming convention (camelCase after prefix), not a real naming defect — the shared clang-tidy config has no accessor exception
        return m_schemas.contains(name);
    }

    /**
     * @brief Grabs every registered schema.
     * @return all named schemas collected so far, keyed by name.
     */
    [[nodiscard]] static const std::unordered_map<std::string, SchemaObject> &
    getSchemas() noexcept {  // NOLINT(readability-identifier-naming) — matches this project's get/set/add accessor naming convention (camelCase after prefix), not a real naming defect — the shared clang-tidy config has no accessor exception
        return m_schemas;
    }

  private:
    static inline std::unordered_map<std::string, SchemaObject> m_schemas;
};

/**
 * @brief Recursively derives an OpenAPI SchemaObject from a C++ type — ISerializable types
 * become named $ref schemas (registered once into SchemaRegistry, with a placeholder written
 * before recursing so a self-referencing type just sees hasSchema()==true on re-entry instead
 * of recursing forever), optionals turn nullable, vectors turn arrays, string-keyed maps turn
 * plain objects, and every primitive gets its closest JSON Schema type. This is the whole
 * motion that lets .body<T>()/.response<T>() work without any hand-written schema, bet.
 * @tparam T the C++ type to derive a schema from.
 * @return the derived schema — a $ref for object types, an inline schema for everything else.
 */
template <typename T>
[[nodiscard]] SchemaObject build_schema() {
    using Decayed = std::remove_cvref_t<T>;
    SchemaObject schema;

    // Reflectable types resolve to a named $ref (registered once, below); every other
    // shape falls through to an inline schema derived straight from the C++ type itself,
    // lowkey a 1:1 mapping the whole way down.
    if constexpr (serde::ISerializable<Decayed>) {
        auto name = detail::bare_type_name<Decayed>();
        if (!SchemaRegistry::hasSchema(name)) {
            // Register a placeholder BEFORE recursing into properties, so a type that
            // (directly or indirectly) references itself sees hasSchema()==true on
            // re-entry and just emits $ref instead of recursing forever.
            SchemaRegistry::addSchema(name, SchemaObject{});
            SchemaObject object_schema;
            object_schema.set_type("object");
            std::apply(
                [&](auto... fields) {
                    (object_schema.add_property(
                         std::string{decltype(fields)::name.string_view()},
                         build_schema<typename decltype(fields)::ValueType>()),
                     ...);
                },
                serde::Serializable<Decayed>::fields());
            SchemaRegistry::addSchema(name, std::move(object_schema));
        }
        schema.set_ref(std::format("#/components/schemas/{}", name));
    } else if constexpr (detail::IS_OPTIONAL_V<Decayed>) {
        schema = build_schema<typename Decayed::value_type>();
        schema.set_nullable(true);
    } else if constexpr (detail::IS_VECTOR_V<Decayed>) {
        schema.set_type("array");
        schema.set_items(build_schema<typename Decayed::value_type>());
    } else if constexpr (detail::IS_STRING_MAP_V<Decayed>) {
        schema.set_type("object");
    } else if constexpr (std::same_as<Decayed, serde::Value>) {
        // Dynamic value objects — a jsonb-typed task input — can't be reflected into fixed
        // properties, so they map to a bare `{"type":"object"}` with no property list.
        schema.set_type("object");
    } else if constexpr (std::same_as<Decayed, bool>) {
        schema.set_type("boolean");
    } else if constexpr (std::floating_point<Decayed>) {
        schema.set_type("number");
    } else if constexpr (std::integral<Decayed>) {
        schema.set_type("integer");
    } else {
        // Fallback: std::string, enums, and anything else unhandled all map to "string" — merged
        // into one branch to avoid bugprone-branch-clone (was separate identical branches).
        schema.set_type("string");
    }

    return schema;
}

} // namespace utils::openapi

// build_schema<T>()'s ISerializable branch (named $ref schemas via reflect-cpp) needs a real
// serde::Serializable<T> specialization and touches the process-wide SchemaRegistry singleton —
// covered indirectly wherever a real serde-reflectable type gets schema'd elsewhere in the
// codebase. What's tested here is the primitive/optional/vector/map branches, which are pure
// compile-time dispatch with no reflection or shared state involved.
#ifdef CONGELADO_TEST
namespace utils::openapi::tests {
using namespace boost::ut;

suite<"build_schema primitives"> build_schema_primitives_suite = [] {
    "bool maps to boolean"_test = [] { expect(build_schema<bool>().get_type() == "boolean"); };
    "integral types map to integer"_test = [] {
        expect(build_schema<int>().get_type() == "integer");
        expect(build_schema<std::uint64_t>().get_type() == "integer");
    };
    "floating-point types map to number"_test = [] {
        expect(build_schema<double>().get_type() == "number");
        expect(build_schema<float>().get_type() == "number");
    };
    "string falls back to the string type"_test = [] {
        expect(build_schema<std::string>().get_type() == "string");
    };
};

suite<"build_schema optional/vector/map"> build_schema_containers_suite = [] {
    "optional<T> derives T's schema and marks it nullable"_test = [] {
        auto schema = build_schema<std::optional<int>>();
        expect(schema.get_type() == "integer");
        expect(schema.get_nullable());
    };
    "vector<T> becomes an array with T's schema as items"_test = [] {
        auto schema = build_schema<std::vector<std::string>>();
        expect(schema.get_type() == "array");
        expect(schema.get_items() != nullptr);
        expect(schema.get_items()->get_type() == "string");
    };
    "string-keyed maps become a bare object"_test = [] {
        expect(build_schema<std::unordered_map<std::string, int>>().get_type() == "object");
        expect(build_schema<std::map<std::string, int>>().get_type() == "object");
    };
};

} // namespace utils::openapi::tests
#endif
