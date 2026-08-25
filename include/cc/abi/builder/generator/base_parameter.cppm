module;

export module cc_abi_builder_generator:base_parameter;

import std;
import cc_abi_builder_intern;
import :base_typeinfo;

export namespace ice {

// Abstract base class for generator parameter view
class GeneratorParameterViewBase {
public:
    virtual ~GeneratorParameterViewBase() = default;

    virtual StringBuilder get_name() const = 0;
    virtual StringBuilder get_description() const = 0;
    virtual int get_position() const = 0;
    virtual std::unique_ptr<GeneratorTypeInfoViewBase> get_type() const = 0;
};

} // namespace ice
