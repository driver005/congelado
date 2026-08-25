module;

export module cc_abi_builder_generator:base_definition;

import std;
import cc_abi_builder_intern;
import :base_parameter;
import :base_attribute;

export namespace ice {

// Abstract base class for generator definition view
class GeneratorDefinitionViewBase {
public:
    virtual ~GeneratorDefinitionViewBase() = default;

    virtual StringBuilder get_name() const = 0;
    virtual StringBuilder get_summary() const = 0;
    virtual StringBuilder get_description() const = 0;

    virtual std::size_t get_input_count() const = 0;
    virtual std::unique_ptr<GeneratorParameterViewBase> get_input(std::size_t index) const = 0;

    virtual std::size_t get_output_count() const = 0;
    virtual std::unique_ptr<GeneratorParameterViewBase> get_output(std::size_t index) const = 0;

    virtual std::size_t get_attr_count() const = 0;
    virtual std::unique_ptr<GeneratorAttributeViewBase> get_attr(std::size_t index) const = 0;
};

} // namespace ice
