// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/random_generator/random_generator.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/random_generator/random_generator.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tensor.h"

export module cc_ice_extern_random_generator_builder:random_generator;

import std;
import cc_ice_extern_random_generator_sonic;
import cc_ice_intern_sonic;

export namespace ice::builder {

class TF_RandomGeneratorOps
{
public:
    explicit TF_RandomGeneratorOps(
        const ::TF_RandomGeneratorOps* TF_RandomGeneratorOps_ops,
        const ::TF_StatusOps* Status_ops,
        const ::TF_TensorOps* TF_TensorOps_ops
    ) noexcept :
        m_handle{.plugin_data = this}
    {
        m_TF_RandomGeneratorOps_ops = TF_RandomGeneratorOps_ops;
        m_Status_ops = Status_ops;
        m_TF_TensorOps_ops = TF_TensorOps_ops;
    }

    TF_RandomGeneratorOps(const TF_RandomGeneratorOps&) = delete;
    TF_RandomGeneratorOps& operator=(const TF_RandomGeneratorOps&) = delete;

    static TF_RandomGeneratorOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TF_RandomGeneratorOps*>(ctx);
    }

    template<typename HandleT>
    static TF_RandomGeneratorOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TF_RandomGeneratorOps*>(handle->plugin_data);
    }

    virtual ~TF_RandomGeneratorOps() = default;
    virtual void destroy() noexcept = 0;
    virtual void set_seed(uint64_t seed) noexcept = 0;
    virtual void get_seed(uint64_t* out_seed) noexcept = 0;
    virtual void reseed_nondeterministic(uint64_t* out_seed) noexcept = 0;
    virtual void set_offset(uint64_t offset) noexcept = 0;
    virtual void get_offset(uint64_t* out_offset) noexcept = 0;
    virtual void set_state(
        const ice::sonic::TF_TensorOps& state,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void
    get_state(TF_Tensor** out_state, const ice::sonic::Status& out_status) noexcept = 0;
    virtual void graphsafe_set_state(
        const ice::sonic::TF_RandomGeneratorOps& other,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void graphsafe_get_state(
        const ice::sonic::TF_RandomGeneratorOps& out_other,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void philox_state(
        uint64_t increment,
        TF_PhiloxState* out_state,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void philox_engine_inputs(
        uint64_t increment,
        uint64_t* out_seed,
        uint64_t* out_offset,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void get_device_index(int* out_device_index) noexcept = 0;
    virtual void clone(
        const ice::sonic::TF_RandomGeneratorOps& out_clone,
        const ice::sonic::Status& out_status
    ) noexcept = 0;

    void get_generic_vtable(void (*create)(::TF_RandomGenerator*)) noexcept
    {
        m_vtable = ::TF_RandomGeneratorOps{
            .struct_size = TF_OFFSET_OF_END(::TF_RandomGeneratorOps, clone),

            .create = create,
            .destroy =
                [](TF_RandomGenerator* handle) noexcept
            {
                auto& self = TF_RandomGeneratorOps::from_handle(handle);
                self.destroy();
            },
            .set_seed =
                [](TF_RandomGenerator* generator, uint64_t seed) noexcept
            {
                auto& self = TF_RandomGeneratorOps::from_handle(generator);
                self.set_seed(seed);
            },
            .get_seed =
                [](TF_RandomGenerator* generator, uint64_t* out_seed) noexcept
            {
                auto& self = TF_RandomGeneratorOps::from_handle(generator);
                self.get_seed(out_seed);
            },
            .reseed_nondeterministic =
                [](TF_RandomGenerator* generator, uint64_t* out_seed) noexcept
            {
                auto& self = TF_RandomGeneratorOps::from_handle(generator);
                self.reseed_nondeterministic(out_seed);
            },
            .set_offset =
                [](TF_RandomGenerator* generator, uint64_t offset) noexcept
            {
                auto& self = TF_RandomGeneratorOps::from_handle(generator);
                self.set_offset(offset);
            },
            .get_offset =
                [](TF_RandomGenerator* generator, uint64_t* out_offset) noexcept
            {
                auto& self = TF_RandomGeneratorOps::from_handle(generator);
                self.get_offset(out_offset);
            },
            .set_state =
                [](TF_RandomGenerator* generator,
                   const TF_Tensor* state,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_RandomGeneratorOps::from_handle(generator);
                self.set_state(
                    self.wrap(std::type_identity<ice::sonic::TF_TensorOps>{}, state),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_state =
                [](TF_RandomGenerator* generator,
                   TF_Tensor** out_state,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_RandomGeneratorOps::from_handle(generator);
                self.get_state(
                    out_state,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .graphsafe_set_state =
                [](TF_RandomGenerator* generator,
                   const TF_RandomGenerator* other,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_RandomGeneratorOps::from_handle(generator);
                self.graphsafe_set_state(
                    self.wrap(std::type_identity<ice::sonic::TF_RandomGeneratorOps>{}, other),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .graphsafe_get_state =
                [](TF_RandomGenerator* generator,
                   TF_RandomGenerator* out_other,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_RandomGeneratorOps::from_handle(generator);
                self.graphsafe_get_state(
                    self.wrap(std::type_identity<ice::sonic::TF_RandomGeneratorOps>{}, out_other),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .philox_state =
                [](TF_RandomGenerator* generator,
                   uint64_t increment,
                   TF_PhiloxState* out_state,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_RandomGeneratorOps::from_handle(generator);
                self.philox_state(
                    increment,
                    out_state,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .philox_engine_inputs =
                [](TF_RandomGenerator* generator,
                   uint64_t increment,
                   uint64_t* out_seed,
                   uint64_t* out_offset,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_RandomGeneratorOps::from_handle(generator);
                self.philox_engine_inputs(
                    increment,
                    out_seed,
                    out_offset,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_device_index =
                [](TF_RandomGenerator* generator, int* out_device_index) noexcept
            {
                auto& self = TF_RandomGeneratorOps::from_handle(generator);
                self.get_device_index(out_device_index);
            },
            .clone =
                [](TF_RandomGenerator* generator,
                   TF_RandomGenerator* out_clone,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_RandomGeneratorOps::from_handle(generator);
                self.clone(
                    self.wrap(std::type_identity<ice::sonic::TF_RandomGeneratorOps>{}, out_clone),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },

        };
    }

    ice::sonic::TF_RandomGeneratorOps wrap(
        std::type_identity<ice::sonic::TF_RandomGeneratorOps>,
        const ::TF_RandomGenerator* handle
    ) const noexcept
    {
        return ice::sonic::TF_RandomGeneratorOps{
            m_TF_RandomGeneratorOps_ops,
            const_cast<::TF_RandomGenerator*>(handle)
        };
    }

    ice::sonic::Status
    wrap(std::type_identity<ice::sonic::Status>, const ::TF_Status* handle) const noexcept
    {
        return ice::sonic::Status{m_Status_ops, const_cast<::TF_Status*>(handle)};
    }

    ice::sonic::TF_TensorOps
    wrap(std::type_identity<ice::sonic::TF_TensorOps>, const ::TF_Tensor* handle) const noexcept
    {
        return ice::sonic::TF_TensorOps{m_TF_TensorOps_ops, const_cast<::TF_Tensor*>(handle)};
    }

    const ::TF_RandomGeneratorOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const ::TF_RandomGenerator& get_handle() const noexcept
    {
        return m_handle;
    }

    template<typename Registry, typename StringType>
    void register_ops(
        Registry& registry,
        const StringType& type,
        const StringType& provider
    ) const noexcept
    {
        registry.register_op(type, provider, const_cast<::TF_RandomGeneratorOps*>(&m_vtable));
    }

private:
    ::TF_RandomGeneratorOps m_vtable;
    ::TF_RandomGenerator m_handle;

    const ::TF_RandomGeneratorOps* m_TF_RandomGeneratorOps_ops{nullptr};

    const ::TF_StatusOps* m_Status_ops{nullptr};

    const ::TF_TensorOps* m_TF_TensorOps_ops{nullptr};
};

} // namespace ice::builder
