module;

export module cc_abi_builder_generator:parameter;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;
import :typeinfo;

export namespace ice::builder {

// Abstract base class for generator parameter view
class Parameter
{
public:
    // Recover the Parameter instance from the opaque void* context slot that every
    // C vtable callback receives.  Named accessor so the cast intent is explicit
    // at the call site and the static_cast appears exactly once, here.
    static Parameter* create(void* ctx) noexcept
    {
        return static_cast<Parameter*>(ctx);
    }

    virtual ~Parameter() = default;

    virtual ice::String get_name() const = 0;
    virtual ice::String get_description() const = 0;
    virtual int get_position() const = 0;
    virtual std::unique_ptr<TypeInfo> get_type() const = 0;
};

} // namespace ice::builder
