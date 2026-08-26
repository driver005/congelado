export module core_generator:statement;

import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::generator {

class Param
{
public:
    /**
     * @brief Builds a single generated function parameter (type + name, optional default
     * value).
     * @param type the parameter's C++ type, emitted verbatim.
     * @param name the parameter's name, emitted verbatim.
     */
    Param(std::string type, std::string name) :
        m_type(std::move(type)),
        m_name(std::move(name))
    {
    }

    /**
     * @brief Attaches a default value to the parameter, e.g. `bool flag = true`.
     * @param value the default-value expression, emitted verbatim after ` = `.
     * @return reference to this parameter for chaining.
     */
    Param& set_default_value(std::string&& value)
    {
        m_default_value = std::move(value);
        return *this;
    }

    /**
     * @brief Gets the parameter's type.
     * @return the type this parameter renders as, verbatim as it was handed in.
     */
    [[nodiscard]] const std::string& get_type() const noexcept
    {
        return m_type;
    }

    /**
     * @brief Gets the parameter's name.
     * @return the name this parameter renders as.
     */
    [[nodiscard]] const std::string& get_name() const noexcept
    {
        return m_name;
    }

    /**
     * @brief Gets the parameter's default value, if one was set.
     * @return the default-value expression, or `std::nullopt` if `set_default_value()` was
     * never called.
     */
    [[nodiscard]] const std::optional<std::string>& get_default_value() const noexcept
    {
        return m_default_value;
    }

    /**
     * @brief Renders this parameter as it appears inside a function's parameter list.
     * @return the parameter text, e.g. `"bool flag = true"` or `"const std::string &name"`
     * — no cap, no trailing comma or newline, joining those onto neighboring params is the
     * caller's job.
     */
    [[nodiscard]] std::string render() const
    {
        // Same reference/pointer spacing trick as Function::render() — a type already
        // ending in `&`/`*` doesn't need an extra space before the param name.
        char last = m_type.empty() ? ' ' : m_type.back();
        std::string_view separator = (last == '&' || last == '*') ? "" : " ";
        std::string decl = m_type + std::string{separator} + m_name;
        // Tack on ` = <value>` only if a default was actually set.
        return m_default_value ? std::format("{} = {}", decl, *m_default_value) : decl;
    }

private:
    std::string m_type;
    std::string m_name;
    std::optional<std::string> m_default_value;
};

// A single generated statement. Kept as one tagged class (rather than a class hierarchy)
// since every kind renders to one contiguous block of text and none needs virtual dispatch.
// Raw is a verbatim escape hatch: it renders exactly the text handed to it, with no added
// indentation or trailing newline, for callers that already have an exact pre-formatted
// line/block to emit unchanged.
class Stmt
{
public:
    /**
     * @brief Builds a verbatim escape-hatch statement — renders exactly the text handed in,
     * with no added indentation or trailing newline.
     * @param text the exact pre-formatted line/block to emit unchanged.
     * @return the constructed raw statement.
     * @note Use this when a caller already has exact output ready and the structured Kinds
     * below (`expr`/`var_decl`/`if_stmt`/...) would just get in the way — bet.
     */
    [[nodiscard]] static Stmt raw(std::string text)
    {
        Stmt stmt;
        stmt.m_kind = Kind::RAW;
        stmt.m_text = std::move(text);
        return stmt;
    }

    /**
     * @brief Builds a bare expression statement, e.g. `m_callback();`.
     * @param expression the expression text, emitted verbatim before the added `;`.
     * @return the constructed expression statement.
     */
    [[nodiscard]] static Stmt expr(std::string expression)
    {
        Stmt stmt;
        stmt.m_kind = Kind::EXPR;
        stmt.m_text = std::move(expression);
        return stmt;
    }

    /**
     * @brief Builds an `auto <name> = <initExpr>;` variable declaration statement.
     * @param name the declared variable's name, emitted verbatim.
     * @param initExpr the initializer expression, emitted verbatim.
     * @return the constructed variable-declaration statement.
     */
    [[nodiscard]] static Stmt var_decl(std::string name, std::string initExpr)
    {
        Stmt stmt;
        stmt.m_kind = Kind::VAR_DECL;
        stmt.m_name = std::move(name);
        stmt.m_text = std::move(initExpr);
        return stmt;
    }

    /**
     * @brief Builds a `return;` or `return <expression>;` statement.
     * @param expression the returned expression, or empty for a bare `return;`.
     * @return the constructed return statement.
     */
    [[nodiscard]] static Stmt return_stmt(std::string expression = "")
    {
        Stmt stmt;
        stmt.m_kind = Kind::RETURN;
        stmt.m_text = std::move(expression);
        return stmt;
    }

    /**
     * @brief Builds an `if (<condition>) { ... }` statement with no `else` branch.
     * @param condition the condition expression, emitted verbatim inside the parens.
     * @param thenBody the statements to render inside the `if` block, in order.
     * @return the constructed if-statement.
     */
    [[nodiscard]] static Stmt if_stmt(std::string condition, std::vector<Stmt> thenBody)
    {
        Stmt stmt;
        stmt.m_kind = Kind::IF;
        stmt.m_text = std::move(condition);
        stmt.m_body = std::move(thenBody);
        return stmt;
    }

    /**
     * @brief Builds a classic three-clause `for (<init>; <condition>; <step>) { ... }`
     * statement.
     * @param init the loop's init clause, emitted verbatim (no trailing `;` needed).
     * @param condition the loop's condition clause, emitted verbatim.
     * @param step the loop's step clause, emitted verbatim.
     * @param body the statements to render inside the loop body, in order.
     * @note Stashes `init` in `m_name` — the same field `var_decl()` uses for a declared
     * variable's name. Harmless today since `render()` only reads the field(s) matching
     * `m_kind`, but it's a landmine for whoever adds a new `Kind` later without checking
     * which fields are already shared between the existing ones.
     * @return the constructed for-statement.
     */
    [[nodiscard]] static Stmt
    for_stmt(std::string init, std::string condition, std::string step, std::vector<Stmt> body)
    {
        Stmt stmt;
        stmt.m_kind = Kind::FOR;
        stmt.m_name = std::move(init);
        stmt.m_text = std::move(condition);
        stmt.m_step = std::move(step);
        stmt.m_body = std::move(body);
        return stmt;
    }

    /**
     * @brief Renders this statement to source text, dispatching on its `Kind`.
     * @param indent number of leading spaces for this statement's line (and, for `If`/`For`,
     * the base indent their bodies nest 4 further under).
     * @warning `Raw` ignores `indent` entirely and returns its text completely unchanged —
     * no padding, no trailing newline unless the caller baked one in. Every other Kind pads
     * and newline-terminates. Mixing a `Raw` statement into a body list next to the others
     * without matching whitespace yourself is how you get lofi-looking generated output
     * that still compiles but reads cooked.
     * @return the rendered statement text.
     */
    [[nodiscard]] std::string render(std::size_t indent) const
    {
        std::string pad(indent, ' ');
        // Dispatch on which factory built this Stmt — each Kind only reads the
        // fields its own factory actually populated, see the for_stmt() @note above.
        switch (m_kind) {
            case Kind::RAW:
                // Raw skips the pad entirely and returns the stashed text unchanged —
                // by design, per the class-level @warning.
                return m_text;
            case Kind::EXPR:
                return std::format("{}{};\n", pad, m_text);
            case Kind::VAR_DECL:
                return std::format("{}auto {} = {};\n", pad, m_name, m_text);
            case Kind::RETURN:
                // Bare `return;` when no expression was given, otherwise `return <expr>;`.
                return m_text.empty() ? std::format("{}return;\n", pad)
                                      : std::format("{}return {};\n", pad, m_text);
            case Kind::IF:
                {
                    // Recurse into the body one indent level deeper, then close the brace
                    // back at this statement's own indent.
                    std::string out = std::format("{}if ({}) {{\n", pad, m_text);
                    for (const auto& child: m_body) {
                        out += child.render(indent + 4);
                    }
                    out += std::format("{}}}\n", pad);
                    return out;
                }
            case Kind::FOR:
                {
                    // Same nested-body pattern as If, plus the three-clause header built
                    // from init/condition/step.
                    std::string out =
                        std::format("{}for ({}; {}; {}) {{\n", pad, m_name, m_text, m_step);
                    for (const auto& child: m_body) {
                        out += child.render(indent + 4);
                    }
                    out += std::format("{}}}\n", pad);
                    return out;
                }
        }
        return "";
    }

private:
    /**
     * @brief Default-constructs an empty raw statement — private because every real
     * instance must come through one of the named static factories (`raw()`/`expr()`/
     * `var_decl()`/...), which set `m_kind` and the fields that Kind actually reads.
     */
    Stmt() = default;

    enum class Kind : std::uint8_t
    {
        RAW,
        EXPR,
        VAR_DECL,
        RETURN,
        IF,
        FOR
    };

    Kind m_kind{Kind::RAW};
    std::string m_name;
    std::string m_text;
    std::string m_step;
    std::vector<Stmt> m_body;
};

} // namespace core::generator

