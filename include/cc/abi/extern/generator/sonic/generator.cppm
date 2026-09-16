// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from c/extern/generator/generator.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "c/extern/generator/generator.h"

export module cc_abi_sonic_generator;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;
import cc_abi_sonic_registration;

export namespace ice::sonic {

class Generator : public ice::sonic::Runtime<Generator, TF_Generator>
{
public:
    explicit Generator(TF_Generator* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "generator";

    [[nodiscard]] std::expected<void, ice::Status> set_name(const ice::sonic::String& name) noexcept
    {
        ice::Status status;
        m_ops->set_name(get_handle(), name.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> get_definitions() noexcept
    {
        ice::Status status;
        m_ops->get_definitions(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> build(TF_String* out) noexcept
    {
        ice::Status status;
        m_ops->build(get_handle(), out, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    create_function(const ice::sonic::String& name) noexcept
    {
        ice::Status status;
        m_ops->create_function(get_handle(), name.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> definition_destroy() noexcept
    {
        ice::Status status;
        m_ops->definition_destroy(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> definition_get_name(TF_String* out) noexcept
    {
        ice::Status status;
        m_ops->definition_get_name(get_handle(), out, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> definition_get_summary(TF_String* out) noexcept
    {
        ice::Status status;
        m_ops->definition_get_summary(get_handle(), out, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    definition_get_description(TF_String* out) noexcept
    {
        ice::Status status;
        m_ops->definition_get_description(get_handle(), out, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> definition_get_inputs() noexcept
    {
        ice::Status status;
        m_ops->definition_get_inputs(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> definition_get_outputs() noexcept
    {
        ice::Status status;
        m_ops->definition_get_outputs(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> definition_get_attrs() noexcept
    {
        ice::Status status;
        m_ops->definition_get_attrs(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> parameter_destroy() noexcept
    {
        ice::Status status;
        m_ops->parameter_destroy(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> parameter_get_name(TF_String* out) noexcept
    {
        ice::Status status;
        m_ops->parameter_get_name(get_handle(), out, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    parameter_get_description(TF_String* out) noexcept
    {
        ice::Status status;
        m_ops->parameter_get_description(get_handle(), out, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> parameter_get_position() noexcept
    {
        ice::Status status;
        m_ops->parameter_get_position(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> parameter_get_type() noexcept
    {
        ice::Status status;
        m_ops->parameter_get_type(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> attribute_destroy() noexcept
    {
        ice::Status status;
        m_ops->attribute_destroy(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> attribute_get_name(TF_String* out) noexcept
    {
        ice::Status status;
        m_ops->attribute_get_name(get_handle(), out, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    attribute_get_description(TF_String* out) noexcept
    {
        ice::Status status;
        m_ops->attribute_get_description(get_handle(), out, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> attribute_get_full_type(TF_String* out) noexcept
    {
        ice::Status status;
        m_ops->attribute_get_full_type(get_handle(), out, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> attribute_get_base_type(TF_String* out) noexcept
    {
        ice::Status status;
        m_ops->attribute_get_base_type(get_handle(), out, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> attribute_is_list() noexcept
    {
        ice::Status status;
        m_ops->attribute_is_list(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> typeinfo_destroy() noexcept
    {
        ice::Status status;
        m_ops->typeinfo_destroy(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> typeinfo_get_data_type() noexcept
    {
        ice::Status status;
        m_ops->typeinfo_get_data_type(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    typeinfo_get_type_attr_name(TF_String* out) noexcept
    {
        ice::Status status;
        m_ops->typeinfo_get_type_attr_name(get_handle(), out, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> typeinfo_is_read_only() noexcept
    {
        ice::Status status;
        m_ops->typeinfo_is_read_only(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> typeinfo_is_list() noexcept
    {
        ice::Status status;
        m_ops->typeinfo_is_list(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    ice::String get_name() const noexcept
    {
        ice::String result;
        m_ops->get_name(get_handle(), result.get_handle());
        return result;
    }
};

} // namespace ice::sonic
