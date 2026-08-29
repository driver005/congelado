module;

export module cc_abi_builder_generator:attribute;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;

export namespace ice::builder {

// Abstract base class for generator attribute view
class Attribute
{
public:
    // Recover the Attribute instance from the opaque void* context slot that every
    // C vtable callback receives.  Named accessor so the cast intent is explicit
    // at the call site and the static_cast appears exactly once, here.
    static Attribute* create(void* ctx) noexcept
    {
        return static_cast<Attribute*>(ctx);
    }

    virtual ~Attribute() = default;

    virtual ice::String get_name() const noexcept = 0;
    virtual ice::String get_description() const noexcept = 0;
    virtual ice::String get_full_type() const noexcept = 0;
    virtual ice::String get_base_type() const noexcept = 0;
    virtual bool is_list() const noexcept = 0;
};

} // namespace ice::builder
