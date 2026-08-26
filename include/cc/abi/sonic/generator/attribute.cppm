module;

#include "c/extern/generator/attribute.h"

export module cc_abi_sonic_generator:attribute;

import cc_abi_sonic_intern;
import cc_abi_builder_generator;

export namespace ice::sonic::generator {

// C ABI adapter: implements ice::builder::generator::Attribute by calling
// TF_Generator_Attribute_* functions.
class Attribute : public ice::builder::generator::Attribute
{
public:
    explicit Attribute(const TF_Generator_Attribute* handle) :
        m_handle(handle)
    {
    }

    ~Attribute() override = default;

    Attribute(const Attribute&) = default;
    Attribute& operator=(const Attribute&) = default;
    Attribute(Attribute&&) = default;
    Attribute& operator=(Attribute&&) = default;

    StringRuntime get_name() const override
    {

        return StringRuntime(TF_Generator_Attribute_GetName(m_handle));
    }

    StringRuntime get_description() const override
    {

        return StringRuntime(TF_Generator_Attribute_GetDescription(m_handle));
    }

    StringRuntime get_full_type() const override
    {

        return StringRuntime(TF_Generator_Attribute_GetFullType(m_handle));
    }

    StringRuntime get_base_type() const override
    {

        return StringRuntime(TF_Generator_Attribute_GetBaseType(m_handle));
    }

    bool is_list() const override
    {

        return TF_Generator_Attribute_IsList(m_handle);
    }

    const TF_Generator_Attribute* get_handle() const
    {
        return m_handle;
    }

private:
    const TF_Generator_Attribute* m_handle;
};

} // namespace ice::sonic::generator
