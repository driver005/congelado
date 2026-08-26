module;

export module cc_abi_builder_generator:attribute;

import std;
import cc_abi_sonic_intern;

export namespace ice::builder::generator {

// Abstract base class for generator attribute view
class Attribute
{
public:
    virtual ~Attribute() = default;

    virtual ice::sonic::StringRuntime get_name() const = 0;
    virtual ice::sonic::StringRuntime get_description() const = 0;
    virtual ice::sonic::StringRuntime get_full_type() const = 0;
    virtual ice::sonic::StringRuntime get_base_type() const = 0;
    virtual bool is_list() const = 0;
};

} // namespace ice::builder::generator
