// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/random_generator/random_generator.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/random_generator/random_generator.h"

export module cc_ice_extern_random_generator_builder:random_generator;

import std;

export namespace ice::builder {

class TF_RandomGeneratorOps
{
public:
    TF_RandomGeneratorOps() noexcept :
        m_handle{.plugin_data = this}
    {
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
    virtual void set_seed(uint64_t seed) noexcept = 0;
    virtual void get_seed(uint64_t* out_seed) noexcept = 0;
    virtual void reseed_nondeterministic(uint64_t* out_seed) noexcept = 0;
    virtual void set_offset(uint64_t offset) noexcept = 0;
    virtual void get_offset(uint64_t* out_offset) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    set_state(const ice::sonic::TF_TensorOps& state) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    get_state(TF_Tensor** out_state) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    graphsafe_set_state(const ice::sonic::TF_RandomGeneratorOps& other) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    graphsafe_get_state(const ice::sonic::TF_RandomGeneratorOps& out_other) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    philox_state(uint64_t increment, TF_PhiloxState* out_state) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    philox_engine_inputs(uint64_t increment, uint64_t* out_seed, uint64_t* out_offset) noexcept = 0;
    virtual void get_device_index(int* out_device_index) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    clone(const ice::sonic::TF_RandomGeneratorOps& out_clone) noexcept = 0;

    void get_generic_vtable() noexcept
    {
        m_vtable = ::TF_RandomGeneratorOps{
            .struct_size = TF_RANDOMGENERATOR_STRUCT_SIZE,
            .set_seed =
                [](TF_RandomGenerator* generator, uint64_t seed) noexcept
            {
                TF_RandomGeneratorOps::from_handle(generator).set_seed(seed);
            },
            .get_seed =
                [](TF_RandomGenerator* generator, uint64_t* out_seed) noexcept
            {
                TF_RandomGeneratorOps::from_handle(generator).get_seed(out_seed);
            },
            .reseed_nondeterministic =
                [](TF_RandomGenerator* generator, uint64_t* out_seed) noexcept
            {
                TF_RandomGeneratorOps::from_handle(generator).reseed_nondeterministic(out_seed);
            },
            .set_offset =
                [](TF_RandomGenerator* generator, uint64_t offset) noexcept
            {
                TF_RandomGeneratorOps::from_handle(generator).set_offset(offset);
            },
            .get_offset =
                [](TF_RandomGenerator* generator, uint64_t* out_offset) noexcept
            {
                TF_RandomGeneratorOps::from_handle(generator).get_offset(out_offset);
            },
            .set_state =
                [](TF_RandomGenerator* generator,
                   const TF_Tensor* state,
                   TF_Status* out_status) noexcept
            {
                auto res = TF_RandomGeneratorOps::from_handle(generator).set_state(
                    ice::sonic::TF_TensorOps::wrap(state)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_state =
                [](TF_RandomGenerator* generator,
                   TF_Tensor** out_state,
                   TF_Status* out_status) noexcept
            {
                auto res = TF_RandomGeneratorOps::from_handle(generator).get_state(out_state);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .graphsafe_set_state =
                [](TF_RandomGenerator* generator,
                   const TF_RandomGenerator* other,
                   TF_Status* out_status) noexcept
            {
                auto res = TF_RandomGeneratorOps::from_handle(generator).graphsafe_set_state(
                    ice::sonic::TF_RandomGeneratorOps::wrap(other)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .graphsafe_get_state =
                [](TF_RandomGenerator* generator,
                   TF_RandomGenerator* out_other,
                   TF_Status* out_status) noexcept
            {
                auto res = TF_RandomGeneratorOps::from_handle(generator).graphsafe_get_state(
                    ice::sonic::TF_RandomGeneratorOps::wrap(out_other)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .philox_state =
                [](TF_RandomGenerator* generator,
                   uint64_t increment,
                   TF_PhiloxState* out_state,
                   TF_Status* out_status) noexcept
            {
                auto res = TF_RandomGeneratorOps::from_handle(generator).philox_state(
                    increment,
                    out_state
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .philox_engine_inputs =
                [](TF_RandomGenerator* generator,
                   uint64_t increment,
                   uint64_t* out_seed,
                   uint64_t* out_offset,
                   TF_Status* out_status) noexcept
            {
                auto res = TF_RandomGeneratorOps::from_handle(generator)
                               .philox_engine_inputs(increment, out_seed, out_offset);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_device_index =
                [](TF_RandomGenerator* generator, int* out_device_index) noexcept
            {
                TF_RandomGeneratorOps::from_handle(generator).get_device_index(out_device_index);
            },
            .clone =
                [](TF_RandomGenerator* generator,
                   TF_RandomGenerator* out_clone,
                   TF_Status* out_status) noexcept
            {
                auto res = TF_RandomGeneratorOps::from_handle(generator).clone(
                    ice::sonic::TF_RandomGeneratorOps::wrap(out_clone)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },

        };
    }

    const ::TF_RandomGeneratorOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const TF_RandomGenerator& get_handle() const noexcept
    {
        return m_handle;
    }


private:
    ::TF_RandomGeneratorOps m_vtable;
    TF_RandomGenerator m_handle;
};

} // namespace ice::builder
