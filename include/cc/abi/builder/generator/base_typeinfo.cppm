module;

export module cc_abi_builder_generator:base_typeinfo;

import std;
import cc_abi_builder_intern;

export namespace ice {

// Abstract base class for generator type info view
class GeneratorTypeInfoViewBase {
public:
    virtual ~GeneratorTypeInfoViewBase() = default;

    virtual int get_data_type() const = 0;
    virtual StringBuilder get_type_attr_name() const = 0;
    virtual bool is_read_only() const = 0;
    virtual bool is_list() const = 0;
};

} // namespace ice
