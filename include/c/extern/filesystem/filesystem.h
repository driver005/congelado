/* Copyright 2024 The Congelado Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/
#ifndef CONGELADO_C_FILESYSTEM_CONTROLLER_H_
#define CONGELADO_C_FILESYSTEM_CONTROLLER_H_

#include "include/c/extern/filesystem/tree.h"
#include "include/c/extern/filesystem/random_access_file.h"
#include "include/c/extern/filesystem/read_only_memory_region.h"
#include "include/c/extern/filesystem/writable_file.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"
#include "include/c/macros.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct TF_Filesystem
    {
        void* plugin_data;
        const TFFilesystemTreeOps* tree_ops;
        const TF_RandomAccessFileOps* random_access_file_ops;
        const TF_WritableFileOps* writable_file_ops;
        const TF_ReadOnlyMemoryRegionOps* read_only_memory_region_ops;
    } TF_Filesystem;

    typedef struct TF_FilesystemOps
    {
        size_t struct_size;
        void (*destroy)(TF_Filesystem* filesystem);
        void (*get_name)(TF_Filesystem* filesystem, TF_String* out_name);
    } TF_FilesystemOps;

#define TF_FILESYSTEM_STRUCT_SIZE                                                                  \
    TF_OFFSET_OF_END(TF_FilesystemOps, get_name)

    TF_CAPI_EXPORT void
    create_filesystem(TF_FilesystemOps** ops, void** plugin_context, TF_Status* out_status);
    TF_CAPI_EXPORT void destroy_filesystem(void* plugin_context);

    static inline void init_filesystem(TF_FilesystemOps** ops, TF_Filesystem* filesystem, TF_Status* out_status)
    {
        create_filesystem(ops, &filesystem->plugin_data, out_status);

        TFFilesystemTreeOps* tree_ops = NULL;
        create_filesystem_tree(&tree_ops, &filesystem->plugin_data, out_status);
        filesystem->tree_ops = tree_ops;

        TF_RandomAccessFileOps* random_access_file_ops = NULL;
        create_random_access_file(&random_access_file_ops, &filesystem->plugin_data, out_status);
        filesystem->random_access_file_ops = random_access_file_ops;

        TF_WritableFileOps* writable_file_ops = NULL;
        create_writable_file(&writable_file_ops, &filesystem->plugin_data, out_status);
        filesystem->writable_file_ops = writable_file_ops;

        TF_ReadOnlyMemoryRegionOps* read_only_memory_region_ops = NULL;
        create_read_only_memory_region(
            &read_only_memory_region_ops,
            &filesystem->plugin_data,
            out_status
        );
        filesystem->read_only_memory_region_ops = read_only_memory_region_ops;
    }

#ifdef __cplusplus
}
#endif

#endif // CONGELADO_C_FILESYSTEM_CONTROLLER_H_
