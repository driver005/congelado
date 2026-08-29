module;

#include "c/extern/generator/generator.h"

export module cc_abi_sonic_generator:parameter;

import std;
import :typeinfo;
import cc_abi_primitives;

export namespace ice::sonic {

// C ABI adapter for the flat TF_Generator vtable's parameter__* slots. Owning —
// destroys the handle with parameter__destroy (the plugin released ownership
// when it handed the handle out).
class Parameter
{
public:
    explicit Parameter(TF_Generator* ops, void* handle) noexcept :
        m_ops{ops},
        m_handle{handle}
    {
    }

    ~Parameter()
    {
        if (m_ops && m_handle) {
            m_ops->parameter__destroy(m_handle);
        }
    }

    Parameter(const Parameter&) = delete;
    Parameter& operator=(const Parameter&) = delete;
    Parameter(Parameter&&) = delete;
    Parameter& operator=(Parameter&&) = delete;

    ice::String get_name() const noexcept
    {
        ice::String out;
        m_ops->parameter__get_name(m_handle, out.get_handle());
        return out;
    }

    ice::String get_description() const noexcept
    {
        ice::String out;
        m_ops->parameter__get_description(m_handle, out.get_handle());
        return out;
    }

    int get_position() const noexcept
    {
        return m_ops->parameter__get_position(m_handle);
    }

    std::unique_ptr<ice::sonic::TypeInfo> get_type() const noexcept
    {
        void* handle = m_ops->parameter__get_type(m_handle);
        if (!handle) {
            return nullptr;
        }
        return std::make_unique<TypeInfo>(m_ops, handle);
    }

private:
    TF_Generator* m_ops;
    void* m_handle;
};

} // namespace ice::sonic
