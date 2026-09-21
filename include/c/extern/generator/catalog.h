#ifndef CONGELADO_C_GENERATOR_CATALOG_H_
#define CONGELADO_C_GENERATOR_CATALOG_H_

#include "include/c/macros.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tensor.h"
#include "include/c/intern/tstring.h"
#include "include/c/extern/generator/module.h"

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct TFGeneratorCatalog
    {
        void* plugin_data;
    } TFGeneratorCatalog;

    typedef struct TFGeneratorCatalogOps
    {
        size_t struct_size;

        void (*add_module)(
            TFGeneratorCatalog* manager,
            TFGeneratorModule* module,
            TF_Status* out_status
        );

        void (*get_module)(TFGeneratorCatalog* manager, const TF_String* name, TFGeneratorModule* out_module, TF_Status* out_status);

        void (*list_modules)(TFGeneratorCatalog* manager, TF_Tensor** out_modules, TF_Status* out_status);
    } TFGeneratorCatalogOps;

#define TF_GENERATOR_CATALOG_STRUCT_SIZE TF_OFFSET_OF_END(TFGeneratorCatalogOps, list_modules)

    TF_CAPI_EXPORT void
    create_generator_catalog(TFGeneratorCatalogOps** ops, void** plugin_context, TF_Status* out_status);
    TF_CAPI_EXPORT void destroy_generator_catalog(void* plugin_context);

#ifdef __cplusplus
}
#endif

#endif // CONGELADO_C_GENERATOR_CATALOG_H_
