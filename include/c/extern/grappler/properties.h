#ifndef CONGELADO_C_GRAPPLER_PROPERTIES_H_
#define CONGELADO_C_GRAPPLER_PROPERTIES_H_

#include "include/c/macros.h"
#include "include/c/intern/tstring.h"
#include "include/c/intern/status.h"
#include "include/c/intern/buffer.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

    typedef struct TFGrapplerProperties { void* plugin_data; } TFGrapplerProperties;

    typedef struct TFGrapplerPropertiesOps {
        size_t struct_size;
        void (*infer_statically)(TFGrapplerProperties* props, bool assume_valid_feeds, bool aggressive_shape_inference, bool include_input_tensor_values, bool include_output_tensor_values, TF_Status* out_status);
        void (*get_input_properties_size)(TFGrapplerProperties* props, const TF_String* name, int* out_num_values, TF_Status* out_status);
        void (*get_output_properties_size)(TFGrapplerProperties* props, const TF_String* name, int* out_num_values, TF_Status* out_status);
        void (*get_input_properties)(TFGrapplerProperties* props, const TF_String* name, TF_Buffer** out_properties, int num_values, TF_Status* out_status);
        void (*get_output_properties)(TFGrapplerProperties* props, const TF_String* name, TF_Buffer** out_properties, int num_values, TF_Status* out_status);
    } TFGrapplerPropertiesOps;
    #define TF_GRAPPLER_PROPERTIES_STRUCT_SIZE TF_OFFSET_OF_END(TFGrapplerPropertiesOps, get_output_properties)

    TF_CAPI_EXPORT void create_grappler_properties(TFGrapplerPropertiesOps** ops, void** plugin_context, TF_Status* out_status);
    TF_CAPI_EXPORT void destroy_grappler_properties(void* plugin_context);

#ifdef __cplusplus
}
#endif
#endif
