export module openapi_generator_plugin:dto_writer;

import std;
import core_generator;
import :schema_model;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace congelado::client {

class DtoWriter
{
public:
    /**
     * @brief Turns the parsed OpenAPI named schemas into a full DTO module — topo-sorts them so
     * $refs come out declared before their users, emits a class with getters/setters per
     * property, then bolts on a serde::Serializable specialization per class. This is the W
     * that makes the generated DTOs actually (de)serialize.
     * @param namedSchemas every named schema pulled from `components.schemas`, keyed by name.
     * @param moduleName name of the module to emit (used in the module decl and in the
     * generated serde field-descriptor blocks).
     * @return generated C++ source for the DTO module, or an error string if a named schema
     * isn't an object (only object schemas can become classes).
     */
    [[nodiscard]] static std::expected<std::string, std::string> write(
        const std::unordered_map<std::string, SchemaType>& namedSchemas, std::string_view moduleName
    )
    {
        // Walk every named schema through topo_sort so refs always land before their users —
        // the visited map keeps shared deps from getting queued twice.
        std::vector<std::string> order;
        std::unordered_map<std::string, bool> visited;
        for (const auto& [name, schema]: namedSchemas) {
            (void)schema;
            topo_sort(name, namedSchemas, visited, order);
        }

        // Stand up the target module — std/serde imports plus one namespace to hold every DTO
        // class.
        core::generator::Generator generator{std::string{moduleName}};
        generator.addImport("std").addImport("serde");
        auto& ns = generator.addNamespace(std::string{moduleName});

        // First pass: emit one class per named schema, in the dependency order computed above.
        for (const auto& name: order) {
            const auto& schema = namedSchemas.at(name);
            // Only object schemas can become classes — anything else means the OpenAPI doc
            // named a primitive/array at the top level, which isn't a supported shape here.
            if (schema.get_kind() != SchemaKind::OBJECT) {
                return std::unexpected{std::format("named schema '{}' is not an object", name)};
            }

            auto& cls = ns.addClass(name);
            // Setters first, bet — every property gets a set<Name>(value) that moves straight
            // into the backing member.
            for (const auto& [prop_name, prop_type]: schema.get_properties()) {
                auto member = to_pascal_case(prop_name);
                auto type = resolve_cpp_type(prop_type);
                cls.addMethod("void", std::format("set{}", member))
                    .add_param(core::generator::Param{type, "value"})
                    .add_statement(
                        core::generator::Stmt::expr(
                            std::format("m_{} = std::move(value)", prop_name)
                        )
                    )
                    .set_inline();
            }
            // Then getters — const-ref return, noexcept, nodiscard, the whole accessor package.
            for (const auto& [prop_name, prop_type]: schema.get_properties()) {
                auto member = to_pascal_case(prop_name);
                auto type = resolve_cpp_type(prop_type);
                cls.addMethod(std::format("const {} &", type), std::format("get{}", member))
                    .add_statement(
                        core::generator::Stmt::return_stmt(std::format("m_{}", prop_name))
                    )
                    .set_const()
                    .set_noexcept()
                    .set_nodiscard()
                    .set_inline();
            }
            // Last, the backing fields themselves — one m_-prefixed member per property.
            for (const auto& [prop_name, prop_type]: schema.get_properties()) {
                cls.addField(resolve_cpp_type(prop_type), std::format("m_{}", prop_name));
            }
        }

        // Second pass, no cap: for every class just emitted, bolt on a serde::Serializable
        // specialization so it actually knows how to (de)serialize itself.
        for (const auto& name: order) {
            const auto& schema = namedSchemas.at(name);
            std::string block = std::format(
                "template <>\nstruct serde::Serializable<{}::{}> {{\n", moduleName, name
            );
            block += "    static constexpr auto fields() {\n";
            block += std::format("        using {}::{};\n", moduleName, name);
            block += "        return std::tuple{\n";
            // One FieldDesc per property, wiring the property name to its get/set pair.
            for (const auto& [prop_name, prop_type]: schema.get_properties()) {
                (void)prop_type;
                auto member = to_pascal_case(prop_name);
                block += std::format(
                    "            serde::FieldDesc<\"{}\", &{}::get{}, &{}::set{}>{{}},\n",
                    prop_name, name, member, name, member
                );
            }
            block += "        };\n    }\n};\n\n";
            generator.addRawBlock(std::move(block));
        }

        return generator.render();
    }

private:
    /**
     * @brief Turns a snake_case property name into PascalCase so it can be glued onto
     * "get"/"set" for the generated accessor names — no cap, straight naming-convention motion.
     * @param name property name as it appears in the schema (e.g. "task_name").
     * @return PascalCase form of the name (e.g. "TaskName").
     */
    [[nodiscard]] static std::string to_pascal_case(std::string_view name)
    {
        std::string result;
        bool capitalize_next = true;
        // Underscores just flip the "capitalize next" flag and get dropped — every other char
        // rides through as-is unless it's due for a capital.
        for (char character: name) {
            if (character == '_') {
                capitalize_next = true;
                continue;
            }
            result += capitalize_next ? static_cast<char>(std::toupper(character)) : character;
            capitalize_next = false;
        }
        return result;
    }

    /**
     * @brief Maps a parsed SchemaType to the C++ type string generated member fields and setter
     * params actually use — Ref becomes the bare class name, Array recurses into
     * `std::vector<...>`, nullable wraps the whole thing in `std::optional<...>`.
     * Straightforward motion, no funny business.
     * @param type schema node to resolve.
     * @return C++ type name as a string, ready to drop straight into generated code.
     */
    [[nodiscard]] static std::string resolve_cpp_type(const SchemaType& type)
    {
        // Resolve the base type first — Array recurses into its own element type, everything
        // else is a direct mapping.
        std::string base;
        switch (type.get_kind()) {
            case SchemaKind::REF:
                base = type.get_ref();
                break;
            case SchemaKind::ARRAY:
                base = std::format("std::vector<{}>", resolve_cpp_type(type.get_items()));
                break;
            case SchemaKind::STRING:
                base = "std::string";
                break;
            case SchemaKind::INTEGER:
                base = "std::int64_t";
                break;
            case SchemaKind::NUMBER:
                base = "double";
                break;
            case SchemaKind::BOOLEAN:
                base = "bool";
                break;
            case SchemaKind::OBJECT:
                base = "std::string"; // anonymous inline object — not expected from our own
                                      // server, see Task 1 note
                break;
        }
        // Then wrap in optional if the schema says null's on the table.
        return type.get_nullable() ? std::format("std::optional<{}>", base) : base;
    }

    // Collects every named schema a SchemaType depends on, following Ref both directly and
    // nested inside Array (e.g. a "tasks: array of TaskDef" property) — a plain top-level
    // Ref check misses the array case, which is a standard shape for list-style relationships.
    /**
     * @brief Walks a schema node and scoops up every named-schema dependency it $refs, whether
     * the ref sits right on the node or one level deep inside an Array's items — no missed
     * deps, that's the whole point.
     * @param type schema node to walk.
     * @param[out] out every $ref name found gets pushed onto this vector.
     */
    static void collect_refs(const SchemaType& type, std::vector<std::string>& out)
    {
        switch (type.get_kind()) {
            case SchemaKind::REF:
                out.push_back(type.get_ref());
                break;
            case SchemaKind::ARRAY:
                collect_refs(type.get_items(), out);
                break;
            default:
                break;
        }
    }

    // Post-order DFS so a schema is emitted after every named schema it $refs (directly or
    // via an array item).
    /**
     * @brief Post-order DFS that pushes `name` onto `order` only after every named schema it
     * depends on (directly or through an array item) has already landed — that's the whole
     * motion that guarantees a schema referencing another one always renders after it, no
     * forward-decl drama, no cap.
     * @param name schema currently being visited.
     * @param schemas every named schema, keyed by name, used to resolve refs.
     * @param[in,out] visited memo of names already visited, so shared deps aren't walked twice.
     * @param[out] order build-up of the resulting topological order.
     */
    static void topo_sort(
        const std::string& name,
        const std::unordered_map<std::string, SchemaType>& schemas,
        std::unordered_map<std::string, bool>& visited,
        std::vector<std::string>& order
    )
    {
        // Already walked this one — bail so shared deps don't get visited (or emitted) twice.
        if (visited[name]) {
            return;
        }
        visited[name] = true;
        const auto& schema = schemas.at(name);
        // Recurse into every named-schema dependency this schema's properties $ref before
        // this schema itself gets pushed — that's what guarantees post-order.
        for (const auto& [prop_name, prop_type]: schema.get_properties()) {
            (void)prop_name;
            std::vector<std::string> refs;
            collect_refs(prop_type, refs);
            for (const auto& ref: refs) {
                if (schemas.contains(ref)) {
                    topo_sort(ref, schemas, visited, order);
                }
            }
        }
        order.push_back(name);
    }
};

} // namespace congelado::client

