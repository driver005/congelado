module;

#include "c/extern/generator/attribute.h"

export module cc_abi_sonic_generator:attribute;

import cc_abi_sonic_intern;
export namespace ice::sonic {

// C ABI adapter: implements ice::builder::Attribute by calling
// TF_Generator_Attribute_* functions.
class Attribute : public ice::builder::Attribute
{
public:
    explicit Attribute(const TF_Generator_Attribute* handle) :
        m_handle(handle)
    {
    }

    ~Attribute() = default;

    Attribute(const Attribute&) = default;
    Attribute& operator=(const Attribute&) = default;
    Attribute(Attribute&&) = default;
    Attribute& operator=(Attribute&&) = default;

    String get_name() const
    {

        return String(TF_Generator_Attribute_GetName(m_handle));
    }

    String get_description() const
    {

        return String(TF_Generator_Attribute_GetDescription(m_handle));
    }

    String get_full_type() const
    {

        return String(TF_Generator_Attribute_GetFullType(m_handle));
    }

    String get_base_type() const
    {

        return String(TF_Generator_Attribute_GetBaseType(m_handle));
    }

    bool is_list() const
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

} // namespace ice::sonic
