module;

export module cc_abi_builder_generator:typeinfo;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;

export namespace ice::builder::generator {

// Abstract base class for generator type info view
class TypeInfo
{
public:
    virtual ~TypeInfo() = default;

    virtual int get_data_type() const = 0;
    virtual ice::String get_type_attr_name() const = 0;
    virtual bool is_read_only() const = 0;
    virtual bool is_list() const = 0;
};

} // namespace ice::builder
