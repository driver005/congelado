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

#include "c/extern/filesystem/option_types.h"
#include "c/extern/filesystem/random_access_file.h"
#include "c/extern/filesystem/read_only_memory_region.h"
#include "c/extern/filesystem/writable_file.h"
#include "c/intern/tf_file_statistics.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tensor.h"
#include "c/intern/tf_tstring.h"
#include "c/macros.h"

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
        const TF_RandomAccessFileOps* random_access_file_ops;
        const TF_WritableFileOps* writable_file_ops;
        const TF_ReadOnlyMemoryRegionOps* read_only_memory_region_ops;
    } TF_Filesystem;

    typedef struct TF_FilesystemOps
    {
        size_t struct_size;
        void (*destroy)(TF_Filesystem* filesystem);
        void (*get_name)(TF_Filesystem* filesystem, TF_String* out_name);
        void (*free_options)(
            TF_Filesystem* filesystem,
            TFFilesystemOption* options,
            int num_options
        );

        void (*create_dir)(TF_Filesystem* filesystem, const TF_String* path, TF_Status* out_status);
        void (*recursively_create_dir)(
            TF_Filesystem* filesystem,
            const TF_String* path,
            TF_Status* out_status
        );
        void (*delete_file)(TF_Filesystem* filesystem, const TF_String* path, TF_Status* out_status);
        void (*delete_dir)(TF_Filesystem* filesystem, const TF_String* path, TF_Status* out_status);
        void (*delete_recursively)(
            TF_Filesystem* filesystem,
            const TF_String* path,
            uint64_t* undeleted_files,
            uint64_t* undeleted_dirs,
            TF_Status* out_status
        );
        void (*rename_file)(
            TF_Filesystem* filesystem,
            const TF_String* src,
            const TF_String* dst,
            TF_Status* out_status
        );
        void (*copy_file)(
            TF_Filesystem* filesystem,
            const TF_String* src,
            const TF_String* dst,
            TF_Status* out_status
        );
        void (*path_exists)(TF_Filesystem* filesystem, const TF_String* path, TF_Status* out_status);
        void (*paths_exist)(
            TF_Filesystem* filesystem,
            const TF_String* paths,
            int num_paths,
            TF_Status* out_status
        );
        void (*stat)(
            TF_Filesystem* filesystem,
            const TF_String* path,
            TF_FileStatistics* out_stats,
            TF_Status* out_status
        );
        void (*is_directory)(TF_Filesystem* filesystem, const TF_String* path, int* out_is_directory, TF_Status* out_status);
        void (*get_file_size)(
            TF_Filesystem* filesystem,
            const TF_String* path,
            int64_t* out_size,
            TF_Status* out_status
        );
        void (*translate_name)(TF_Filesystem* filesystem, const TF_String* uri, TF_String* out_name);
        void (*get_children)(
            TF_Filesystem* filesystem,
            const TF_String* path,
            TF_Tensor** out_children,
            TF_Status* out_status
        );
        void (*get_matching_paths)(
            TF_Filesystem* filesystem,
            const TF_String* glob,
            TF_Tensor** out_matches,
            TF_Status* out_status
        );
        void (*flush_caches)(TF_Filesystem* filesystem);
        void (*get_filesystem_configuration)(TF_Filesystem* filesystem, TF_Tensor** out_config, TF_Status* out_status);
        void (*set_filesystem_configuration)(
            TF_Filesystem* filesystem,
            const TF_Tensor* options,
            TF_Status* out_status
        );
        void (*get_filesystem_configuration_option)(
            TF_Filesystem* filesystem,
            const TF_String* key,
            TFFilesystemOption* out_option,
            TF_Status* out_status
        );
        void (*set_filesystem_configuration_option)(
            TF_Filesystem* filesystem,
            const TFFilesystemOption* option,
            TF_Status* out_status
        );
        void (*get_filesystem_configuration_keys)(
            TF_Filesystem* filesystem,
            TF_Tensor** out_keys,
            TF_Status* out_status
        );
    } TF_FilesystemOps;

#define TF_FILESYSTEM_STRUCT_SIZE                                                                  \
    TF_OFFSET_OF_END(TF_FilesystemOps, get_filesystem_configuration_keys)

    TF_CAPI_EXPORT void
    create_filesystem(TF_FilesystemOps** ops, void** plugin_context, TF_Status* out_status);
    TF_CAPI_EXPORT void destroy_filesystem(void* plugin_context);

    static inline void init_filesystem(TF_FilesystemOps** ops, TF_Filesystem* filesystem, TF_Status* out_status)
    {
        create_filesystem(ops, &filesystem->plugin_data, out_status);

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
