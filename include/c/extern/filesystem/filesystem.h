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

#include "c/abi/macros.h"
#include "c/extern/filesystem/option_types.h"
#include "c/intern/tf_file_statistics.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tensor.h"
#include "c/intern/tf_tstring.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    // Opaque file-handle types: implementations allocate the underlying objects and
    // hand back pointers; the host only ever passes them back to the vtable slots.
    typedef struct TF_RandomAccessFile TF_RandomAccessFile;
    typedef struct TF_WritableFile TF_WritableFile;
    typedef struct TF_ReadOnlyMemoryRegion TF_ReadOnlyMemoryRegion;

    // --------------------------------------------------------------------------
    // Error channel: TF_Status* is the sole error channel for every fallible slot.
    // For value-returning slots (bool, int64_t, ...) the returned value is the
    // query result and is meaningful only when status is OK; on failure the value
    // is unspecified and the error lives in *status. Slots with no TF_Status*
    // parameter are pure accessors that cannot fail.
    // --------------------------------------------------------------------------

    // --------------------------------------------------------------------------
    // Filesystem — one instance per URI scheme.

    typedef struct TF_Filesystem
    {
        size_t struct_size;
        void (*destroy)(void* plugin_context);
        void (*get_name)(void* plugin_context, TF_String* out);
        // The only raw-array free here: entries arrays returned by the getters below
        // (as TF_Tensor_Handle) are freed with the tensor runtime's delete.
        void (*free_options)(void* plugin_context, TF_Filesystem_Option* options, int num_options);

        // RandomAccessFile
        TF_RandomAccessFile* (*create_random_access_file)(
            void* plugin_context,
            const TF_TString* path,
            TF_Status* status
        );
        void (*random_access_file__destroy)(TF_RandomAccessFile* file_context);
        int64_t (*random_access_file__read)(
            TF_RandomAccessFile* file_context,
            uint64_t offset,
            size_t n,
            char* buffer,
            TF_Status* status
        );

        // WritableFile
        TF_WritableFile* (*create_writable_file)(void* plugin_context, const TF_TString* path, TF_Status* status);
        TF_WritableFile* (*create_appendable_file)(
            void* plugin_context,
            const TF_TString* path,
            TF_Status* status
        );
        void (*writable_file__destroy)(TF_WritableFile* file_context);
        void (*writable_file__append)(
            TF_WritableFile* file_context,
            const TF_TString* buffer,
            TF_Status* status
        );
        int64_t (*writable_file__tell)(TF_WritableFile* file_context, TF_Status* status);
        void (*writable_file__flush)(TF_WritableFile* file_context, TF_Status* status);
        void (*writable_file__sync)(TF_WritableFile* file_context, TF_Status* status);
        void (*writable_file__close)(TF_WritableFile* file_context, TF_Status* status);

        // ReadOnlyMemoryRegion
        TF_ReadOnlyMemoryRegion* (*create_read_only_memory_region_from_file)(
            void* plugin_context,
            const TF_TString* path,
            TF_Status* status
        );
        void (*read_only_memory_region__destroy)(TF_ReadOnlyMemoryRegion* region_context);
        const void* (*read_only_memory_region__data)(TF_ReadOnlyMemoryRegion* region_context);
        uint64_t (*read_only_memory_region__length)(TF_ReadOnlyMemoryRegion* region_context);

        void (*create_dir)(void* plugin_context, const TF_TString* path, TF_Status* status);
        void (*recursively_create_dir)(
            void* plugin_context,
            const TF_TString* path,
            TF_Status* status
        );
        void (*delete_file)(void* plugin_context, const TF_TString* path, TF_Status* status);
        void (*delete_dir)(void* plugin_context, const TF_TString* path, TF_Status* status);
        void (*delete_recursively)(
            void* plugin_context,
            const TF_TString* path,
            uint64_t* undeleted_files,
            uint64_t* undeleted_dirs,
            TF_Status* status
        );
        void (*rename_file)(
            void* plugin_context,
            const TF_TString* src,
            const TF_TString* dst,
            TF_Status* status
        );
        void (*copy_file)(
            void* plugin_context,
            const TF_TString* src,
            const TF_TString* dst,
            TF_Status* status
        );
        void (*path_exists)(void* plugin_context, const TF_TString* path, TF_Status* status);
        // `paths` points to `num_paths` strings. Fails with the first error; on
        // success all paths exist.
        void (*paths_exist)(
            void* plugin_context,
            const TF_TString* paths,
            int num_paths,
            TF_Status* status
        );
        void (*stat)(
            void* plugin_context,
            const TF_TString* path,
            TF_FileStatistics* out_stats,
            TF_Status* status
        );
        bool (*is_directory)(void* plugin_context, const TF_TString* path, TF_Status* status);
        int64_t (*get_file_size)(void* plugin_context, const TF_TString* path, TF_Status* status);
        void (*translate_name)(void* plugin_context, const TF_TString* uri, TF_String* out);
        // Entries arrays returned by the getters below are freed with free_options.
        TF_Tensor_Handle* (*get_children)(
            void* plugin_context,
            const TF_TString* path,
            TF_Status* status
        );
        TF_Tensor_Handle* (*get_matching_paths)(
            void* plugin_context,
            const TF_TString* glob,
            TF_Status* status
        );
        void (*flush_caches)(void* plugin_context);
        TF_Tensor_Handle* (*get_filesystem_configuration)(void* plugin_context, TF_Status* status);
        void (*set_filesystem_configuration)(
            void* plugin_context,
            const TF_Tensor_Handle* options,
            TF_Status* status
        );
        void (*get_filesystem_configuration_option)(
            void* plugin_context,
            const TF_TString* key,
            TF_Filesystem_Option* out_option,
            TF_Status* status
        );
        void (*set_filesystem_configuration_option)(
            void* plugin_context,
            const TF_Filesystem_Option* option,
            TF_Status* status
        );
        TF_Tensor_Handle* (*get_filesystem_configuration_keys)(
            void* plugin_context,
            TF_Status* status
        );
    } TF_Filesystem;

#define TF_FILESYSTEM_STRUCT_SIZE TF_OFFSET_OF_END(TF_Filesystem, get_filesystem_configuration_keys)

    TF_CAPI_EXPORT void
    init_filesystem(TF_Filesystem** ops, void** plugin_context, TF_Status* status);

#ifdef __cplusplus
}
#endif

#endif // CONGELADO_C_FILESYSTEM_CONTROLLER_H_
