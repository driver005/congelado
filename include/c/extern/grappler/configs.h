#ifndef CONGELADO_C_GRAPPLER_CONFIGS_H_
#define CONGELADO_C_GRAPPLER_CONFIGS_H_

#include "include/c/macros.h"
#include "include/c/intern/status.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

    typedef enum TF_TriState {
        TF_TriState_Default = 0,
        TF_TriState_Off,
        TF_TriState_On,
    } TF_TriState;

    typedef enum TFGrapplerOptimizationLevel {
        TF_GRAPPLER_OPT_L0 = 0,
        TF_GRAPPLER_OPT_L1 = 1,
        TF_GRAPPLER_OPT_L2 = 2,
        TF_GRAPPLER_OPT_L3 = 3
    } TFGrapplerOptimizationLevel;

    typedef struct TFGrapplerOptimizerConfigs {
        size_t struct_size;
        void* ext;
        TF_TriState disable_model_pruning;
        TF_TriState implementation_selector;
        TF_TriState function_optimization;
        TF_TriState common_subgraph_elimination;
        TF_TriState arithmetic_optimization;
        TF_TriState debug_stripper;
        TF_TriState constant_folding;
        TF_TriState shape_optimization;
        TF_TriState auto_mixed_precision;
        TF_TriState auto_mixed_precision_onednn_bfloat16;
        TF_TriState auto_mixed_precision_mkl;
        TF_TriState pin_to_host_optimization;
        TF_TriState layout_optimizer;
        TF_TriState remapping;
        TF_TriState loop_optimization;
        TF_TriState dependency_optimization;
        TF_TriState auto_parallel;
        TF_TriState memory_optimization;
        TF_TriState scoped_allocator_optimization;
    } TFGrapplerOptimizerConfigs;

    #define TF_GRAPPLER_OPTIMIZER_CONFIGS_STRUCT_SIZE \
        TF_OFFSET_OF_END(TFGrapplerOptimizerConfigs, scoped_allocator_optimization)

    typedef struct TFGrapplerConfigs { void* plugin_data; } TFGrapplerConfigs;

    typedef struct TFGrapplerConfigsOps {
        size_t struct_size;
        void (*create)(TFGrapplerConfigs* out_handle);
        void (*destroy)(TFGrapplerConfigs* handle);
        void (*get_optimization_level)(TFGrapplerConfigs* configs, TFGrapplerOptimizationLevel* out_level);
        void (*set_optimization_level)(TFGrapplerConfigs* configs, TFGrapplerOptimizationLevel level);
        void (*get_optimizer_configs)(TFGrapplerConfigs* configs, TFGrapplerOptimizerConfigs* out_configs, TF_Status* out_status);
        void (*set_optimizer_configs)(TFGrapplerConfigs* configs, const TFGrapplerOptimizerConfigs* in_configs, TF_Status* out_status);
    } TFGrapplerConfigsOps;
    #define TF_GRAPPLER_CONFIGS_STRUCT_SIZE TF_OFFSET_OF_END(TFGrapplerConfigsOps, set_optimizer_configs)

    TF_CAPI_EXPORT void create_grappler_configs(TFGrapplerConfigsOps** ops, void** plugin_context, TF_Status* out_status);
    TF_CAPI_EXPORT void destroy_grappler_configs(void* plugin_context);

#ifdef __cplusplus
}
#endif
#endif
