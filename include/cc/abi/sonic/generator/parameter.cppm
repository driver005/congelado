module;

#include "c/extern/generator/generator.h"

export module cc_abi_sonic_generator:parameter;

import std;
import :typeinfo;
import cc_abi_primitives;

export namespace ice::sonic {

// C ABI adapter for the flat TF_Generator vtable's parameter_* slots. Owning —
// destroys the handle with parameter_destroy (the plugin released ownership
// when it handed the handle out).
class Parameter
{
public:
    explicit Parameter(TF_Generator* ops, TF_Generator_Parameter* handle) noexcept :
        m_ops{ops},
        m_handle{handle}
    {
    }

    ~Parameter()
    {
        if (m_ops && m_handle) {
            m_ops->parameter_destroy(m_handle);
        }
    }

    Parameter(const Parameter&) = delete;
    Parameter& operator=(const Parameter&) = delete;
    Parameter(Parameter&&) = delete;
    Parameter& operator=(Parameter&&) = delete;

    ice::String get_name() const noexcept
    {
        ice::String out;
        m_ops->parameter_get_name(m_handle, out.get_handle());
        return out;
    }

    ice::String get_description() const noexcept
    {
        ice::String out;
        m_ops->parameter_get_description(m_handle, out.get_handle());
        return out;
    }

    int get_position() const noexcept
    {
        return m_ops->parameter_get_position(m_handle);
    }

    std::unique_ptr<ice::sonic::TypeInfo> get_type() const noexcept
    {
        TF_TypeInfo* handle = m_ops->parameter_get_type(m_handle);
        if (!handle) {
            return nullptr;
        }
        return std::make_unique<TypeInfo>(m_ops, handle);
    }

private:
    TF_Generator* m_ops;
    TF_Generator_Parameter* m_handle;
};

} // namespace ice::sonic
