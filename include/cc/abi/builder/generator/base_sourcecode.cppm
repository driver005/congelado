module;

export module cc_abi_builder_generator:base_sourcecode;

import std;
import cc_abi_builder_intern;

export namespace ice {

// Abstract base class for generator source code builder
class GeneratorSourceCodeBase {
public:
    virtual ~GeneratorSourceCodeBase() = default;

    virtual void add_line(std::string_view line) = 0;
    virtual void add_blank_line() = 0;
    virtual void increase_indent() = 0;
    virtual void decrease_indent() = 0;
    virtual void set_spaces_per_indent(int spaces) = 0;

    // Render the complete source code as a string
    virtual StringBuilder render() const = 0;
};

} // namespace ice
