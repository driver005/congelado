module;

#include "c/extern/generator/generator.h"

export module cc_abi_sonic_generator:definition;

import std;
import cc_abi_primitives;

export namespace ice::sonic {

// C ABI adapter for the flat TF_Generator vtable's definition_* slots. Owning — destroys the
// handle with definition_destroy (the plugin released ownership when it handed the handle out).
// Mirrors ice::builder::Definition's query surface for the mainframe side.
class Definition
{
public:
    explicit Definition(TF_Generator* ops, TF_Generator_Definition* handle) noexcept :
        m_ops{ops},
        m_handle{handle}
    {
    }

    ~Definition()
    {
        if (m_ops && m_handle) {
            m_ops->definition_destroy(m_handle);
        }
    }

    Definition(const Definition&) = delete;
    Definition& operator=(const Definition&) = delete;
    Definition(Definition&&) = delete;
    Definition& operator=(Definition&&) = delete;

    ice::String get_name() const noexcept
    {
        ice::String out;
        m_ops->definition_get_name(m_handle, out.get_handle());
        return out;
    }

    ice::String get_summary() const noexcept
    {
        ice::String out;
        m_ops->definition_get_summary(m_handle, out.get_handle());
        return out;
    }

    ice::String get_description() const noexcept
    {
        ice::String out;
        m_ops->definition_get_description(m_handle, out.get_handle());
        return out;
    }

    // Inputs/outputs/attrs come back as plugin-allocated 1-D handle tensors — passed through
    // opaquely, same contract as ice::sonic::Generator::get_definitions().
    [[nodiscard]] std::expected<ice::TensorHandle, ice::Status> get_inputs() const noexcept
    {
        ice::Status status;
        TF_Tensor_Handle* handle = m_ops->definition_get_inputs(m_handle, status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return ice::TensorHandle{handle};
    }

    [[nodiscard]] std::expected<ice::TensorHandle, ice::Status> get_outputs() const noexcept
    {
        ice::Status status;
        TF_Tensor_Handle* handle = m_ops->definition_get_outputs(m_handle, status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return ice::TensorHandle{handle};
    }

    [[nodiscard]] std::expected<ice::TensorHandle, ice::Status> get_attrs() const noexcept
    {
        ice::Status status;
        TF_Tensor_Handle* handle = m_ops->definition_get_attrs(m_handle, status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return ice::TensorHandle{handle};
    }

private:
    TF_Generator* m_ops;
    TF_Generator_Definition* m_handle;
};

} // namespace ice::sonic
