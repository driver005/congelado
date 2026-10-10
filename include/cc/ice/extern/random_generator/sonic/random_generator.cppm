// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/random_generator/random_generator.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/random_generator/random_generator.h"
#include "include/c/extern/registration/registration.h"

export module cc_ice_extern_random_generator_sonic:random_generator;

import std;
import cc_ice_intern_sonic;

export namespace ice::sonic {

class TF_RandomGeneratorOps :
    public ice::sonic::Runtime<::TF_RandomGeneratorOps, ::TF_RandomGenerator>
{
public:
    TF_RandomGeneratorOps(
        const ::TF_RegistrationOps& registry_ops,
        ::TF_Registration* registry_handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry_ops, registry_handle, type.get_handle(), provider.get_handle())
    {
    }

    TF_RandomGeneratorOps(
        const ::TF_RegistrationOps& registry_ops,
        ::TF_Registration* registry_handle,
        ::TF_RandomGenerator* handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry_ops, registry_handle, handle, type.get_handle(), provider.get_handle())
    {
    }

    explicit TF_RandomGeneratorOps(const ::TF_RandomGeneratorOps* ops) noexcept :
        Runtime(ops)
    {
    }

    TF_RandomGeneratorOps(const ::TF_RandomGeneratorOps* ops, ::TF_RandomGenerator* handle) noexcept
        :
        Runtime(ops, handle)
    {
    }

    void create() const noexcept
    {
        m_ops->create(get_handle());
    }

    void destroy() const noexcept
    {
        m_ops->destroy(get_handle());
    }

    void set_seed(uint64_t seed) const noexcept
    {
        m_ops->set_seed(get_handle(), seed);
    }

    void get_seed(uint64_t* out_seed) const noexcept
    {
        m_ops->get_seed(get_handle(), out_seed);
    }

    void reseed_nondeterministic(uint64_t* out_seed) const noexcept
    {
        m_ops->reseed_nondeterministic(get_handle(), out_seed);
    }

    void set_offset(uint64_t offset) const noexcept
    {
        m_ops->set_offset(get_handle(), offset);
    }

    void get_offset(uint64_t* out_offset) const noexcept
    {
        m_ops->get_offset(get_handle(), out_offset);
    }

    void set_state(
        const ice::sonic::TF_TensorOps& state,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->set_state(get_handle(), state.get_handle(), out_status.get_handle());
    }

    void get_state(TF_Tensor** out_state, const ice::sonic::Status& out_status) const noexcept
    {
        m_ops->get_state(get_handle(), out_state, out_status.get_handle());
    }

    void graphsafe_set_state(
        const ice::sonic::TF_RandomGeneratorOps& other,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->graphsafe_set_state(get_handle(), other.get_handle(), out_status.get_handle());
    }

    void graphsafe_get_state(
        const ice::sonic::TF_RandomGeneratorOps& out_other,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->graphsafe_get_state(get_handle(), out_other.get_handle(), out_status.get_handle());
    }

    void philox_state(
        uint64_t increment,
        TF_PhiloxState* out_state,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->philox_state(get_handle(), increment, out_state, out_status.get_handle());
    }

    void philox_engine_inputs(
        uint64_t increment,
        uint64_t* out_seed,
        uint64_t* out_offset,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->philox_engine_inputs(
            get_handle(),
            increment,
            out_seed,
            out_offset,
            out_status.get_handle()
        );
    }

    void get_device_index(int* out_device_index) const noexcept
    {
        m_ops->get_device_index(get_handle(), out_device_index);
    }

    void clone(
        const ice::sonic::TF_RandomGeneratorOps& out_clone,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->clone(get_handle(), out_clone.get_handle(), out_status.get_handle());
    }
};

} // namespace ice::sonic
