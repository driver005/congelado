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
    static TF_RandomGeneratorOps* create(void* ctx) noexcept
    {
        return static_cast<TF_RandomGeneratorOps*>(ctx);
    }

    template<typename HandleT>
    static TF_RandomGeneratorOps* create(HandleT* handle) noexcept
    {
        return static_cast<TF_RandomGeneratorOps*>(handle->plugin_data);
    }

    virtual ~TF_RandomGeneratorOps() = default;
    [[nodiscard]] virtual std::expected<void, ice::Status> set_seed(uint64_t seed) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_seed(uint64_t* out_seed) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    reseed_nondeterministic(uint64_t* out_seed) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> set_offset(uint64_t offset) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_offset(uint64_t* out_offset) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    set_state(const ice::sonic::TF_TensorOps& state) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_state(TF_Tensor** out_state) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    graphsafe_set_state(const ice::sonic::TF_RandomGeneratorOps& other) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    graphsafe_get_state(const ice::sonic::TF_RandomGeneratorOps& out_other) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    philox_state(uint64_t increment, TF_PhiloxState* out_state) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    philox_engine_inputs(uint64_t increment, uint64_t* out_seed, uint64_t* out_offset) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_device_index(int* out_device_index) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    clone(const ice::sonic::TF_RandomGeneratorOps& out_clone) noexcept = 0;

    static TF_RandomGeneratorOps* get_generic_vtable()
    {
        static TF_RandomGeneratorOps vtable = {
            .struct_size = TF_RANDOMGENERATOR_STRUCT_SIZE,
            .set_seed =
                [](TF_RandomGenerator* generator, uint64_t seed) noexcept
            {
                auto* self = TF_RandomGeneratorOps::create(generator);
                auto res = self->set_seed(seed);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_seed =
                [](TF_RandomGenerator* generator, uint64_t* out_seed) noexcept
            {
                auto* self = TF_RandomGeneratorOps::create(generator);
                auto res = self->get_seed(out_seed);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .reseed_nondeterministic =
                [](TF_RandomGenerator* generator, uint64_t* out_seed) noexcept
            {
                auto* self = TF_RandomGeneratorOps::create(generator);
                auto res = self->reseed_nondeterministic(out_seed);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .set_offset =
                [](TF_RandomGenerator* generator, uint64_t offset) noexcept
            {
                auto* self = TF_RandomGeneratorOps::create(generator);
                auto res = self->set_offset(offset);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_offset =
                [](TF_RandomGenerator* generator, uint64_t* out_offset) noexcept
            {
                auto* self = TF_RandomGeneratorOps::create(generator);
                auto res = self->get_offset(out_offset);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .set_state =
                [](TF_RandomGenerator* generator,
                   const TF_Tensor* state,
                   TF_Status* out_status) noexcept
            {
                auto* self = TF_RandomGeneratorOps::create(generator);
                auto res = self->set_state(ice::sonic::TF_TensorOps::wrap(state));
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_state =
                [](TF_RandomGenerator* generator,
                   TF_Tensor** out_state,
                   TF_Status* out_status) noexcept
            {
                auto* self = TF_RandomGeneratorOps::create(generator);
                auto res = self->get_state(out_state);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .graphsafe_set_state =
                [](TF_RandomGenerator* generator,
                   const TF_RandomGenerator* other,
                   TF_Status* out_status) noexcept
            {
                auto* self = TF_RandomGeneratorOps::create(generator);
                auto res =
                    self->graphsafe_set_state(ice::sonic::TF_RandomGeneratorOps::wrap(other));
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .graphsafe_get_state =
                [](TF_RandomGenerator* generator,
                   TF_RandomGenerator* out_other,
                   TF_Status* out_status) noexcept
            {
                auto* self = TF_RandomGeneratorOps::create(generator);
                auto res =
                    self->graphsafe_get_state(ice::sonic::TF_RandomGeneratorOps::wrap(out_other));
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
                auto* self = TF_RandomGeneratorOps::create(generator);
                auto res = self->philox_state(increment, out_state);
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
                auto* self = TF_RandomGeneratorOps::create(generator);
                auto res = self->philox_engine_inputs(increment, out_seed, out_offset);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_device_index =
                [](TF_RandomGenerator* generator, int* out_device_index) noexcept
            {
                auto* self = TF_RandomGeneratorOps::create(generator);
                auto res = self->get_device_index(out_device_index);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .clone =
                [](TF_RandomGenerator* generator,
                   TF_RandomGenerator* out_clone,
                   TF_Status* out_status) noexcept
            {
                auto* self = TF_RandomGeneratorOps::create(generator);
                auto res = self->clone(ice::sonic::TF_RandomGeneratorOps::wrap(out_clone));
                if (!res) {
                    res.error().to_c(out_status);
                }
            },

        };

        return &vtable;
    }

    builder::String get_name() const noexcept
    {
        builder::String result;
        m_ops->get_name(get_handle(), result.get_handle());
        return result;
    }
};

} // namespace ice::builder
