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
#include "c/intern/tf_tstring.h"
#include "c/intern/tf_tensor.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif
    typedef struct TF_RandomAccessFile_Controller TF_RandomAccessFile_Controller;
    typedef struct TF_WritableFile_Controller TF_WritableFile_Controller;
    typedef struct TF_ReadOnlyMemoryRegion_Controller TF_ReadOnlyMemoryRegion_Controller;

    // Plugin-owned buffers returned by the accessors below (an entries array from
    // TF_Filesystem_GetChildren/GetMatchingPaths/GetFilesystemConfigurationKeys, or an options
    // array from TF_Filesystem_GetFilesystemConfiguration) are freed with these — same
    // "matching Free* utility" pattern as TF_Serde_FreeString/TF_Worker_FreeString.
    
    

    // --------------------------------------------------------------------------
    // Filesystem — one instance per URI scheme.

    
    

    
    
    
    

    
    
    
    
    
    
    

    
    // statuses — caller-allocated array of num_paths non-null TF_Status*, one per path, same
    // order as paths.
    

    
    
    

    

    // Returns the number of entries (same as TF_Filesystem_FreeStringArray's count) or -1 on
    // failure (see status); *out_entries left untouched on failure.
    
    

    

    // Returns the number of options (same as TF_Filesystem_FreeOptions's num_options) or -1 on
    // failure (see status); *out_options left untouched on failure.
    
    
    
    
    

    // --------------------------------------------------------------------------
    // RandomAccessFile — produced by TF_Filesystem_NewRandomAccessFile.

    
    

    // --------------------------------------------------------------------------
    // WritableFile — produced by TF_Filesystem_NewWritableFile/NewAppendableFile.

    
    
    
    
    
    

    // --------------------------------------------------------------------------
    // ReadOnlyMemoryRegion — produced by TF_Filesystem_NewReadOnlyMemoryRegionFromFile.

    
    
    

    
    typedef struct TF_Filesystem {
        size_t struct_size;
        void (*free_options)(TF_Filesystem_Option* options, int num_options);
        void (*destroy)(void* plugin_context);
        
        // RandomAccessFile
        void* (*new_random_access_file)(void* plugin_context, const TF_TString* path, TF_Status* status);
        void (*random_access_file__destroy)(void* file_context);
        int64_t (*random_access_file__read)(void* file_context, uint64_t offset, size_t n, char* buffer, TF_Status* status);
        
        // WritableFile
        void* (*new_writable_file)(void* plugin_context, const TF_TString* path, TF_Status* status);
        void* (*new_appendable_file)(void* plugin_context, const TF_TString* path, TF_Status* status);
        void (*writable_file__destroy)(void* file_context);
        void (*writable_file__append)(void* file_context, const TF_String* buffer, size_t n, TF_Status* status);
        int64_t (*writable_file__tell)(void* file_context, TF_Status* status);
        void (*writable_file__flush)(void* file_context, TF_Status* status);
        void (*writable_file__sync)(void* file_context, TF_Status* status);
        void (*writable_file__close)(void* file_context, TF_Status* status);
        
        // ReadOnlyMemoryRegion
        void* (*new_read_only_memory_region_from_file)(void* plugin_context, const TF_TString* path, TF_Status* status);
        void (*read_only_memory_region__destroy)(void* region_context);
        const void* (*read_only_memory_region__data)(void* region_context);
        uint64_t (*read_only_memory_region__length)(void* region_context);
        void (*create_dir)(void* plugin_context, const TF_TString* path, TF_Status* status);
        void (*recursively_create_dir)(void* plugin_context, const TF_TString* path, TF_Status* status);
        void (*delete_file)(void* plugin_context, const TF_TString* path, TF_Status* status);
        void (*delete_dir)(void* plugin_context, const TF_TString* path, TF_Status* status);
        void (*delete_recursively)(void* plugin_context, const TF_TString* path, uint64_t* undeleted_files, uint64_t* undeleted_dirs, TF_Status* status);
        void (*rename_file)(void* plugin_context, const TF_TString* src, const TF_TString* dst, TF_Status* status);
        void (*copy_file)(void* plugin_context, const TF_TString* src, const TF_TString* dst, TF_Status* status);
        void (*path_exists)(void* plugin_context, const TF_TString* path, TF_Status* status);
        void (*paths_exist)(void* plugin_context, const TF_Tensor_Handle* paths, TF_Tensor_Handle* out_statuses, TF_Status* status);
        void (*stat)(void* plugin_context, const TF_TString* path, TF_FileStatistics* out_stats, TF_Status* status);
        bool (*is_directory)(void* plugin_context, const TF_TString* path, TF_Status* status);
        int64_t (*get_file_size)(void* plugin_context, const TF_TString* path, TF_Status* status);
        void (*translate_name)(void* plugin_context, const TF_TString* uri, TF_String* out);
        TF_Tensor_Handle* (*get_children)(void* plugin_context, const TF_TString* path, TF_Status* status);
        TF_Tensor_Handle* (*get_matching_paths)(void* plugin_context, const TF_TString* glob, TF_Status* status);
        void (*flush_caches)(void* plugin_context);
        TF_Tensor_Handle* (*get_filesystem_configuration)(void* plugin_context, TF_Status* status);
        void (*set_filesystem_configuration)(void* plugin_context, const TF_Tensor_Handle* options, TF_Status* status);
        void (*get_filesystem_configuration_option)(void* plugin_context, const TF_TString* key, TF_Filesystem_Option* out_option, TF_Status* status);
        void (*set_filesystem_configuration_option)(void* plugin_context, const TF_Filesystem_Option* option, TF_Status* status);
        TF_Tensor_Handle* (*get_filesystem_configuration_keys)(void* plugin_context, TF_Status* status);
    } TF_Filesystem;

    TF_CAPI_EXPORT extern void TF_InitFilesystem(TF_Filesystem** ops, void** plugin_context, TF_Status* status);


#ifdef __cplusplus
}
#endif

#endif // CONGELADO_C_FILESYSTEM_CONTROLLER_H_
