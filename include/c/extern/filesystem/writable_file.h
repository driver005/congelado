#ifndef CONGELADO_C_FILESYSTEM_WRITABLE_FILE_H_
#define CONGELADO_C_FILESYSTEM_WRITABLE_FILE_H_

#include "c/macros.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct TF_WritableFile
    {
        void* plugin_data;
    } TF_WritableFile;

    typedef struct TF_WritableFileOps
    {
        size_t struct_size;
        void (*destroy)(TF_WritableFile* file);
        void (*get_name)(TF_WritableFile* file, TF_String* out_name);
        void (*append)(TF_WritableFile* file, const TF_String* buffer, TF_Status* out_status);
        void (*tell)(TF_WritableFile* file, int64_t* out_position, TF_Status* out_status);
        void (*flush)(TF_WritableFile* file, TF_Status* out_status);
        void (*sync)(TF_WritableFile* file, TF_Status* out_status);
        void (*close)(TF_WritableFile* file, TF_Status* out_status);
    } TF_WritableFileOps;

#define TF_WRITABLE_FILE_STRUCT_SIZE TF_OFFSET_OF_END(TF_WritableFileOps, close)

    TF_CAPI_EXPORT void
    create_writable_file(TF_WritableFileOps** ops, void** plugin_context, TF_Status* out_status);
    TF_CAPI_EXPORT void destroy_writable_file(void* plugin_context);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // CONGELADO_C_FILESYSTEM_WRITABLE_FILE_H_
