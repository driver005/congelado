module;

#include "c/extern/generator/generator.h"

export module cc_abi_sonic_generator:runtime;

import std;
import :function;
import cc_abi_primitives;
import cc_abi_sonic_intern;
import cc_abi_sonic_registration;

export namespace ice::sonic {

// Runtime — the mainframe-facing generator handle. Resolved by name through
// ice::sonic::Registration (type="generator"): the plugin's init_generator
// fills the flat TF_Generator vtable (typically from its Builder's
// get_generic_vtable()) and hands back the plugin context; every call below forwards
// across that vtable.
class Generator : public ice::sonic::Runtime<Generator, TF_Generator>
{
public:
    explicit Generator(TF_Generator* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "generator";

    void set_name(const ice::String& name) noexcept
    {
        m_ops->set_name(get_handle(), name.get_handle());
    }

    ice::String get_name() const noexcept
    {
        ice::String out;
        m_ops->get_name(get_handle(), out.get_handle());
        return out;
    }

    // Definitions come back as a tensor allocated by the plugin — see the
    // TF_Generator_GetDefinitions contract (a 1-D tensor whose elements are the
    // opaque definition handles consumed by the definition_* vtable slots).
    [[nodiscard]] std::expected<ice::TensorHandle, ice::Status> get_definitions() const noexcept
    {
        ice::Status status;
        TF_Tensor_Handle* handle = m_ops->get_definitions(get_handle(), status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return ice::TensorHandle{handle};
    }

    [[nodiscard]] std::expected<ice::String, ice::Status> build() const noexcept
    {
        ice::Status status;
        ice::String out;
        m_ops->build(get_handle(), out.get_handle(), status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return out;
    }

    // Opens a construction unit and transfers ownership of it to the caller — the
    // returned Function owns its C handle (function_destroy runs in ~Function).
    // Same "fresh call replaces the current one" lifetime story as the rest of this
    // interface: a second create_function() call simply returns another owned unit.
    [[nodiscard]] std::expected<std::unique_ptr<ice::sonic::Function>, ice::Status>
    create_function(const ice::String& name) noexcept
    {
        ice::Status status;
        TF_Generator_Function* handle =
            m_ops->create_function(get_handle(), name.get_handle(), status.get_handle());
        if (!status.ok()) {
            if (handle) {
                m_ops->function_destroy(handle);
            }
            return std::unexpected{status};
        }
        return std::make_unique<ice::sonic::Function>(m_ops, handle);
    }
};

} // namespace ice::sonic
