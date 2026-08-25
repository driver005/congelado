module;

export module cc_abi_builder_generator:generator_builder_view_base;

import std;
import cc_abi_builder_intern;
import :base_definition;
import :base_sourcecode;

export namespace ice {

// Non-owning view (for read-only access)
class GeneratorBuilderViewBase {
public:
    virtual ~GeneratorBuilderViewBase() = default;

    virtual std::size_t get_definition_count() const = 0;
    virtual std::unique_ptr<GeneratorDefinitionViewBase> get_definition(std::size_t index) const = 0;
    virtual void write_file(std::string_view path, const GeneratorSourceCodeBase& code) = 0;
    virtual void write_module(std::string_view path) = 0;
    virtual StringBuilder get_name() const = 0;
};

} // namespace ice
