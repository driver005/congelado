#ifndef CONGELADO_C_FILESYSTEM_TREE_H_
#define CONGELADO_C_FILESYSTEM_TREE_H_

#include "include/c/extern/filesystem/option_types.h"
#include "include/c/intern/file_statistics.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tensor.h"
#include "include/c/intern/tstring.h"
#include "include/c/macros.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct TFFilesystemTree
    {
        void* plugin_data;
    } TFFilesystemTree;

    typedef struct TFFilesystemTreeOps
    {
        size_t struct_size;
        void (*create)(TFFilesystemTree* out_handle);
        void (*destroy)(TFFilesystemTree* handle);
        void (*free_options)(
            TFFilesystemTree* manager,
            TFFilesystemOption* options,
            int num_options
        );

        void (*create_dir)(TFFilesystemTree* manager, const TF_String* path, TF_Status* out_status);
        void (*recursively_create_dir)(
            TFFilesystemTree* manager,
            const TF_String* path,
            TF_Status* out_status
        );
        void (*delete_file)(
            TFFilesystemTree* manager,
            const TF_String* path,
            TF_Status* out_status
        );
        void (*delete_dir)(TFFilesystemTree* manager, const TF_String* path, TF_Status* out_status);
        void (*delete_recursively)(
            TFFilesystemTree* manager,
            const TF_String* path,
            uint64_t* undeleted_files,
            uint64_t* undeleted_dirs,
            TF_Status* out_status
        );
        void (*rename_file)(
            TFFilesystemTree* manager,
            const TF_String* src,
            const TF_String* dst,
            TF_Status* out_status
        );
        void (*copy_file)(
            TFFilesystemTree* manager,
            const TF_String* src,
            const TF_String* dst,
            TF_Status* out_status
        );
        void (*path_exists)(
            TFFilesystemTree* manager,
            const TF_String* path,
            TF_Status* out_status
        );
        void (*paths_exist)(
            TFFilesystemTree* manager,
            const TF_String* paths,
            int num_paths,
            TF_Status* out_status
        );
        void (*stat)(
            TFFilesystemTree* manager,
            const TF_String* path,
            TF_FileStatistics* out_stats,
            TF_Status* out_status
        );
        void (*is_directory)(
            TFFilesystemTree* manager,
            const TF_String* path,
            int* out_is_directory,
            TF_Status* out_status
        );
        void (*get_file_size)(
            TFFilesystemTree* manager,
            const TF_String* path,
            int64_t* out_size,
            TF_Status* out_status
        );
        void (*translate_name)(
            TFFilesystemTree* manager,
            const TF_String* uri,
            TF_String* out_name
        );
        void (*get_children)(
            TFFilesystemTree* manager,
            const TF_String* path,
            TF_Tensor** out_children,
            TF_Status* out_status
        );
        void (*get_matching_paths)(
            TFFilesystemTree* manager,
            const TF_String* glob,
            TF_Tensor** out_matches,
            TF_Status* out_status
        );
        void (*flush_caches)(TFFilesystemTree* manager);
        void (*get_filesystem_configuration)(
            TFFilesystemTree* manager,
            TF_Tensor** out_config,
            TF_Status* out_status
        );
        void (*set_filesystem_configuration)(
            TFFilesystemTree* manager,
            const TF_Tensor* options,
            TF_Status* out_status
        );
        void (*get_filesystem_configuration_option)(
            TFFilesystemTree* manager,
            const TF_String* key,
            TFFilesystemOption* out_option,
            TF_Status* out_status
        );
        void (*set_filesystem_configuration_option)(
            TFFilesystemTree* manager,
            const TFFilesystemOption* option,
            TF_Status* out_status
        );
        void (*get_filesystem_configuration_keys)(
            TFFilesystemTree* manager,
            TF_Tensor** out_keys,
            TF_Status* out_status
        );
    } TFFilesystemTreeOps;

#define TF_FILESYSTEM_TREE_STRUCT_SIZE                                                             \
    TF_OFFSET_OF_END(TFFilesystemTreeOps, get_filesystem_configuration_keys)

    TF_CAPI_EXPORT void
    create_filesystem_tree(TFFilesystemTreeOps** ops, void** plugin_context, TF_Status* out_status);
    TF_CAPI_EXPORT void destroy_filesystem_tree(void* plugin_context);

#ifdef __cplusplus
}
#endif

#endif // CONGELADO_C_FILESYSTEM_TREE_H_
