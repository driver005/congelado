module;

#include "c/extern/generator/generator.h"

export module cc_abi_sonic_generator:function;

import std;
import :parameter;
import cc_abi_primitives;

export namespace ice::sonic {

// C ABI adapter for the flat TF_Generator vtable's function__* slots. Owning —
// this wraps a real opened construction resource (the plugin released ownership
// when enter_border_patrol returned the handle), non-copyable, destroys the
// handle in its destructor — same ownership shape as the Generator runtime
// itself.
class Function
{
public:
    explicit Function(TF_Generator* ops, void* handle) :
        m_ops{ops},
        m_handle{handle}
    {
    }

    ~Function()
    {
        if (m_ops && m_handle) {
            m_ops->function__destroy(m_handle);
        }
    }

    Function(const Function&) = delete;
    Function& operator=(const Function&) = delete;
    Function(Function&&) = delete;
    Function& operator=(Function&&) = delete;

    [[nodiscard]] std::expected<std::unique_ptr<ice::sonic::Parameter>, ice::Status>
    add_parameter(const ice::String& name, const ice::String& type_text)
    {
        ice::Status status;
        void* handle = m_ops->function__add_parameter(
            m_handle,
            name.get_handle(),
            type_text.get_handle(),
            status.get_handle()
        );
        if (!status.ok()) {
            if (handle) {
                m_ops->parameter__destroy(handle);
            }
            return std::unexpected{status};
        }
        return std::make_unique<Parameter>(m_ops, handle);
    }

    [[nodiscard]] std::expected<void, ice::Status> add_node(
        const void* def_context,
        ice::TensorHandle operands,
        ice::TensorHandle attrs,
        ice::TensorHandle out_results
    )
    {
        ice::Status status;
        m_ops->function__add_node(
            m_handle,
            def_context,
            operands.get_handle(),
            attrs.get_handle(),
            out_results.get_handle(),
            status.get_handle()
        );
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> exit_border_patrol(ice::TensorHandle outputs)
    {
        ice::Status status;
        m_ops->function__exit_border_patrol(m_handle, outputs.get_handle(), status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

private:
    TF_Generator* m_ops;
    void* m_handle; // owning — this object destroys it
};

} // namespace ice::sonic
