#ifndef CONGELADO_C_FILESYSTEM_RANDOM_ACCESS_FILE_H_
#define CONGELADO_C_FILESYSTEM_RANDOM_ACCESS_FILE_H_

#include "c/macros.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct TF_RandomAccessFile
    {
        void* plugin_data;
    } TF_RandomAccessFile;

    typedef struct TF_RandomAccessFileOps
    {
        size_t struct_size;
        void (*destroy)(TF_RandomAccessFile* file);
        void (*get_name)(void* plugin_context, TF_String* out);

        TF_RandomAccessFile* (*new_random_access_file)(
            void* plugin_context,
            const TF_String* path,
            TF_Status* status
        );

        int64_t (*read)(TF_RandomAccessFile* file, uint64_t offset, size_t n, char* buffer, TF_Status* status);
    } TF_RandomAccessFileOps;

#define TF_RANDOM_ACCESS_FILE_STRUCT_SIZE TF_OFFSET_OF_END(TF_RandomAccessFileOps, read)

    TF_CAPI_EXPORT void
    create_random_access_file(TF_RandomAccessFileOps** ops, void** plugin_context, TF_Status* status);
    TF_CAPI_EXPORT void destroy_random_access_file(void* plugin_context);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // CONGELADO_C_FILESYSTEM_RANDOM_ACCESS_FILE_H_
