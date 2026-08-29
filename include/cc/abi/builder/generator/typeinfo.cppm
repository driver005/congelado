module;

export module cc_abi_builder_generator:typeinfo;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;

export namespace ice::builder {

// Abstract base class for generator type info view
class TypeInfo
{
public:
    // Recover the TypeInfo instance from the opaque void* context slot that every
    // C vtable callback receives.  Named accessor so the cast intent is explicit
    // at the call site and the static_cast appears exactly once, here.
    static TypeInfo* create(void* ctx) noexcept
    {
        return static_cast<TypeInfo*>(ctx);
    }

    virtual ~TypeInfo() = default;

    virtual int get_data_type() const = 0;
    virtual ice::String get_type_attr_name() const = 0;
    virtual bool is_read_only() const = 0;
    virtual bool is_list() const = 0;
};

} // namespace ice::builder
