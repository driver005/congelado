module;

export module cc_abi_builder_generator:definition;

import std;
import cc_abi_sonic_intern;
import :parameter;
import :attribute;

export namespace ice::builder::generator {

// Abstract base class for generator definition view
class Definition
{
public:
    virtual ~Definition() = default;

    virtual ice::sonic::StringRuntime get_name() const = 0;
    virtual ice::sonic::StringRuntime get_summary() const = 0;
    virtual ice::sonic::StringRuntime get_description() const = 0;

    virtual std::size_t get_input_count() const = 0;
    virtual std::unique_ptr<Parameter> get_input(std::size_t index) const = 0;

    virtual std::size_t get_output_count() const = 0;
    virtual std::unique_ptr<Parameter> get_output(std::size_t index) const = 0;

    virtual std::size_t get_attr_count() const = 0;
    virtual std::unique_ptr<Attribute> get_attr(std::size_t index) const = 0;
};

} // namespace ice::builder::generator
