// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/random_generator/random_generator.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/random_generator/random_generator.h"

export module cc_ice_extern_random_generator_sonic:random_generator;

import std;
import cc_abi_sonic_registration;

export namespace ice::sonic {

class TF_RandomGeneratorOps :
    public ice::sonic::Runtime<TF_RandomGeneratorOps, TF_RandomGeneratorOps>
{
public:
    explicit TF_RandomGeneratorOps(TF_RandomGeneratorOps* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "random_generator";

    [[nodiscard]] std::expected<void, ice::Status> set_seed(uint64_t seed) noexcept
    {
        ice::Status status;
        m_ops->set_seed(get_handle(), seed status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> get_seed(uint64_t* out_seed) noexcept
    {
        ice::Status status;
        m_ops->get_seed(get_handle(), out_seed status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    reseed_nondeterministic(uint64_t* out_seed) noexcept
    {
        ice::Status status;
        m_ops->reseed_nondeterministic(get_handle(), out_seed status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> set_offset(uint64_t offset) noexcept
    {
        ice::Status status;
        m_ops->set_offset(get_handle(), offset status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> get_offset(uint64_t* out_offset) noexcept
    {
        ice::Status status;
        m_ops->get_offset(get_handle(), out_offset status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    set_state(const ice::sonic::TF_TensorOps& state) noexcept
    {
        ice::Status status;
        m_ops->set_state(get_handle(), state.get_handle() status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> get_state(TF_Tensor** out_state) noexcept
    {
        ice::Status status;
        m_ops->get_state(get_handle(), out_state status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    graphsafe_set_state(const ice::sonic::TF_RandomGeneratorOps& other) noexcept
    {
        ice::Status status;
        m_ops->graphsafe_set_state(get_handle(), other.get_handle() status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    graphsafe_get_state(const ice::sonic::TF_RandomGeneratorOps& out_other) noexcept
    {
        ice::Status status;
        m_ops->graphsafe_get_state(get_handle(), out_other.get_handle() status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    philox_state(uint64_t increment, TF_PhiloxState* out_state) noexcept
    {
        ice::Status status;
        m_ops->philox_state(get_handle(), increment, out_state status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    philox_engine_inputs(uint64_t increment, uint64_t* out_seed, uint64_t* out_offset) noexcept
    {
        ice::Status status;
        m_ops->philox_engine_inputs(
            get_handle(),
            increment,
            out_seed,
            out_offset status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> get_device_index(int* out_device_index) noexcept
    {
        ice::Status status;
        m_ops->get_device_index(get_handle(), out_device_index status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    clone(const ice::sonic::TF_RandomGeneratorOps& out_clone) noexcept
    {
        ice::Status status;
        m_ops->clone(get_handle(), out_clone.get_handle() status.get_handle());

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
