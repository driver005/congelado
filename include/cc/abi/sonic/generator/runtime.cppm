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
// ice::sonic::RegistrationRuntime (type="generator"): the plugin's init_generator
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
    // opaque definition handles consumed by the definition__* vtable slots).
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

    // Opens a construction unit. A fresh call replaces the current one: emplace()
    // destroys whatever was previously open (running ~Function()'s
    // function__destroy) before constructing the new one — same "a fresh call
    // replaces the current one" lifetime story as the rest of this interface.
    [[nodiscard]] std::expected<std::reference_wrapper<ice::sonic::Function>, ice::Status>
    enter_border_patrol(const ice::String& name) noexcept
    {
        ice::Status status;
        void* handle =
            m_ops->enter_border_patrol(get_handle(), name.get_handle(), status.get_handle());
        if (!status.ok()) {
            if (handle) {
                m_ops->function__destroy(handle);
            }
            return std::unexpected{status};
        }
        m_open_function.emplace(m_ops, handle);
        ice::sonic::Function& function = *m_open_function;
        return std::ref(function);
    }

private:
    // Owns whichever construction unit is currently open — the mainframe keeps the
    // reference_wrapper returned by enter_border_patrol for as long as it needs it.
    std::optional<Function> m_open_function;
};

} // namespace ice::sonic
