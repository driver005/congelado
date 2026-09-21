#ifndef CONGELADO_C_GRAPPLER_ITEM_H_
#define CONGELADO_C_GRAPPLER_ITEM_H_

#include "include/c/macros.h"
#include "include/c/intern/status.h"

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

    typedef struct TFGrapplerItem { void* plugin_data; } TFGrapplerItem;

    typedef struct TFGrapplerItemOps {
        size_t struct_size;
        void (*get_nodes_to_preserve_size)(TFGrapplerItem* item, int* out_num_values, size_t* out_storage_size, TF_Status* out_status);
        void (*get_nodes_to_preserve_list)(TFGrapplerItem* item, char** out_values, size_t* out_lengths, int num_values, void* storage, size_t storage_size, TF_Status* out_status);
        void (*get_fetch_nodes_size)(TFGrapplerItem* item, int* out_num_values, size_t* out_storage_size, TF_Status* out_status);
        void (*get_fetch_nodes_list)(TFGrapplerItem* item, char** out_values, size_t* out_lengths, int num_values, void* storage, size_t storage_size, TF_Status* out_status);
    } TFGrapplerItemOps;
    #define TF_GRAPPLER_ITEM_STRUCT_SIZE TF_OFFSET_OF_END(TFGrapplerItemOps, get_fetch_nodes_list)

    TF_CAPI_EXPORT void create_grappler_item(TFGrapplerItemOps** ops, void** plugin_context, TF_Status* out_status);
    TF_CAPI_EXPORT void destroy_grappler_item(void* plugin_context);

#ifdef __cplusplus
}
#endif
#endif
