module;

#include "c/extern/generator/generator.h"

export module cc_abi_sonic_generator:typeinfo;

import std;
import cc_abi_primitives;

export namespace ice::sonic {

// C ABI adapter for the flat TF_Generator vtable's typeinfo__* slots. Owning —
// destroys the handle with typeinfo__destroy (the plugin released ownership
// when it handed the handle out).
class TypeInfo
{
public:
    explicit TypeInfo(TF_Generator* ops, TF_TypeInfo* handle) noexcept :
        m_ops{ops},
        m_handle{handle}
    {
    }

    ~TypeInfo()
    {
        if (m_ops && m_handle) {
            m_ops->typeinfo__destroy(m_handle);
        }
    }

    TypeInfo(const TypeInfo&) = delete;
    TypeInfo& operator=(const TypeInfo&) = delete;
    TypeInfo(TypeInfo&&) = delete;
    TypeInfo& operator=(TypeInfo&&) = delete;

    int get_data_type() const noexcept
    {
        return m_ops->typeinfo__get_data_type(m_handle);
    }

    ice::String get_type_attr_name() const noexcept
    {
        ice::String out;
        m_ops->typeinfo__get_type_attr_name(m_handle, out.get_handle());
        return out;
    }

    bool is_read_only() const noexcept
    {
        return m_ops->typeinfo__is_read_only(m_handle);
    }

    bool is_list() const noexcept
    {
        return m_ops->typeinfo__is_list(m_handle);
    }

private:
    TF_Generator* m_ops;
    TF_TypeInfo* m_handle;
};

} // namespace ice::sonic
