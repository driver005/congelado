module;

export module cc_abi_builder_generator:base_attribute;

import std;
import cc_abi_builder_intern;

export namespace ice {

// Abstract base class for generator attribute view
class GeneratorAttributeViewBase {
public:
    virtual ~GeneratorAttributeViewBase() = default;

    virtual StringBuilder get_name() const = 0;
    virtual StringBuilder get_description() const = 0;
    virtual StringBuilder get_full_type() const = 0;
    virtual StringBuilder get_base_type() const = 0;
    virtual bool is_list() const = 0;
};

} // namespace ice
