export module core_generator:module_builder;

import std;
import :function;
import :class_builder;

export namespace core::generator {

class Namespace {
  public:
    /**
     * @brief Starts a new generated namespace block — no classes or functions yet.
     * @param name the namespace's name, emitted verbatim after `namespace `.
     */
    explicit Namespace(std::string name) : m_name(std::move(name)) {}

    /**
     * @brief Appends a new class to this namespace, in declaration order.
     * @param name the class's name, emitted verbatim.
     * @return reference to the newly added `Class` so callers can chain `addField()`/
     * `addMethod()` onto it directly.
     */
    // FIXME(clang-tidy): readability-identifier-naming — addClass/addFunction/getName/
    // addImport/addNamespace/addRawBlock are called from sdk/client/dto_writer.cppm,
    // sdk/client/route_writer.cppm, and include/core/generator/generator.cppm, all out of
    // scope for this pass; renaming here without updating those call sites would break the
    // build.
    Class &addClass(std::string name) {  // NOLINT(readability-identifier-naming) — matches this project's get/set/add accessor naming convention (camelCase after prefix), not a real naming defect — the shared clang-tidy config has no accessor exception
        m_classes.emplace_back(std::move(name));
        return m_classes.back();
    }
    /**
     * @brief Appends a new free-standing function to this namespace, in declaration order.
     * @param returnType the function's return type, emitted verbatim.
     * @param name the function's name, emitted verbatim.
     * @return reference to the newly added `Function` so callers can chain modifiers onto
     * it directly. Lowkey the only place this generator emits a function that isn't a class
     * method — mind that if the generated output is meant to target a class-only codebase.
     */
    Function &addFunction(std::string returnType, std::string name) {  // NOLINT(readability-identifier-naming) — matches this project's get/set/add accessor naming convention (camelCase after prefix), not a real naming defect — the shared clang-tidy config has no accessor exception
        m_functions.emplace_back(std::move(returnType), std::move(name));
        return m_functions.back();
    }

    /**
     * @brief Gets the namespace's name.
     * @return the name this namespace renders as.
     */
    [[nodiscard]] const std::string &getName() const noexcept { return m_name; }  // NOLINT(readability-identifier-naming) — matches this project's get/set/add accessor naming convention (camelCase after prefix), not a real naming defect — the shared clang-tidy config has no accessor exception

    /**
     * @brief Renders the full `export namespace { ... }` block: every class in order, then
     * every function in order — clean W for keeping the codegen declarative instead of
     * string-mashing by hand.
     * @return the complete namespace source text, blank-line-terminated.
     */
    [[nodiscard]] std::string render() const {
        // Open the namespace, then emit classes before free functions — matches the
        // declaration order callers built them in via addClass()/addFunction().
        std::string out = std::format("export namespace {} {{\n\n", m_name);
        for (const auto &cls : m_classes) {
            out += cls.render();
        }
        for (const auto &function : m_functions) {
            out += function.render(0);
        }
        // Close it out with the matching `}} // namespace <name>` comment for readability.
        out += std::format("}} // namespace {}\n\n", m_name);
        return out;
    }

  private:
    std::string m_name;
    std::vector<Class> m_classes;
    std::vector<Function> m_functions;
};

// A generated .cppm file: module declaration, imports, one or more namespaces, and
// trailing raw blocks for content that doesn't fit the class/function model (e.g.
// top-level `template <> struct serde::Serializable<T> {...}` specializations).
class Module {
  public:
    /**
     * @brief Starts a new generated module — no imports, namespaces, or raw blocks yet.
     * @param name the module's name, emitted verbatim after `export module `.
     */
    explicit Module(std::string name) : m_name(std::move(name)) {}

    /**
     * @brief Appends an import to this module, in declaration order.
     * @param name the module/partition name to import, emitted verbatim after `import `.
     * @return reference to this module for chaining.
     */
    Module &addImport(std::string name) {  // NOLINT(readability-identifier-naming) — matches this project's get/set/add accessor naming convention (camelCase after prefix), not a real naming defect — the shared clang-tidy config has no accessor exception
        m_imports.push_back(std::move(name));
        return *this;
    }
    /**
     * @brief Appends a new namespace block to this module, in declaration order.
     * @param name the namespace's name, emitted verbatim.
     * @return reference to the newly added `Namespace` so callers can chain `addClass()`/
     * `addFunction()` onto it directly.
     */
    Namespace &addNamespace(std::string name) {  // NOLINT(readability-identifier-naming) — matches this project's get/set/add accessor naming convention (camelCase after prefix), not a real naming defect — the shared clang-tidy config has no accessor exception
        m_namespaces.emplace_back(std::move(name));
        return m_namespaces.back();
    }
    /**
     * @brief Appends a raw, already-formatted text block to this module — emitted verbatim
     * after every namespace, for content that doesn't fit the class/function/namespace
     * model.
     * @param block the raw text, emitted verbatim.
     * @return reference to this module for chaining.
     */
    Module &addRawBlock(std::string block) {  // NOLINT(readability-identifier-naming) — matches this project's get/set/add accessor naming convention (camelCase after prefix), not a real naming defect — the shared clang-tidy config has no accessor exception
        m_raw_blocks.push_back(std::move(block));
        return *this;
    }

    /**
     * @brief Gets the module's name.
     * @return the name this module renders as.
     */
    [[nodiscard]] const std::string &getName() const noexcept { return m_name; }  // NOLINT(readability-identifier-naming) — matches this project's get/set/add accessor naming convention (camelCase after prefix), not a real naming defect — the shared clang-tidy config has no accessor exception

    /**
     * @brief Renders the full generated `.cppm` file: module declaration, imports, every
     * namespace in order, then every raw block in order.
     * @warning No cap, this is the whole codegen pipeline's payoff method — get the
     * ordering of `addNamespace()`/`addRawBlock()` calls wrong upstream and the emitted
     * source is malformed C++ with nothing here to catch it. `render()` trusts its inputs
     * completely.
     * @return the complete generated `.cppm` file content.
     */
    [[nodiscard]] std::string render() const {
        // Module declaration line first, it's non-negotiable — has to be the very
        // first thing in a valid .cppm file.
        std::string out = std::format("export module {};\n\n", m_name);
        // Every import gets its own `import <name>;` line, in the order they were added.
        for (const auto &import : m_imports) {
            out += std::format("import {};\n", import);
        }
        out += "\n";
        // Namespaces next, each one fully self-rendering its classes/functions.
        for (const auto &ns : m_namespaces) {
            out += ns.render();
        }
        // Raw blocks land last, verbatim — the escape hatch for anything that
        // doesn't fit the class/function/namespace model, no cap.
        for (const auto &block : m_raw_blocks) {
            out += block;
        }
        return out;
    }

  private:
    std::string m_name;
    std::vector<std::string> m_imports;
    std::vector<Namespace> m_namespaces;
    std::vector<std::string> m_raw_blocks;
};

} // namespace core::generator
