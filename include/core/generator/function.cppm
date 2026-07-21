export module core_generator:function;

import std;
import :statement;

export namespace core::generator {

// Renders either as a full block-style function/method (own line, braced body, blank line
// after — used for free functions) or, with set_inline(), as a single-line body (used for
// the terse setter/getter accessor idiom: "void setX(T value) { m_x = ...; }").
class Function {
  public:
    /**
     * @brief Starts building a generated function/method with the given return type and
     * name — no params, no body, no modifiers yet.
     * @param returnType the return type, emitted verbatim (e.g. `"void"`,
     * `"const std::string &"`).
     * @param name the function's name, emitted verbatim.
     */
    Function(std::string returnType, std::string name)
        : m_return_type(std::move(returnType)), m_name(std::move(name)) {}

    /**
     * @brief Appends one parameter to the end of the parameter list, in call order.
     * @param param the parameter to append.
     * @return reference to this function for chaining.
     */
    Function &add_param(Param param) {
        m_params.push_back(std::move(param));
        return *this;
    }
    /**
     * @brief Appends one statement to the function body, in emission order.
     * @param statement the statement to append.
     * @return reference to this function for chaining.
     */
    Function &add_statement(Stmt statement) {
        m_statements.push_back(std::move(statement));
        return *this;
    }
    /**
     * @brief Toggles the `static` keyword on the rendered declaration.
     * @param value true to render `static`, false to drop it. Defaults to true so a bare
     * `.set_static()` call flips it on — that's the motion.
     * @return reference to this function for chaining.
     */
    Function &set_static(bool value = true) noexcept {
        m_is_static = value;
        return *this;
    }
    /**
     * @brief Toggles the trailing `const` qualifier on the rendered declaration.
     * @param value true to render `const`, false to drop it.
     * @return reference to this function for chaining.
     */
    Function &set_const(bool value = true) noexcept {
        m_is_const = value;
        return *this;
    }
    /**
     * @brief Toggles the trailing `noexcept` qualifier on the rendered declaration.
     * @param value true to render `noexcept`, false to drop it.
     * @return reference to this function for chaining.
     */
    Function &set_noexcept(bool value = true) noexcept {
        m_is_noexcept = value;
        return *this;
    }
    /**
     * @brief Toggles the leading `[[nodiscard]]` attribute on the rendered declaration.
     * @param value true to render `[[nodiscard]]`, false to drop it.
     * @return reference to this function for chaining.
     */
    Function &set_nodiscard(bool value = true) noexcept {
        m_is_nodiscard = value;
        return *this;
    }
    /**
     * @brief Switches rendering between full block-style (own braces, blank line after) and
     * single-line inline body — the terse getter/setter idiom this codebase leans on hard:
     * `void setX(T value) { m_x = ...; }`.
     * @param value true for single-line inline rendering, false for block-style. Lowkey the
     * whole reason this class exists is to spit out that exact accessor shape.
     * @return reference to this function for chaining.
     */
    Function &set_inline(bool value = true) noexcept {
        m_is_inline = value;
        return *this;
    }

    /**
     * @brief Gets the function's name.
     * @return the name this function renders as.
     */
    [[nodiscard]] const std::string &get_name() const noexcept { return m_name; }
    /**
     * @brief Gets the function's return type.
     * @return the return type this function renders as, verbatim as it was handed in.
     */
    [[nodiscard]] const std::string &get_return_type() const noexcept { return m_return_type; }
    /**
     * @brief Checks whether the trailing `const` qualifier is set.
     * @return true if `set_const()` left this function marked const.
     */
    [[nodiscard]] bool is_const() const noexcept { return m_is_const; }

    /**
     * @brief Renders the full function/method: attributes, qualifiers, params, and body, in
     * either block or inline form depending on `set_inline()`.
     * @param indent number of leading spaces for the declaration line (and, in block form,
     * the base indent statements nest under).
     * @warning Inline mode joins every statement's `render(0)` output with a single space,
     * stripping only a trailing newline off each — so it's only safe for one-liner
     * statement kinds like `Expr`/`Return`/`Raw`. Feed it a multi-line `Stmt` (an `If`/`For`
     * with a body) and it's straight cooked: the inner lines get mashed onto one line with
     * no separators, silently producing invalid C++ with no error or warning.
     * @return the rendered declaration plus body, always ending in at least one trailing
     * newline (block form ends in a blank line).
     */
    [[nodiscard]] std::string render(std::size_t indent) const {
        std::string pad(indent, ' ');
        // Join every param's own render() with ", " — comma only goes between entries,
        // never trailing, so the loop tracks index instead of just appending blindly.
        std::string params;
        for (std::size_t i = 0; i < m_params.size(); ++i) {
            if (i > 0) {
                params += ", ";
            }
            params += m_params[i].render();  // FIXME(clang-tidy): unchecked operator[], consider .at()
        }

        // Build the declaration line: leading attributes/qualifiers first, in the order
        // real C++ expects them ([[nodiscard]] before static, both before the signature).
        std::string decl = pad;
        if (m_is_nodiscard) {
            decl += "[[nodiscard]] ";
        }
        if (m_is_static) {
            decl += "static ";
        }
        // Reference/pointer return types already end in their own spacing char, so skip
        // the extra space before the name — otherwise you'd get "T & name" instead of "T &name".
        char last = m_return_type.empty() ? ' ' : m_return_type.back();
        std::string_view separator = (last == '&' || last == '*') ? "" : " ";
        decl += std::format("{}{}{}({})", m_return_type, separator, m_name, params);
        // Trailing qualifiers come after the param list, const before noexcept.
        if (m_is_const) {
            decl += " const";
        }
        if (m_is_noexcept) {
            decl += " noexcept";
        }

        // Inline mode: squash every statement onto one line for the terse
        // getter/setter idiom — strip each statement's trailing newline and glue
        // them with single spaces instead. Only safe for one-liner statement kinds,
        // see the @warning above.
        if (m_is_inline) {
            std::string body;
            for (const auto &statement : m_statements) {
                auto line = statement.render(0);
                if (!line.empty() && line.back() == '\n') {
                    line.pop_back();
                }
                body += line;
                body += " ";
            }
            if (!body.empty() && body.back() == ' ') {
                body.pop_back();
            }
            return std::format("{} {{ {} }}\n", decl, body);
        }

        // Block mode: own braces, each statement rendered on its own line one indent
        // level deeper, blank line after the closing brace to separate from whatever
        // follows.
        std::string out = decl + " {\n";
        for (const auto &statement : m_statements) {
            out += statement.render(indent + 4);
        }
        out += pad + "}\n\n";
        return out;
    }

  private:
    std::string m_return_type;
    std::string m_name;
    std::vector<Param> m_params;
    std::vector<Stmt> m_statements;
    bool m_is_static{false};
    bool m_is_const{false};
    bool m_is_noexcept{false};
    bool m_is_nodiscard{false};
    bool m_is_inline{false};
};

} // namespace core::generator
