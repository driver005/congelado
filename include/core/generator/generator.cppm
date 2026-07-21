export module core_generator;

export import :statement;
export import :function;
export import :class_builder;
export import :module_builder;

import std;

export namespace core::generator {

// Entry point: either a bare file-writer (default-constructed, used for content that's
// already fully rendered elsewhere — e.g. JSON) or, constructed with a module name, a
// facade over Module for building generated C++ source (addImport/addNamespace/addRawBlock
// delegate to an internally owned Module, then render()/write() emit its content).
class Generator {
  public:
    /**
     * @brief Default-constructs a bare generator with no owned `Module` — no cap, this
     * mode only exists for content that's already fully rendered elsewhere (e.g. JSON) and
     * just needs a `write()` to disk.
     */
    Generator() = default;
    /**
     * @brief Constructs a generator that owns a `Module`, ready to build a generated
     * `.cppm` file via `addImport()`/`addNamespace()`/`addRawBlock()`.
     * @param moduleName the module's name, forwarded straight to `Module`'s constructor.
     */
    explicit Generator(std::string moduleName) : m_module(Module{std::move(moduleName)}) {}

    /**
     * @brief Adds an import to the owned module.
     * @param name the module/partition name to import, emitted verbatim after `import `.
     * @return reference to this generator for chaining.
     * @warning Bare `Generator()` (no module name given at construction) leaves `m_module`
     * empty — call this on one and it's a null `std::optional` dereference, straight L, no
     * exception thrown, just UB.
     */
    Generator &addImport(std::string name) {  // NOLINT(readability-identifier-naming) — matches this project's get/set/add accessor naming convention (camelCase after prefix), not a real naming defect — the shared clang-tidy config has no accessor exception
        // .value() rather than -> : a bare Generator() (no owned Module) throws
        // std::bad_optional_access here instead of silently invoking UB.
        m_module.value().addImport(std::move(name));  // NOLINT(bugprone-unchecked-optional-access) — .value() is the deliberate check, throws instead of UB by design
        return *this;
    }
    /**
     * @brief Adds a namespace block to the owned module.
     * @param name the namespace's name.
     * @return reference to the newly added `Namespace` so callers can chain `addClass()`/
     * `addFunction()` onto it directly.
     * @warning Same `m_module` requirement as `addImport()` — null-optional dereference on
     * a bare `Generator()`.
     */
    // .value() (not ->) so a bare Generator() throws std::bad_optional_access instead of UB.
    // NOLINTNEXTLINE(readability-identifier-naming,bugprone-unchecked-optional-access) — camelCase matches project accessor convention; .value() is the deliberate check
    Namespace &addNamespace(std::string name) { return m_module.value().addNamespace(std::move(name)); }
    /**
     * @brief Adds a raw, already-formatted text block to the owned module — for content
     * that doesn't fit the class/function/namespace model (e.g. a top-level template
     * specialization).
     * @param block the raw text, emitted verbatim.
     * @return reference to this generator for chaining.
     * @warning Same `m_module` requirement as `addImport()`.
     */
    Generator &addRawBlock(std::string block) {  // NOLINT(readability-identifier-naming) — matches this project's get/set/add accessor naming convention (camelCase after prefix), not a real naming defect — the shared clang-tidy config has no accessor exception
        // .value() rather than -> : a bare Generator() (no owned Module) throws
        // std::bad_optional_access here instead of silently invoking UB.
        m_module.value().addRawBlock(std::move(block));  // NOLINT(bugprone-unchecked-optional-access) — .value() is the deliberate check, throws instead of UB by design
        return *this;
    }

    /**
     * @brief Renders the owned module's full generated source text.
     * @return the complete generated `.cppm` file content.
     * @warning Same `m_module` requirement as `addImport()` — dereferences a null
     * `std::optional` on a bare `Generator()`.
     */
    // .value() (not ->) so a bare Generator() throws std::bad_optional_access instead of UB.
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access) — .value() is the deliberate check
    [[nodiscard]] std::string render() const { return m_module.value().render(); }

    /**
     * @brief Writes arbitrary content to disk at the given path. Unlike the other methods
     * here, this one never touches `m_module`, so it works fine even on a bare
     * `Generator()`.
     * @param path the destination file path.
     * @param content the exact bytes to write — unrelated to `render()` unless the caller
     * passes that output in directly.
     * @return an empty success value, or an error string naming the path that failed to
     * open.
     */
    [[nodiscard]] static std::expected<void, std::string>
    write(const std::filesystem::path &path, std::string_view content) {
        // Open and write in one motion — no separate existence/permission check up
        // front, the stream's fail state after the write is the only signal we get.
        std::ofstream out{path};
        out << content;
        // Guard clause: bail with an error naming the path if the stream's in a bad
        // state (failed to open, disk full, whatever) — otherwise it's a clean W.
        if (!out) {
            return std::unexpected{std::format("failed to write '{}'", path.string())};
        }
        return {};
    }

    /**
     * @brief Renders the owned module and writes it to disk in one motion — the go-to path
     * for a `Generator` constructed with a module name.
     * @param path the destination file path.
     * @return an empty success value, or an error string naming the path that failed to
     * open.
     * @warning Delegates to `render()`, so it inherits the same bare-`Generator()`
     * null-optional footgun.
     */
    [[nodiscard]] std::expected<void, std::string> write(const std::filesystem::path &path) const {
        return write(path, render());
    }

  private:
    std::optional<Module> m_module;
};

} // namespace core::generator
