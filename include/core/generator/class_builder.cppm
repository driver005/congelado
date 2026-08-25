export module core_generator:class_builder;

import std;
import :function;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::generator {

class Field {
  public:
    /**
     * @brief Builds a single generated member-variable declaration (type + name, optional
     * default value).
     * @param type the C++ type spelled out verbatim as source text — no validation, so
     * whatever you hand it lands in the emitted class as-is.
     * @param name the field's name, also emitted verbatim.
     */
    Field(std::string type, std::string name) : m_type(std::move(type)), m_name(std::move(name)) {}

    /**
     * @brief Attaches a brace-init default value to the field, e.g. `int x{5};` instead of
     * `int x;` — bet, that's the difference between a footgun-prone uninitialized member
     * and one that's actually safe to read.
     * @param value the default-value expression, rendered verbatim inside `{}`.
     * @return reference to this field for chaining.
     */
    Field &setDefaultValue(std::string &&value) {  // NOLINT(readability-identifier-naming) — matches this project's get/set/add accessor naming convention (camelCase after prefix), not a real naming defect — the shared clang-tidy config has no accessor exception
        m_default_value = std::move(value);
        return *this;
    }

    /**
     * @brief Renders this field as one generated member-variable line, default value and all.
     * @return the declaration text, e.g. `"int m_count{0};\n"` or `"int m_count;\n"` —
     * always newline-terminated.
     */
    [[nodiscard]] std::string render() const {
        return m_default_value ? std::format("{} {}{{{}}};\n", m_type, m_name, *m_default_value)
                               : std::format("{} {};\n", m_type, m_name);
    }

  private:
    std::string m_type;
    std::string m_name;
    std::optional<std::string> m_default_value;
};

// Renders the mandatory project class layout: default constructor, then non-const
// (mutator/setter-style) methods, a blank line, then const (accessor/getter-style)
// methods, then a "private:" section with fields — regardless of how many methods
// fall into each group, matching every hand-written class in this codebase.
class Class {
  public:
    /**
     * @brief Starts a new generated class builder — no fields or methods yet.
     * @param name the class's name as it'll appear in `class <name> {`.
     */
    explicit Class(std::string name) : m_name(std::move(name)) {}

    /**
     * @brief Appends a new member-variable field to this class, in declaration order.
     * @param type the field's C++ type, emitted verbatim.
     * @param name the field's name, emitted verbatim.
     * @return reference to the newly added `Field` so callers can chain `setDefaultValue()`
     * onto it directly.
     */
    Field &addField(std::string type, std::string name) {  // NOLINT(readability-identifier-naming) — matches this project's get/set/add accessor naming convention (camelCase after prefix), not a real naming defect — the shared clang-tidy config has no accessor exception
        m_fields.emplace_back(std::move(type), std::move(name));
        return m_fields.back();
    }
    /**
     * @brief Appends a new method to this class. Lowkey the only thing that decides which
     * half of the generated class body it lands in is the added `Function`'s const-ness —
     * see `render()`.
     * @param returnType the method's return type, emitted verbatim.
     * @param name the method's name, emitted verbatim.
     * @return reference to the newly added `Function` so callers can chain `setConst()`,
     * `addParam()`, etc. onto it directly.
     */
    Function &addMethod(std::string returnType, std::string name) {  // NOLINT(readability-identifier-naming) — matches this project's get/set/add accessor naming convention (camelCase after prefix), not a real naming defect — the shared clang-tidy config has no accessor exception
        m_methods.emplace_back(std::move(returnType), std::move(name));
        return m_methods.back();
    }

    /**
     * @brief Gets the class's name.
     * @return the name this class renders as.
     */
    [[nodiscard]] const std::string &getName() const noexcept { return m_name; }  // NOLINT(readability-identifier-naming) — matches this project's get/set/add accessor naming convention (camelCase after prefix), not a real naming defect — the shared clang-tidy config has no accessor exception

    /**
     * @brief Renders the full generated class: default ctor, non-const methods, blank line,
     * const methods, blank line, `private:` fields.
     * @warning No cap — this always emits a `<Name>() = default;` constructor no matter
     * what's in `m_methods`. Add a method literally named the same as the class (i.e. try
     * to model a user-defined constructor) and the output has two constructor declarations,
     * which won't compile. Nothing here checks for that collision.
     * @return the complete `class { ... };` source text, blank-line-terminated.
     */
    [[nodiscard]] std::string render() const {
        // Kick off with the class header plus the mandatory default ctor — every
        // generated class in this codebase gets one, bet, no exceptions.
        std::string out =
            std::format("class {} {{\n  public:\n    {}() = default;\n\n", m_name, m_name);

        // First pass: non-const methods only — mutators/setters land right under the ctor.
        for (const auto &method : m_methods) {
            if (!method.is_const()) {
                out += method.render(4);
            }
        }
        out += "\n";
        // Second pass over the same method list, this time picking up only the const
        // ones — accessors/getters go after the blank-line split, matching the
        // hand-written class layout this generator is mimicking.
        for (const auto &method : m_methods) {
            if (method.is_const()) {
                out += method.render(4);
            }
        }
        // Wrap up with the private section — every field gets its own indented line.
        out += "\n  private:\n";
        for (const auto &field : m_fields) {
            out += "    " + field.render();
        }
        out += "};\n\n";
        return out;
    }

  private:
    std::string m_name;
    std::vector<Function> m_methods;
    std::vector<Field> m_fields;
};

} // namespace core::generator

#ifdef CONGELADO_TEST
namespace core::generator::tests {
using namespace boost::ut;

suite<"Field"> field_suite = [] {
    "renders without a default value"_test = [] {
        Field field{"int", "m_count"};

        expect(field.render() == "int m_count;\n");
    };

    "renders a brace-init default value"_test = [] {
        Field field{"int", "m_count"};
        field.setDefaultValue("0");

        expect(field.render() == "int m_count{0};\n");
    };
};

suite<"Class"> class_suite = [] {
    "getName returns the class's own name"_test = [] {
        Class cls{"Widget"};

        expect(cls.getName() == "Widget");
    };

    "render emits header, ctor, non-const methods before const, then fields"_test = [] {
        Class cls{"Widget"};
        cls.addMethod("void", "setX");
        cls.addMethod("int", "getX").set_const();
        cls.addField("int", "m_x").setDefaultValue("0");

        auto rendered = cls.render();

        expect(rendered.contains("class Widget {\n"));
        expect(rendered.contains("Widget() = default;"));
        expect(rendered.contains("private:\n    int m_x{0};\n"));

        auto setter_pos = rendered.find("setX");
        auto getter_pos = rendered.find("getX");
        expect(setter_pos != std::string::npos);
        expect(getter_pos != std::string::npos);
        expect(setter_pos < getter_pos);
    };

    "addField/addMethod return references usable for chaining"_test = [] {
        Class cls{"Widget"};
        cls.addField("int", "m_value").setDefaultValue("7");
        cls.addMethod("void", "run").set_static();

        expect(cls.render().contains("m_value{7}"));
        expect(cls.render().contains("static void run"));
    };
};

} // namespace core::generator::tests
#endif
