#ifndef CONGELADO_C_EXTERN_RANDOM_GENERATOR_RANDOM_GENERATOR_H_
#define CONGELADO_C_EXTERN_RANDOM_GENERATOR_RANDOM_GENERATOR_H_

#include "include/c/macros.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tensor.h"

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

    typedef struct TF_RandomGenerator { void* plugin_data; } TF_RandomGenerator;

    // Philox kernel arguments. When captured is false use seed/offset by value, otherwise read the values through seed_ptr/offset_ptr at replay time.
    typedef struct TF_PhiloxState {
        size_t struct_size;
        uint64_t seed;
        uint64_t offset;
        const int64_t* seed_ptr;
        const int64_t* offset_ptr;
        uint32_t offset_intragraph;
        bool captured;
    } TF_PhiloxState;

    // TF_RandomGeneratorOps — created by TF_ExecutorOps::create_random_generator_internal. Replaces XPUGeneratorImpl. Not to be confused with TF_Generator (code-gen catalog).
    typedef struct TF_RandomGeneratorOps {
        size_t struct_size;
        void (*set_seed)(TF_RandomGenerator* generator, uint64_t seed);
        void (*get_seed)(TF_RandomGenerator* generator, uint64_t* out_seed);
        void (*reseed_nondeterministic)(TF_RandomGenerator* generator, uint64_t* out_seed);
        void (*set_offset)(TF_RandomGenerator* generator, uint64_t offset);
        void (*get_offset)(TF_RandomGenerator* generator, uint64_t* out_offset);
        void (*set_state)(TF_RandomGenerator* generator, const TF_Tensor* state, TF_Status* out_status);
        void (*get_state)(TF_RandomGenerator* generator, TF_Tensor** out_state, TF_Status* out_status);
        void (*graphsafe_set_state)(TF_RandomGenerator* generator, const TF_RandomGenerator* other, TF_Status* out_status);
        void (*graphsafe_get_state)(TF_RandomGenerator* generator, TF_RandomGenerator* out_other, TF_Status* out_status);
        void (*philox_state)(TF_RandomGenerator* generator, uint64_t increment, TF_PhiloxState* out_state, TF_Status* out_status);
        void (*philox_engine_inputs)(TF_RandomGenerator* generator, uint64_t increment, uint64_t* out_seed, uint64_t* out_offset, TF_Status* out_status);
        void (*get_device_index)(TF_RandomGenerator* generator, int* out_device_index);
        void (*clone)(TF_RandomGenerator* generator, TF_RandomGenerator* out_clone, TF_Status* out_status);
    } TF_RandomGeneratorOps;
    #define TF_RANDOM_GENERATOR_STRUCT_SIZE TF_OFFSET_OF_END(TF_RandomGeneratorOps, clone)

    TF_CAPI_EXPORT void create_random_generator(TF_RandomGeneratorOps** ops, void** plugin_context, TF_Status* out_status);
    TF_CAPI_EXPORT void destroy_random_generator(void* plugin_context);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif  // CONGELADO_C_EXTERN_RANDOM_GENERATOR_RANDOM_GENERATOR_H_
