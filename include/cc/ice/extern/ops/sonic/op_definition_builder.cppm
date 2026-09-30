// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/ops/op_definition_builder.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/ops/op_definition_builder.h"

export module cc_ice_extern_ops_sonic:op_definition_builder;

import std;
import cc_abi_sonic_registration;

export namespace ice::sonic {

class TF_OpDefinitionBuilderOps :
    public ice::sonic::Runtime<TF_OpDefinitionBuilderOps, TF_OpDefinitionBuilderOps>
{
public:
    explicit TF_OpDefinitionBuilderOps(
        TF_OpDefinitionBuilderOps* ops,
        void* plugin_context
    ) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "ops";

    [[nodiscard]] std::expected<void, ice::Status>
    add_attr(const ice::sonic::String& attr_spec) noexcept
    {
        ice::Status status;
        m_ops->add_attr(get_handle(), attr_spec.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    add_input(const ice::sonic::String& input_spec) noexcept
    {
        ice::Status status;
        m_ops->add_input(get_handle(), input_spec.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    add_output(const ice::sonic::String& output_spec) noexcept
    {
        ice::Status status;
        m_ops->add_output(get_handle(), output_spec.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> set_is_commutative(_Bool is_commutative) noexcept
    {
        ice::Status status;
        m_ops->set_is_commutative(get_handle(), is_commutative, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> set_is_aggregate(_Bool is_aggregate) noexcept
    {
        ice::Status status;
        m_ops->set_is_aggregate(get_handle(), is_aggregate, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> set_is_stateful(_Bool is_stateful) noexcept
    {
        ice::Status status;
        m_ops->set_is_stateful(get_handle(), is_stateful, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    set_allows_uninitialized_input(_Bool allows_uninitialized_input) noexcept
    {
        ice::Status status;
        m_ops->set_allows_uninitialized_input(
            get_handle(),
            allows_uninitialized_input,
            status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    deprecated(int version, const ice::sonic::String& explanation) noexcept
    {
        ice::Status status;
        m_ops->deprecated(get_handle(), version, explanation.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> set_shape_inference_function(
        void (*)(TF_ShapeInferenceContext*, TF_Status*) shape_inference_func
    ) noexcept
    {
        ice::Status status;
        m_ops
            ->set_shape_inference_function(get_handle(), shape_inference_func, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> register_op_definition() noexcept
    {
        ice::Status status;
        m_ops->register_op_definition(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    virtual ice::String get_name() const noexcept = 0;

    sonic::String get_name() const noexcept
    {
        sonic::String result;
        m_ops->get_name(get_handle(), result.get_handle());
        return result;
    }
};

} // namespace ice::sonic
