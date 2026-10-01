#ifndef CONGELADO_C_GRAPPLER_FUNCTION_LIBRARY_H_
#define CONGELADO_C_GRAPPLER_FUNCTION_LIBRARY_H_

#include "include/c/macros.h"
#include "include/c/intern/tstring.h"
#include "include/c/intern/status.h"
#include "include/c/intern/buffer.h"

#ifdef __cplusplus
extern "C" {
#endif

    typedef struct TFGrapplerFunctionLibrary { void* plugin_data; } TFGrapplerFunctionLibrary;

    typedef struct TFGrapplerFunctionLibraryOps {
        size_t struct_size;
        void (*create)(TFGrapplerFunctionLibrary* out_handle);
        void (*destroy)(TFGrapplerFunctionLibrary* handle);
        void (*look_up_op_def)(TFGrapplerFunctionLibrary* lib, const TF_String* name, TF_Buffer* out_buf, TF_Status* out_status);
    } TFGrapplerFunctionLibraryOps;
    #define TF_GRAPPLER_FUNCTION_LIBRARY_STRUCT_SIZE TF_OFFSET_OF_END(TFGrapplerFunctionLibraryOps, look_up_op_def)

    TF_CAPI_EXPORT void create_grappler_function_library(TFGrapplerFunctionLibraryOps** ops, void** plugin_context, TF_Status* out_status);
    TF_CAPI_EXPORT void destroy_grappler_function_library(void* plugin_context);

#ifdef __cplusplus
}
#endif
#endif