#ifdef CONGELADO_TEST
namespace openapi_gen_dto_writer_tests {
using namespace boost::ut;
using congelado::client::DtoWriter;
using congelado::client::SchemaKind;
using congelado::client::SchemaType;

[[nodiscard]] SchemaType make_primitive(SchemaKind kind, bool nullable = false)
{
    SchemaType schema;
    schema.set_kind(kind);
    schema.set_nullable(nullable);
    return schema;
}

[[nodiscard]] SchemaType make_ref(std::string name)
{
    SchemaType schema;
    schema.set_kind(SchemaKind::REF);
    schema.set_ref(std::move(name));
    return schema;
}

[[nodiscard]] SchemaType make_array(SchemaType items)
{
    SchemaType schema;
    schema.set_kind(SchemaKind::ARRAY);
    schema.set_items(std::move(items));
    return schema;
}

suite<"DtoWriter"> dto_writer_suite = [] {
    "write emits a setter/getter/field per property, PascalCase-derived from snake_case"_test = [] {
        SchemaType widget;
        widget.set_kind(SchemaKind::OBJECT);
        widget.add_property("task_name", make_primitive(SchemaKind::STRING));

        std::unordered_map<std::string, SchemaType> schemas{{"Widget", widget}};
        auto result = DtoWriter::write(schemas, "dto_mod");

        expect(result.has_value()) << fatal;
        expect(result->contains("class Widget"));
        expect(result->contains("void setTaskName(std::string value)"));
        expect(result->contains("const std::string &getTaskName() const noexcept"));
        expect(result->contains("m_task_name"));
    };

    "to_pascal_case: a single-char segment name still capitalizes"_test = [] {
        SchemaType widget;
        widget.set_kind(SchemaKind::OBJECT);
        widget.add_property("id", make_primitive(SchemaKind::STRING));

        std::unordered_map<std::string, SchemaType> schemas{{"Widget", widget}};
        auto result = DtoWriter::write(schemas, "dto_mod");

        expect(result.has_value()) << fatal;
        expect(result->contains("setId("));
        expect(result->contains("getId("));
    };

    "to_pascal_case: an internal underscore run collapses to one capital each"_test = [] {
        SchemaType widget;
        widget.set_kind(SchemaKind::OBJECT);
        widget.add_property("a_b", make_primitive(SchemaKind::STRING));

        std::unordered_map<std::string, SchemaType> schemas{{"Widget", widget}};
        auto result = DtoWriter::write(schemas, "dto_mod");

        expect(result.has_value()) << fatal;
        expect(result->contains("setAB("));
        expect(result->contains("getAB("));
    };

    "to_pascal_case: a leading underscore is dropped, not preserved"_test = [] {
        SchemaType widget;
        widget.set_kind(SchemaKind::OBJECT);
        widget.add_property("_foo", make_primitive(SchemaKind::STRING));

        std::unordered_map<std::string, SchemaType> schemas{{"Widget", widget}};
        auto result = DtoWriter::write(schemas, "dto_mod");

        expect(result.has_value()) << fatal;
        expect(result->contains("setFoo("));
    };

    "resolve_cpp_type: Ref becomes the bare referenced class name"_test = [] {
        SchemaType widget;
        widget.set_kind(SchemaKind::OBJECT);
        widget.add_property("owner", make_ref("Person"));

        std::unordered_map<std::string, SchemaType> schemas{
            {"Widget", widget}, {"Person", SchemaType{}}
        };
        schemas.at("Person").set_kind(SchemaKind::OBJECT);
        auto result = DtoWriter::write(schemas, "dto_mod");

        expect(result.has_value()) << fatal;
        expect(result->contains("void setOwner(Person value)"));
    };

    "resolve_cpp_type: Array of Ref becomes std::vector<Ref>"_test = [] {
        SchemaType widget;
        widget.set_kind(SchemaKind::OBJECT);
        widget.add_property("items", make_array(make_ref("Item")));

        std::unordered_map<std::string, SchemaType> schemas{
            {"Widget", widget}, {"Item", SchemaType{}}
        };
        schemas.at("Item").set_kind(SchemaKind::OBJECT);
        auto result = DtoWriter::write(schemas, "dto_mod");

        expect(result.has_value()) << fatal;
        expect(result->contains("std::vector<Item>"));
    };

    "resolve_cpp_type: Integer/Number/Boolean map to int64_t/double/bool"_test = [] {
        SchemaType widget;
        widget.set_kind(SchemaKind::OBJECT);
        widget.add_property("count", make_primitive(SchemaKind::INTEGER));
        widget.add_property("ratio", make_primitive(SchemaKind::NUMBER));
        widget.add_property("active", make_primitive(SchemaKind::BOOLEAN));

        std::unordered_map<std::string, SchemaType> schemas{{"Widget", widget}};
        auto result = DtoWriter::write(schemas, "dto_mod");

        expect(result.has_value()) << fatal;
        expect(result->contains("std::int64_t value"));
        expect(result->contains("double value"));
        expect(result->contains("bool value"));
    };

    "resolve_cpp_type: nullable wraps the base type in std::optional"_test = [] {
        SchemaType widget;
        widget.set_kind(SchemaKind::OBJECT);
        widget.add_property("nickname", make_primitive(SchemaKind::STRING, true));

        std::unordered_map<std::string, SchemaType> schemas{{"Widget", widget}};
        auto result = DtoWriter::write(schemas, "dto_mod");

        expect(result.has_value()) << fatal;
        expect(result->contains("std::optional<std::string>"));
    };

    "resolve_cpp_type: Object kind is currently always std::string, even with properties"_test =
        [] {
            // Locks in resolve_cpp_type's current, unmodified behavior: an anonymous inline
            // OBJECT-kind property always resolves to "std::string", regardless of whether it
            // carries any properties of its own. Not a claim this is ideal — just the
            // documented, as-written behavior this pass must not change.
            SchemaType inline_object;
            inline_object.set_kind(SchemaKind::OBJECT);
            inline_object.add_property("nested", make_primitive(SchemaKind::STRING));

            SchemaType widget;
            widget.set_kind(SchemaKind::OBJECT);
            widget.add_property("blob", inline_object);

            std::unordered_map<std::string, SchemaType> schemas{{"Widget", widget}};
            auto result = DtoWriter::write(schemas, "dto_mod");

            expect(result.has_value()) << fatal;
            expect(result->contains("void setBlob(std::string value)"));
            expect(not result->contains("serde::Value"));
        };

    "write topo-sorts direct Ref dependencies before their dependents"_test = [] {
        SchemaType parent;
        parent.set_kind(SchemaKind::OBJECT);
        parent.add_property("child", make_ref("Child"));
        SchemaType child;
        child.set_kind(SchemaKind::OBJECT);
        child.add_property("value", make_primitive(SchemaKind::STRING));

        std::unordered_map<std::string, SchemaType> schemas{{"Parent", parent}, {"Child", child}};
        auto result = DtoWriter::write(schemas, "dto_mod");

        expect(result.has_value()) << fatal;
        auto child_pos = result->find("class Child");
        auto parent_pos = result->find("class Parent");
        expect(child_pos != std::string::npos);
        expect(parent_pos != std::string::npos);
        expect(child_pos < parent_pos);
    };

    "write topo-sorts array-of-Ref dependencies before their dependents"_test = [] {
        SchemaType parent;
        parent.set_kind(SchemaKind::OBJECT);
        parent.add_property("children", make_array(make_ref("Child")));
        SchemaType child;
        child.set_kind(SchemaKind::OBJECT);

        std::unordered_map<std::string, SchemaType> schemas{{"Parent", parent}, {"Child", child}};
        auto result = DtoWriter::write(schemas, "dto_mod");

        expect(result.has_value()) << fatal;
        auto child_pos = result->find("class Child");
        auto parent_pos = result->find("class Parent");
        expect(child_pos != std::string::npos);
        expect(parent_pos != std::string::npos);
        expect(child_pos < parent_pos);
    };

    "write errors when a named schema isn't an object"_test = [] {
        std::unordered_map<std::string, SchemaType> schemas{
            {"NotAnObject", make_primitive(SchemaKind::STRING)}
        };

        auto result = DtoWriter::write(schemas, "dto_mod");

        expect(not result.has_value()) << fatal;
        expect(result.error() == "named schema 'NotAnObject' is not an object");
    };

    "write emits a serde::Serializable specialization per class"_test = [] {
        SchemaType widget;
        widget.set_kind(SchemaKind::OBJECT);
        widget.add_property("name", make_primitive(SchemaKind::STRING));

        std::unordered_map<std::string, SchemaType> schemas{{"Widget", widget}};
        auto result = DtoWriter::write(schemas, "dto_mod");

        expect(result.has_value()) << fatal;
        expect(result->contains("struct serde::Serializable<dto_mod::Widget>"));
        expect(result->contains("serde::FieldDesc<\"name\", &Widget::getName, &Widget::setName>"));
    };

    "write on an empty schema map still renders a valid, empty module"_test = [] {
        std::unordered_map<std::string, SchemaType> schemas;

        auto result = DtoWriter::write(schemas, "dto_mod");

        expect(result.has_value()) << fatal;
        expect(result->starts_with("export module dto_mod;\n"));
        expect(not result->contains("class "));
    };
};

} // namespace openapi_gen_dto_writer_tests
#endif
