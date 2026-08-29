module;

#include "c/extern/generator/generator.h"

export module cc_abi_sonic_generator:attribute;

import std;
import cc_abi_primitives;

export namespace ice::sonic {

// C ABI adapter for the flat TF_Generator vtable's attribute__* slots. Owning —
// destroys the handle with attribute__destroy.
class Attribute
{
public:
    explicit Attribute(TF_Generator* ops, void* handle) noexcept :
        m_ops{ops},
        m_handle{handle}
    {
    }

    ~Attribute()
    {
        if (m_ops && m_handle) {
            m_ops->attribute__destroy(m_handle);
        }
    }

    Attribute(const Attribute&) = delete;
    Attribute& operator=(const Attribute&) = delete;
    Attribute(Attribute&&) = delete;
    Attribute& operator=(Attribute&&) = delete;

    ice::String get_name() const noexcept
    {
        ice::String out;
        m_ops->attribute__get_name(m_handle, out.get_handle());
        return out;
    }

    ice::String get_description() const noexcept
    {
        ice::String out;
        m_ops->attribute__get_description(m_handle, out.get_handle());
        return out;
    }

    ice::String get_full_type() const noexcept
    {
        ice::String out;
        m_ops->attribute__get_full_type(m_handle, out.get_handle());
        return out;
    }

    ice::String get_base_type() const noexcept
    {
        ice::String out;
        m_ops->attribute__get_base_type(m_handle, out.get_handle());
        return out;
    }

    bool is_list() const noexcept
    {
        return m_ops->attribute__is_list(m_handle);
    }

private:
    TF_Generator* m_ops;
    void* m_handle;
};

} // namespace ice::sonic
