/* Copyright 2019 The TensorFlow Authors. All Rights Reserved.

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
#ifndef CONGELADO_C_FILESYSTEM_REGISTRATION_H_
#define CONGELADO_C_FILESYSTEM_REGISTRATION_H_

#include "c/extern/filesystem/filesystem_ops.h"
#include "c/extern/filesystem/option_types.h"
#include "c/extern/filesystem/random_access_file.h"
#include "c/extern/filesystem/read_only_memory_region.h"
#include "c/extern/filesystem/writable_file.h"

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

    /// SECTION 3. ABI and API compatibility
    /// ----------------------------------------------------------------------------
    ///
    /// In this section we define constants and macros to record versioning
    /// information for each of the structures in section 2: ABI and API versions
    /// and the number of functions in each of the function tables (which is
    /// automatically determined, so ignored for the rest of this comment).
    ///
    /// Since filesystem plugins are outside of TensorFlow's code tree, they are not
    /// tied with TensorFlow releases and should have their own versioning metadata
    /// in addition with the data discussed in this section. Each plugin author can
    /// use a custom scheme, but it should only relate to changes in plugin code.
    /// This section only touches metadata related to the versioning of this
    /// interface that is shared by all possible plugins.
    ///
    /// The API number increases whenever we break API compatibility while still
    /// maintaining ABI compatibility. This happens only in the following cases:
    ///   1. A new method is added _at the end_ of the function table.
    ///   2. Preconditions or postconditions for one operation in these function
    ///   table change. Note that only core TensorFlow is able to impose these
    ///   invariants (i.e., guarantee the preconditions before calling the operation
    ///   and check the postconditions after the operation returns). If plugins need
    ///   additional invariants, they should be checked on the plugin side and the
    ///   `status` out variable should be updated accordingly (e.g., to include
    ///   plugin version information that relates to the condition change).
    ///
    /// All other changes to the data structures (e.g., method removal, method
    /// reordering, argument reordering, adding or removing arguments, changing the
    /// type or the constness of a parameter, etc.) results in an ABI breakage.
    /// Thus, we should not do any of these types of changes, except, potentially,
    /// when we are releasing a new major version of TensorFlow. This is an escape
    /// hatch, to be used rarely, preferably only to cleanup these structures.
    /// Whenever we do these changes, the ABI number must be increased.
    ///
    /// Next section will detail how this metadata is used at plugin registration to
    /// only load compatible plugins and discard all others.

    // LINT.IfChange(random_access_file_ops_version)
    constexpr int TF_RANDOM_ACCESS_FILE_OPS_API = 0;
    constexpr int TF_RANDOM_ACCESS_FILE_OPS_ABI = 0;
    constexpr size_t TF_RANDOM_ACCESS_FILE_OPS_SIZE = sizeof(TF_RandomAccessFileOps);
    // LINT.ThenChange()

    // LINT.IfChange(writable_file_ops_version)
    constexpr int TF_WRITABLE_FILE_OPS_API = 0;
    constexpr int TF_WRITABLE_FILE_OPS_ABI = 0;
    constexpr size_t TF_WRITABLE_FILE_OPS_SIZE = sizeof(TF_WritableFileOps);
    // LINT.ThenChange()

    // LINT.IfChange(read_only_memory_region_ops_version)
    constexpr int TF_READ_ONLY_MEMORY_REGION_OPS_API = 0;
    constexpr int TF_READ_ONLY_MEMORY_REGION_OPS_ABI = 0;
    constexpr size_t TF_READ_ONLY_MEMORY_REGION_OPS_SIZE = sizeof(TF_ReadOnlyMemoryRegionOps);
    // LINT.ThenChange()

    // LINT.IfChange(filesystem_ops_version)
    constexpr int TF_FILESYSTEM_OPS_API = 0;
    constexpr int TF_FILESYSTEM_OPS_ABI = 0;
    constexpr size_t TF_FILESYSTEM_OPS_SIZE = sizeof(TF_FilesystemOps);

    // LINT.ThenChange()

    /// SECTION 4. Plugin registration and initialization
    /// ----------------------------------------------------------------------------
    ///
    /// In this section we define the API used by core TensorFlow to initialize a
    /// filesystem provided by a plugin. That is, we define the following:
    ///   * `TF_InitPlugin` function: must be present in the plugin shared object as
    ///     it will be called by core TensorFlow when the filesystem plugin is
    ///     loaded;
    ///   * `TF_FilesystemPluginOps` struct: used to transfer information between
    ///     plugins and core TensorFlow about the operations provided and metadata;
    ///   * `TF_FilesystemPluginInfo` struct: similar to the above structure, but
    ///     collects information about all the file schemes that the plugin provides
    ///     support for, as well as about the plugin's memory handling routines;
    ///   * `TF_SetFilesystemVersionMetadata` function: must be called by plugins in
    ///     their `TF_InitPlugin` to record the versioning information the plugins
    ///     are compiled against.
    ///
    /// The `TF_InitPlugin` function is used by plugins to set up the data
    /// structures that implement this interface, as presented in Section 2. In
    /// order to not have plugin shared objects call back symbols defined in core
    /// TensorFlow, `TF_InitPlugin` has a `TF_FilesystemPluginInfo` argument which
    /// the plugin must fill (using the `TF_SetFilesystemVersionMetadata` for the
    /// metadata and setting up all the supported operations and the URI schemes
    /// that are supported).

    /// This structure incorporates the operations defined in Section 2 and the
    /// metadata defined in section 3, allowing plugins to define different ops
    /// for different URI schemes.
    ///
    /// Every URI scheme is of the form "fs" for URIs of form "fs:///path/to/file".
    /// For local filesystems (i.e., when the URI is "/path/to/file"), the scheme
    /// must be "". The scheme must never be `nullptr`.
    ///
    /// Every plugin fills this in `TF_InitPlugin`, using the alocator passed as
    /// argument to allocate memory. After `TF_InitPlugin` finishes, core
    /// TensorFlow uses the information present in this to initialize filesystems
    /// for the URI schemes that the plugin requests.
    ///
    /// All pointers defined in this structure point to memory allocated by the DSO
    /// using an allocator provided by core TensorFlow when calling `TF_InitPlugin`.
    ///
    /// IMPORTANT: To maintain binary compatibility, the layout of this structure
    /// must not change! In the unlikely case that a new type of file needs to be
    /// supported, add the new ops and metadata at the end of the structure.
    typedef struct TF_FilesystemPluginOps
    {
        char* scheme;               // URI scheme (e.g., "file", "gcs", null-terminated, owned)
        int filesystem_ops_abi;     // Filesystem ops ABI version
        int filesystem_ops_api;     // Filesystem ops API version
        size_t filesystem_ops_size; // Size of filesystem_ops struct
        TF_FilesystemOps* filesystem_ops;               // Filesystem operations table
        int random_access_file_ops_abi;                 // RandomAccessFile ops ABI version
        int random_access_file_ops_api;                 // RandomAccessFile ops API version
        size_t random_access_file_ops_size;             // Size of random_access_file_ops struct
        TF_RandomAccessFileOps* random_access_file_ops; // RandomAccessFile operations table
        int writable_file_ops_abi;                      // WritableFile ops ABI version
        int writable_file_ops_api;                      // WritableFile ops API version
        size_t writable_file_ops_size;                  // Size of writable_file_ops struct
        TF_WritableFileOps* writable_file_ops;          // WritableFile operations table
        int read_only_memory_region_ops_abi;            // ReadOnlyMemoryRegion ops ABI version
        int read_only_memory_region_ops_api;            // ReadOnlyMemoryRegion ops API version
        size_t read_only_memory_region_ops_size; // Size of read_only_memory_region_ops struct
        TF_ReadOnlyMemoryRegionOps*
            read_only_memory_region_ops; // ReadOnlyMemoryRegion operations table
    } TF_FilesystemPluginOps;

    /// This structure gathers together all the operations provided by the plugin.
    ///
    /// Plugins must provide exactly `num_schemes` elements in the `ops` array.
    ///
    /// Since memory that is allocated by the DSO gets transferred to core
    /// TensorFlow, we need to provide a way for the allocation and deallocation to
    /// match. This is why this structure also defines `plugin_memory_allocate` and
    /// `plugin_memory_free` members.
    ///
    /// All memory allocated by the plugin that will be owned by core TensorFlow
    /// must be allocated using the allocator in this structure. Core TensorFlow
    /// will use the deallocator to free this memory once it no longer needs it.
    ///
    /// IMPORTANT: To maintain binary compatibility, the layout of this structure
    /// must not change! In the unlikely case that new global operations must be
    /// provided, add them at the end of the structure.
    typedef struct TF_FilesystemPluginInfo
    {
        size_t num_schemes;          // Number of URI schemes supported
        TF_FilesystemPluginOps* ops; // Array of operations per scheme
        TP_FilesystemPlugin_MemoryAllocate plugin_memory_allocate; // Memory allocation callback
        TP_FilesystemPlugin_MemoryFree plugin_memory_free;         // Memory deallocation callback
    } TF_FilesystemPluginInfo;

    // --------------------------------------------------------------------------
    // Plugin utility inlines

    // TF_FilesystemPluginOpsAlloc


    static inline TF_FilesystemPluginOps* TF_FilesystemPluginOpsNew(void)
    {
        TF_FilesystemPluginOps* ptr =
            (TF_FilesystemPluginOps*)malloc(sizeof(TF_FilesystemPluginOps));
        if (!ptr) {
            return nullptr;
        }
        ptr->scheme = nullptr;
        ptr->filesystem_ops_abi = 0;
        ptr->filesystem_ops_api = 0;
        ptr->filesystem_ops_size = 0;
        ptr->filesystem_ops = nullptr;
        ptr->random_access_file_ops_abi = 0;
        ptr->random_access_file_ops_api = 0;
        ptr->random_access_file_ops_size = 0;
        ptr->random_access_file_ops = nullptr;
        ptr->writable_file_ops_abi = 0;
        ptr->writable_file_ops_api = 0;
        ptr->writable_file_ops_size = 0;
        ptr->writable_file_ops = nullptr;
        ptr->read_only_memory_region_ops_abi = 0;
        ptr->read_only_memory_region_ops_api = 0;
        ptr->read_only_memory_region_ops_size = 0;
        ptr->read_only_memory_region_ops = nullptr;
        return ptr;
    }

    static inline void TF_FilesystemPluginOpsDelete(TF_FilesystemPluginOps* ptr)
    {
        if (!ptr) {
            return;
        }
        free(ptr);
    }

    static inline void
    TF_FilesystemPluginOps_SetScheme(TF_FilesystemPluginOps* builder, char* scheme)
    {
        builder->scheme = scheme;
    }

    static inline void
    TF_FilesystemPluginOps_SetFilesystemOps(TF_FilesystemPluginOps* builder, TF_FilesystemOps* ops)
    {
        builder->filesystem_ops = ops;
        builder->filesystem_ops_abi = TF_FILESYSTEM_OPS_ABI;
        builder->filesystem_ops_api = TF_FILESYSTEM_OPS_API;
        builder->filesystem_ops_size = TF_FILESYSTEM_OPS_SIZE;
    }

    static inline void TF_FilesystemPluginOps_SetRandomAccessFileOps(
        TF_FilesystemPluginOps* builder, TF_RandomAccessFileOps* ops
    )
    {
        builder->random_access_file_ops = ops;
        builder->random_access_file_ops_abi = TF_RANDOM_ACCESS_FILE_OPS_ABI;
        builder->random_access_file_ops_api = TF_RANDOM_ACCESS_FILE_OPS_API;
        builder->random_access_file_ops_size = TF_RANDOM_ACCESS_FILE_OPS_SIZE;
    }

    static inline void TF_FilesystemPluginOps_SetWritableFileOps(
        TF_FilesystemPluginOps* builder, TF_WritableFileOps* ops
    )
    {
        builder->writable_file_ops = ops;
        builder->writable_file_ops_abi = TF_WRITABLE_FILE_OPS_ABI;
        builder->writable_file_ops_api = TF_WRITABLE_FILE_OPS_API;
        builder->writable_file_ops_size = TF_WRITABLE_FILE_OPS_SIZE;
    }

    static inline void TF_FilesystemPluginOps_SetReadOnlyMemoryRegionOps(
        TF_FilesystemPluginOps* builder, TF_ReadOnlyMemoryRegionOps* ops
    )
    {
        builder->read_only_memory_region_ops = ops;
        builder->read_only_memory_region_ops_abi = TF_READ_ONLY_MEMORY_REGION_OPS_ABI;
        builder->read_only_memory_region_ops_api = TF_READ_ONLY_MEMORY_REGION_OPS_API;
        builder->read_only_memory_region_ops_size = TF_READ_ONLY_MEMORY_REGION_OPS_SIZE;
    }

    // TF_FilesystemPluginInfoAlloc


    static inline TF_FilesystemPluginInfo* TF_FilesystemPluginInfoNew(void)
    {
        TF_FilesystemPluginInfo* ptr =
            (TF_FilesystemPluginInfo*)malloc(sizeof(TF_FilesystemPluginInfo));
        if (!ptr) {
            return nullptr;
        }
        ptr->num_schemes = 0;
        ptr->ops = nullptr;
        ptr->plugin_memory_allocate = nullptr;
        ptr->plugin_memory_free = nullptr;
        return ptr;
    }

    static inline void TF_FilesystemPluginInfoDelete(TF_FilesystemPluginInfo* ptr)
    {
        if (!ptr) {
            return;
        }
        free(ptr);
    }

    static inline void
    TF_FilesystemPluginInfo_SetNumSchemes(TF_FilesystemPluginInfo* builder, size_t num_schemes)
    {
        builder->num_schemes = num_schemes;
    }

    static inline void
    TF_FilesystemPluginInfo_SetOps(TF_FilesystemPluginInfo* builder, TF_FilesystemPluginOps* ops)
    {
        builder->ops = ops;
    }

    static inline void TF_FilesystemPluginInfo_SetMemoryAllocator(
        TF_FilesystemPluginInfo* builder,
        TP_FilesystemPlugin_MemoryAllocate alloc,
        TP_FilesystemPlugin_MemoryFree dealloc
    )
    {
        builder->plugin_memory_allocate = alloc;
        builder->plugin_memory_free = dealloc;
    }

    /// Convenience function for setting the versioning metadata.
    ///
    /// The argument is guaranteed to not be `nullptr`.
    ///
    /// We want this to be defined in the plugin's memory space and we guarantee
    /// that core TensorFlow will never call this.
    static inline void TF_SetFilesystemVersionMetadata(TF_FilesystemPluginOps* ops)
    {
        ops->filesystem_ops_abi = TF_FILESYSTEM_OPS_ABI;
        ops->filesystem_ops_api = TF_FILESYSTEM_OPS_API;
        ops->filesystem_ops_size = TF_FILESYSTEM_OPS_SIZE;
        ops->random_access_file_ops_abi = TF_RANDOM_ACCESS_FILE_OPS_ABI;
        ops->random_access_file_ops_api = TF_RANDOM_ACCESS_FILE_OPS_API;
        ops->random_access_file_ops_size = TF_RANDOM_ACCESS_FILE_OPS_SIZE;
        ops->writable_file_ops_abi = TF_WRITABLE_FILE_OPS_ABI;
        ops->writable_file_ops_api = TF_WRITABLE_FILE_OPS_API;
        ops->writable_file_ops_size = TF_WRITABLE_FILE_OPS_SIZE;
        ops->read_only_memory_region_ops_abi = TF_READ_ONLY_MEMORY_REGION_OPS_ABI;
        ops->read_only_memory_region_ops_api = TF_READ_ONLY_MEMORY_REGION_OPS_API;
        ops->read_only_memory_region_ops_size = TF_READ_ONLY_MEMORY_REGION_OPS_SIZE;
    }

    /// Initializes a TensorFlow plugin.
    ///
    /// Must be implemented by the plugin DSO. It is called by TensorFlow runtime.
    ///
    /// Filesystem plugins can be loaded on demand by users via
    /// `Env::LoadLibrary` or during TensorFlow's startup if they are on certain
    /// paths (although this has a security risk if two plugins register for the
    /// same filesystem and the malicious one loads before the legimitate one -
    /// but we consider this to be something that users should care about and
    /// manage themselves). In both of these cases, core TensorFlow looks for
    /// the `TF_InitPlugin` symbol and calls this function.
    ///
    /// For every filesystem URI scheme that this plugin supports, the plugin must
    /// add one `TF_FilesystemPluginInfo` entry in `plugin_info->ops` and call
    /// `TF_SetFilesystemVersionMetadata` for that entry.
    ///
    /// Plugins must also initialize `plugin_info->plugin_memory_allocate` and
    /// `plugin_info->plugin_memory_free` to ensure memory allocated by plugin is
    /// freed in a compatible way.
    TF_CAPI_EXPORT extern void TF_InitPlugin(TF_FilesystemPluginInfo* plugin_info);

#ifdef __cplusplus
} // end extern "C"
#endif // __cplusplus

#endif // CONGELADO_C_FILESYSTEM_REGISTRATION_H_
