module;

export module cc_abi_builder_generator:parameter;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;
import :typeinfo;

export namespace ice::builder::generator {

// Abstract base class for generator parameter view
class Parameter
{
public:
    virtual ~Parameter() = default;

    virtual ice::String get_name() const = 0;
    virtual ice::String get_description() const = 0;
    virtual int get_position() const = 0;
    virtual std::unique_ptr<TypeInfo> get_type() const = 0;
};

} // namespace ice::builder