#ifdef CONGELADO_TEST
namespace core::generator::tests {
using namespace boost::ut;

suite<"Param"> param_suite = [] {
    "renders type and name without a default value"_test = [] {
        Param param{"int", "count"};

        expect(param.get_type() == "int");
        expect(param.get_name() == "count");
        expect(not param.get_default_value().has_value());
        expect(param.render() == "int count");
    };

    "renders a default value appended after ' = '"_test = [] {
        Param param{"bool", "flag"};
        param.set_default_value("true");

        expect(param.get_default_value().value() == "true");
        expect(param.render() == "bool flag = true");
    };

    "skips the extra space for a reference type"_test = [] {
        Param param{"const std::string &", "name"};

        expect(param.render() == "const std::string &name");
    };

    "skips the extra space for a pointer type"_test = [] {
        Param param{"int *", "value"};

        expect(param.render() == "int *value");
    };
};

suite<"Stmt"> stmt_suite = [] {
    "raw ignores indent and returns the text unchanged"_test = [] {
        auto stmt = Stmt::raw("// verbatim\n");

        expect(stmt.render(8) == "// verbatim\n");
    };

    "expr renders a padded, semicolon-terminated line"_test = [] {
        auto stmt = Stmt::expr("m_callback()");

        expect(stmt.render(4) == "    m_callback();\n");
    };

    "var_decl renders an auto declaration"_test = [] {
        auto stmt = Stmt::var_decl("value", "compute()");

        expect(stmt.render(0) == "auto value = compute();\n");
    };

    "return_stmt with no expression renders a bare return"_test = [] {
        auto stmt = Stmt::return_stmt();

        expect(stmt.render(0) == "return;\n");
    };

    "return_stmt with an expression renders it"_test = [] {
        auto stmt = Stmt::return_stmt("42");

        expect(stmt.render(0) == "return 42;\n");
    };

    "if_stmt nests its body one indent level deeper"_test = [] {
        std::vector<Stmt> body;
        body.push_back(Stmt::return_stmt("1"));
        auto stmt = Stmt::if_stmt("x > 0", std::move(body));

        expect(stmt.render(0) == "if (x > 0) {\n    return 1;\n}\n");
    };

    "for_stmt renders the three-clause header and nested body"_test = [] {
        std::vector<Stmt> body;
        body.push_back(Stmt::expr("total += i"));
        auto stmt = Stmt::for_stmt("int i = 0", "i < 10", "++i", std::move(body));

        expect(stmt.render(0) == "for (int i = 0; i < 10; ++i) {\n    total += i;\n}\n");
    };
};

} // namespace core::generator::tests
#endif
