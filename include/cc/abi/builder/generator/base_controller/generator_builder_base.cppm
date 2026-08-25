module;

export module cc_abi_builder_generator:generator_builder_base;

import std;
import cc_abi_builder_intern;
import :base_definition;
import :base_sourcecode;

export namespace ice {

// Abstract base class for a generator builder — implemented by generator implementations
// (e.g. stable_hlo) AND by the C ABI adapter (runtime/generator's cross-plugin path).
class GeneratorBuilderBase {
public:
    virtual ~GeneratorBuilderBase() = default;

    // Factory method — implemented by each generator
    using Factory = std::unique_ptr<GeneratorBuilderBase> (*)(std::string_view output_dir, std::string_view source_dir);

    // Write generated source code to a file
    virtual void write_file(std::string_view path, const GeneratorSourceCodeBase& code) = 0;

    // Write the complete built module (catalog) to a file
    virtual void write_module(std::string_view path) = 0;

    // Get number of op definitions
    virtual std::size_t get_definition_count() const = 0;

    // Get definition view by index
    virtual std::unique_ptr<GeneratorDefinitionViewBase> get_definition(std::size_t index) const = 0;

    // Generator identity
    virtual void set_name(std::string_view name) = 0;
    virtual StringBuilder get_name() const = 0;
};

} // namespace ice
