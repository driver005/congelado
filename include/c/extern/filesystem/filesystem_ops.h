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
#ifndef CONGELADO_C_FILESYSTEM_FILESYSTEM_OPS_H_
#define CONGELADO_C_FILESYSTEM_FILESYSTEM_OPS_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "c/intern/tf_file_statistics.h"
#include "c/intern/tf_status.h"
#include "c/extern/filesystem/random_access_file.h"
#include "c/extern/filesystem/writable_file.h"
#include "c/extern/filesystem/read_only_memory_region.h"
#include "c/extern/filesystem/option_types.h"

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

/// Opaque data structure to hold plugin-specific data for a filesystem.
///
/// The wrapper data structure is owned by core TensorFlow. The data pointed to
/// by the `void*` member is always owned by the plugin.
///
/// Plugins will never receive a `TF_Filesystem*` that is `nullptr`. Core
/// TensorFlow will never touch the `void*` wrapped by this structure, except
/// to initialize it as `nullptr`.

typedef struct TF_Filesystem {
  void* plugin_filesystem;
} TF_Filesystem;

// --------------------------------------------------------------------------
// Filesystem callback types

typedef void (*TP_Filesystem_Init)(TF_Filesystem* filesystem, TF_Status* status);
typedef void (*TP_Filesystem_Cleanup)(TF_Filesystem* filesystem);
typedef void (*TP_Filesystem_NewRandomAccessFile)(const TF_Filesystem* filesystem,
    const char* path, TF_RandomAccessFile* file, TF_Status* status);
typedef void (*TP_Filesystem_NewWritableFile)(const TF_Filesystem* filesystem,
    const char* path, TF_WritableFile* file, TF_Status* status);
typedef void (*TP_Filesystem_NewAppendableFile)(const TF_Filesystem* filesystem,
    const char* path, TF_WritableFile* file, TF_Status* status);
typedef void (*TP_Filesystem_NewReadOnlyMemoryRegionFromFile)(
    const TF_Filesystem* filesystem, const char* path,
    TF_ReadOnlyMemoryRegion* region, TF_Status* status);
typedef void (*TP_Filesystem_CreateDir)(const TF_Filesystem* filesystem,
    const char* path, TF_Status* status);
typedef void (*TP_Filesystem_RecursivelyCreateDir)(const TF_Filesystem* filesystem,
    const char* path, TF_Status* status);
typedef void (*TP_Filesystem_DeleteFile)(const TF_Filesystem* filesystem,
    const char* path, TF_Status* status);
typedef void (*TP_Filesystem_DeleteDir)(const TF_Filesystem* filesystem,
    const char* path, TF_Status* status);
typedef void (*TP_Filesystem_DeleteRecursively)(const TF_Filesystem* filesystem,
    const char* path, uint64_t* undeleted_files, uint64_t* undeleted_dirs,
    TF_Status* status);
typedef void (*TP_Filesystem_RenameFile)(const TF_Filesystem* filesystem,
    const char* src, const char* dst, TF_Status* status);
typedef void (*TP_Filesystem_CopyFile)(const TF_Filesystem* filesystem,
    const char* src, const char* dst, TF_Status* status);
typedef void (*TP_Filesystem_PathExists)(const TF_Filesystem* filesystem,
    const char* path, TF_Status* status);
typedef bool (*TP_Filesystem_PathsExist)(const TF_Filesystem* filesystem,
    char** paths, int num_files, TF_Status** statuses);
typedef void (*TP_Filesystem_Stat)(const TF_Filesystem* filesystem,
    const char* path, TF_FileStatistics* stats, TF_Status* status);
typedef bool (*TP_Filesystem_IsDirectory)(const TF_Filesystem* filesystem,
    const char* path, TF_Status* status);
typedef int64_t (*TP_Filesystem_GetFileSize)(const TF_Filesystem* filesystem,
    const char* path, TF_Status* status);
typedef char* (*TP_Filesystem_TranslateName)(const TF_Filesystem* filesystem,
    const char* uri);
typedef int (*TP_Filesystem_GetChildren)(const TF_Filesystem* filesystem,
    const char* path, char*** entries, TF_Status* status);
typedef int (*TP_Filesystem_GetMatchingPaths)(const TF_Filesystem* filesystem,
    const char* glob, char*** entries, TF_Status* status);
typedef void (*TP_Filesystem_FlushCaches)(const TF_Filesystem* filesystem);
typedef void (*TP_Filesystem_GetConfiguration)(const TF_Filesystem* filesystem,
    TF_Filesystem_Option** options, int* num_options, TF_Status* status);
typedef void (*TP_Filesystem_SetConfiguration)(const TF_Filesystem* filesystem,
    const TF_Filesystem_Option* options, int num_options, TF_Status* status);
typedef void (*TP_Filesystem_GetConfigurationOption)(
    const TF_Filesystem* filesystem, const char* key,
    TF_Filesystem_Option** option, TF_Status* status);
typedef void (*TP_Filesystem_SetConfigurationOption)(
    const TF_Filesystem* filesystem, const TF_Filesystem_Option* option,
    TF_Status* status);
typedef void (*TP_Filesystem_GetConfigurationKeys)(
    const TF_Filesystem* filesystem, char** keys, int* num_keys,
    TF_Status* status);

// --------------------------------------------------------------------------
// Filesystem plugin memory callbacks

typedef void* (*TP_FilesystemPlugin_MemoryAllocate)(size_t size);
typedef void (*TP_FilesystemPlugin_MemoryFree)(void* ptr);

// --------------------------------------------------------------------------
// Filesystem vtable

// LINT.IfChange
typedef struct TF_FilesystemOps {
  /// Acquires all resources used by the filesystem.
  ///
  /// This operation must be provided. See "REQUIRED OPERATIONS" above.
  TP_Filesystem_Init init;

  /// Releases all resources used by the filesystem
  ///
  /// NOTE: TensorFlow does not unload DSOs. Thus, the only way a filesystem
  /// won't be registered anymore is if this function gets called by core
  /// TensorFlow and the `TF_Filesystem*` object is destroyed. However, due to
  /// registration being done in a static instance of `Env`, the destructor of
  /// `FileSystem` is never called (see
  /// https://github.com/tensorflow/tensorflow/issues/27535). In turn, this
  /// function will never be called. There are plans to refactor registration
  /// and fix this.
  ///
  /// TODO(b/139060984): After all filesystems are converted, revisit note.
  ///
  /// This operation must be provided. See "REQUIRED OPERATIONS" above.
  TP_Filesystem_Cleanup cleanup;

  /// Creates a new random access read-only file from given `path`.
  ///
  /// After this call `file` may be concurrently accessed by multiple threads.
  ///
  /// Plugins:
  ///   * Must set `status` to `TF_OK` if `file` was updated.
  ///   * Must set `status` to `TF_NOT_FOUND` if `path` doesn't point to an
  ///     existing file or one of the parent entries in `path` doesn't exist.
  ///   * Must set `status` to `TF_FAILED_PRECONDITION` if `path` points to a
  ///     directory or if it is invalid (e.g., malformed, or has a parent entry
  ///     which is a file).
  ///   * Might use any other error value for `status` to signal other errors.
  ///
  /// REQUIREMENTS: If plugins implement this, they must also provide a filled
  /// `TF_RandomAccessFileOps` table. See "REQUIRED OPERATIONS" above.
  TP_Filesystem_NewRandomAccessFile new_random_access_file;

  /// Creates an object to write to a file with the specified `path`.
  ///
  /// If the file already exists, it is deleted and recreated. The `file` object
  /// must only be accessed by one thread at a time.
  ///
  /// Plugins:
  ///   * Must set `status` to `TF_OK` if `file` was updated.
  ///   * Must set `status` to `TF_NOT_FOUND` if one of the parents entries in
  ///     `path` doesn't exist.
  ///   * Must set `status` to `TF_FAILED_PRECONDITION` if `path` points to a
  ///     directory or if it is invalid.
  ///   * Might use any other error value for `status` to signal other errors.
  ///
  /// REQUIREMENTS: If plugins implement this, they must also provide a filled
  /// `TF_WritableFileOps` table. See "REQUIRED OPERATIONS" above.
  TP_Filesystem_NewWritableFile new_writable_file;

  /// Creates an object to append to a file with the specified `path`.
  ///
  /// If the file doesn't exists, it is first created with empty contents.
  /// The `file` object must only be accessed by one thread at a time.
  ///
  /// Plugins:
  ///   * Must set `status` to `TF_OK` if `file` was updated.
  ///   * Must set `status` to `TF_NOT_FOUND` if one of the parents entries in
  ///     `path` doesn't exist.
  ///   * Must set `status` to `TF_FAILED_PRECONDITION` if `path` points to a
  ///     directory or if it is invalid.
  ///   * Might use any other error value for `status` to signal other errors.
  ///
  /// REQUIREMENTS: If plugins implement this, they must also provide a filled
  /// `TF_WritableFileOps` table. See "REQUIRED OPERATIONS" above.
  TP_Filesystem_NewAppendableFile new_appendable_file;

  /// Creates a read-only region of memory from contents of `path`.
  ///
  /// After this call `region` may be concurrently accessed by multiple threads.
  ///
  /// Plugins:
  ///   * Must set `status` to `TF_OK` if `region` was updated.
  ///   * Must set `status` to `TF_NOT_FOUND` if `path` doesn't point to an
  ///     existing file or one of the parent entries in `path` doesn't exist.
  ///   * Must set `status` to `TF_FAILED_PRECONDITION` if `path` points to a
  ///     directory or if it is invalid.
  ///   * Must set `status` to `TF_INVALID_ARGUMENT` if `path` points to an
  ///     empty file.
  ///   * Might use any other error value for `status` to signal other errors.
  ///
  /// REQUIREMENTS: If plugins implement this, they must also provide a filled
  /// `TF_ReadOnlyMemoryRegionOps` table. See "REQUIRED OPERATIONS" above.
  TP_Filesystem_NewReadOnlyMemoryRegionFromFile new_read_only_memory_region_from_file;

  /// Creates the directory specified by `path`, assuming parent exists.
  ///
  /// Plugins:
  ///   * Must set `status` to `TF_OK` if directory was created.
  ///   * Must set `status` to `TF_NOT_FOUND` if one of the parents entries in
  ///     `path` doesn't exist.
  ///   * Must set `status` to `TF_FAILED_PRECONDITION` if `path` is invalid.
  ///   * Must set `status` to `TF_ALREADY_EXISTS` if `path` already exists.
  ///   * Might use any other error value for `status` to signal other errors.
  TP_Filesystem_CreateDir create_dir;

  /// Creates the directory specified by `path` and all needed ancestors.
  ///
  /// Plugins:
  ///   * Must set `status` to `TF_OK` if directory was created.
  ///   * Must set `status` to `TF_FAILED_PRECONDITION` if `path` is invalid or
  ///     if it exists but is not a directory.
  ///   * Might use any other error value for `status` to signal other errors.
  ///
  /// NOTE: The requirements specify that `TF_ALREADY_EXISTS` is not returned if
  /// directory exists. Similarly, `TF_NOT_FOUND` is not be returned, as the
  /// missing directory entry and all its descendants will be created by the
  /// plugin.
  ///
  /// DEFAULT IMPLEMENTATION: Creates directories one by one. Needs
  /// `path_exists`, `is_directory`, and `create_dir`.
  TP_Filesystem_RecursivelyCreateDir recursively_create_dir;

  /// Deletes the file specified by `path`.
  ///
  /// Plugins:
  ///   * Must set `status` to `TF_OK` if file was deleted.
  ///   * Must set `status` to `TF_NOT_FOUND` if `path` doesn't exist.
  ///   * Must set `status` to `TF_FAILED_PRECONDITION` if `path` points to a
  ///     directory or if it is invalid.
  ///   * Might use any other error value for `status` to signal other errors.
  TP_Filesystem_DeleteFile delete_file;

  /// Deletes the empty directory specified by `path`.
  ///
  /// Plugins:
  ///   * Must set `status` to `TF_OK` if directory was deleted.
  ///   * Must set `status` to `TF_NOT_FOUND` if `path` doesn't exist.
  ///   * Must set `status` to `TF_FAILED_PRECONDITION` if `path` does not point
  ///     to a directory, if `path` is invalid, or if directory is not empty.
  ///   * Might use any other error value for `status` to signal other errors.
  TP_Filesystem_DeleteDir delete_dir;

  /// Deletes the directory specified by `path` and all its contents.
  ///
  /// This is accomplished by traversing directory tree rooted at `path` and
  /// deleting entries as they are encountered, from leaves to root. Each plugin
  /// is free to choose a different approach which obtains similar results.
  ///
  /// On successful deletion, `status` must be `TF_OK` and `*undeleted_files`
  /// and `*undeleted_dirs` must be 0. On unsuccessful deletion, `status` must
  /// be set to the reason why one entry couldn't be removed and the proper
  /// count must be updated. If the deletion is unsuccessful because the
  /// traversal couldn't start, `*undeleted_files` must be set to 0 and
  /// `*undeleted_dirs` must be set to 1.
  ///
  /// TODO(b/139060984): After all filesystems are converted, consider
  /// invariant about `*undeleted_files` and `*undeleted_dirs`.
  ///
  /// Plugins:
  ///   * Must set `status` to `TF_OK` if directory was deleted.
  ///   * Must set `status` to `TF_NOT_FOUND` if `path` doesn't exist.
  ///   * Must set `status` to `TF_FAILED_PRECONDITION` if `path` is invalid.
  ///   * Might use any other error value for `status` to signal other errors.
  ///
  /// DEFAULT IMPLEMENTATION: Does a BFS traversal of tree rooted at `path`,
  /// deleting entries as needed. Needs `path_exists`, `get_children`,
  /// `is_directory`, `delete_file`, and `delete_dir`.
  TP_Filesystem_DeleteRecursively delete_recursively;

  /// Renames the file given by `src` to that in `dst`.
  ///
  /// Replaces `dst` if it exists. In case of error, both `src` and `dst` keep
  /// the same state as before the call.
  ///
  /// Plugins:
  ///   * Must set `status` to `TF_OK` if rename was completed.
  ///   * Must set `status` to `TF_NOT_FOUND` if one of the parents entries in
  ///     either `src` or `dst` doesn't exist or if the specified `src` path
  ///     doesn't exist.
  ///   * Must set `status` to `TF_FAILED_PRECONDITION` if either `src` or
  ///     `dst` is a directory or if either of them is invalid.
  ///   * Might use any other error value for `status` to signal other errors.
  ///
  /// DEFAULT IMPLEMENTATION: Copies file and deletes original. Needs
  /// `copy_file`. and `delete_file`.
  TP_Filesystem_RenameFile rename_file;

  /// Copies the file given by `src` to that in `dst`.
  ///
  /// Similar to `rename_file`, but both `src` and `dst` exist after this call
  /// with the same contents. In case of error, both `src` and `dst` keep the
  /// same state as before the call.
  ///
  /// If `dst` is a directory, creates a file with the same name as the source
  /// inside the target directory.
  ///
  /// Plugins:
  ///   * Must set `status` to `TF_OK` if rename was completed.
  ///   * Must set `status` to `TF_NOT_FOUND` if one of the parents entries in
  ///     either `src` or `dst` doesn't exist or if the specified `src` path
  ///     doesn't exist.
  ///   * Must set `status` to `TF_FAILED_PRECONDITION` if either `src` or
  ///     `dst` is a directory or if either of them is invalid.
  ///   * Might use any other error value for `status` to signal other errors.
  ///
  /// DEFAULT IMPLEMENTATION: Reads from `src` and writes to `dst`. Needs
  /// `new_random_access_file` and `new_writable_file`.
  TP_Filesystem_CopyFile copy_file;

  /// Checks if `path` exists.
  ///
  /// Note that this doesn't differentiate between files and directories.
  ///
  /// Plugins:
  ///   * Must set `status` to `TF_OK` if `path` exists.
  ///   * Must set `status` to `TF_NOT_FOUND` if `path` doesn't point to a
  ///     filesystem entry.
  ///   * Must set `status` to `TF_FAILED_PRECONDITION` if `path` is invalid.
  ///   * Might use any other error value for `status` to signal other errors.
  TP_Filesystem_PathExists path_exists;

  /// Checks if all values in `paths` exist in the filesystem.
  ///
  /// Returns `true` if and only if calling `path_exists` on each entry in
  /// `paths` would set `status` to `TF_OK`.
  ///
  /// Caller guarantees that:
  ///   * `paths` has exactly `num_files` entries.
  ///   * `statuses` is either null or an array of `num_files` non-null elements
  ///     of type `TF_Status*`.
  ///
  /// If `statuses` is not null, plugins must fill each element with detailed
  /// status for each file, as if calling `path_exists` on each one. Core
  /// TensorFlow initializes the `statuses` array and plugins must use
  /// `TF_SetStatus` to set each element instead of directly assigning.
  ///
  /// DEFAULT IMPLEMENTATION: Checks existence of every file. Needs
  /// `path_exists`.
  TP_Filesystem_PathsExist paths_exist;

  /// Obtains statistics for the given `path`.
  ///
  /// Updates `stats` only if `status` is set to `TF_OK`.
  ///
  /// Plugins:
  ///   * Must set `status` to `TF_OK` if `path` exists.
  ///   * Must set `status` to `TF_NOT_FOUND` if `path` doesn't point to a
  ///     filesystem entry.
  ///   * Must set `status` to `TF_FAILED_PRECONDITION` if `path` is invalid.
  ///   * Might use any other error value for `status` to signal other errors.
  TP_Filesystem_Stat stat;

  /// Checks whether the given `path` is a directory or not.
  ///
  /// If `status` is not `TF_OK`, returns `false`, otherwise returns the same
  /// as the `is_directory` member of a `TF_FileStatistics` that would be used
  /// on the equivalent call of `stat`.
  ///
  /// Plugins:
  ///   * Must set `status` to `TF_OK` if `path` exists.
  ///   * Must set `status` to `TF_NOT_FOUND` if `path` doesn't point to a
  ///     filesystem entry.
  ///   * Must set `status` to `TF_FAILED_PRECONDITION` if `path` is invalid.
  ///   * Might use any other error value for `status` to signal other errors.
  ///
  /// DEFAULT IMPLEMENTATION: Gets statistics about `path`. Needs `stat`.
  TP_Filesystem_IsDirectory is_directory;

  /// Returns the size of the file given by `path`.
  ///
  /// If `status` is not `TF_OK`, return value is undefined. Otherwise, returns
  /// the same as `length` member of a `TF_FileStatistics` that would be used on
  /// the equivalent call of `stat`.
  ///
  /// Plugins:
  ///   * Must set `status` to `TF_OK` if `path` exists.
  ///   * Must set `status` to `TF_NOT_FOUND` if `path` doesn't point to a
  ///     filesystem entry.
  ///   * Must set `status` to `TF_FAILED_PRECONDITION` if `path` is invalid or
  ///     points to a directory.
  ///   * Might use any other error value for `status` to signal other errors.
  ///
  /// DEFAULT IMPLEMENTATION: Gets statistics about `path`. Needs `stat`.
  TP_Filesystem_GetFileSize get_file_size;

  /// Translates `uri` to a filename for the filesystem
  ///
  /// A filesystem is registered for a specific scheme and all of the methods
  /// should work with URIs. Hence, each filesystem needs to be able to
  /// translate from an URI to a path on the filesystem. For example, this
  /// function could translate `fs:///path/to/a/file` into `/path/to/a/file`, if
  /// implemented by a filesystem registered to handle the `fs://` scheme.
  ///
  /// A new `char*` buffer must be allocated by this method. Core TensorFlow
  /// manages the lifetime of the buffer after the call. Thus, all callers of
  /// this method must take ownership of the returned pointer.
  ///
  /// The implementation should clean up paths, including but not limited to,
  /// removing duplicate `/`s, and resolving `..` and `.`.
  ///
  /// Plugins must not return `nullptr`. Returning empty strings is allowed.
  ///
  /// The allocation and freeing of memory must happen via the functions sent to
  /// core TensorFlow upon registration (see the `TF_FilesystemPluginInfo`
  /// structure in Section 4).
  ///
  /// This function will be called by core TensorFlow to clean up all path
  /// arguments for all other methods in the filesystem API.
  ///
  /// DEFAULT IMPLEMENTATION: Uses `io::CleanPath` and `io::ParseURI`.
  TP_Filesystem_TranslateName translate_name;

  /// Finds all entries in the directory given by `path`.
  ///
  /// The returned entries are paths relative to `path`.
  ///
  /// Plugins must allocate `entries` to hold all names that need to be returned
  /// and return the size of `entries`. Caller takes ownership of `entries`
  /// after the call.
  ///
  /// In case of error, plugins must set `status` to a value different than
  /// `TF_OK`, free memory allocated for `entries` and return -1.
  ///
  /// The allocation and freeing of memory must happen via the functions sent to
  /// core TensorFlow upon registration (see the `TF_FilesystemPluginInfo`
  /// structure in Section 4).
  ///
  /// Plugins:
  ///   * Must set `status` to `TF_OK` if all children were returned.
  ///   * Must set `status` to `TF_NOT_FOUND` if `path` doesn't point to a
  ///     filesystem entry or if one of the parents entries in `path` doesn't
  ///     exist.
  ///   * Must set `status` to `TF_FAILED_PRECONDITION` if one of the parent
  ///     entries in `path` is not a directory, or if `path` is a file.
  ///   * Might use any other error value for `status` to signal other errors.
  TP_Filesystem_GetChildren get_children;

  /// Finds all entries matching the regular expression given by `glob`.
  ///
  /// Pattern must match the entire entry name, not just a substring.
  ///
  /// pattern: { term }
  /// term:
  ///   '*': matches any sequence of non-'/' characters
  ///   '?': matches a single non-'/' character
  ///   '[' [ '^' ] { match-list } ']':
  ///        matches any single character (not) on the list
  ///   c: matches character c (c != '*', '?', '\\', '[')
  ///   '\\' c: matches character c
  /// character-range:
  ///   c: matches character c (c != '\\', '-', ']')
  ///   '\\' c: matches character c
  ///   lo '-' hi: matches character c for lo <= c <= hi
  ///
  /// Implementations must allocate `entries` to hold all names that need to be
  /// returned and return the size of `entries`. Caller takes ownership of
  /// `entries` after the call.
  ///
  /// In case of error, the implementations must set `status` to a value
  /// different than `TF_OK`, free any memory that might have been allocated for
  /// `entries` and return -1.
  ///
  /// The allocation and freeing of memory must happen via the functions sent to
  /// core TensorFlow upon registration (see the `TF_FilesystemPluginInfo`
  /// structure in Section 4).
  ///
  /// Plugins:
  ///   * Must set `status` to `TF_OK` if all matches were returned.
  ///   * Might use any other error value for `status` to signal other errors.
  ///
  /// DEFAULT IMPLEMENTATION: Scans the directory tree (in parallel if possible)
  /// and fills `*entries`. Needs `get_children` and `is_directory`.
  TP_Filesystem_GetMatchingPaths get_matching_paths;

  /// Flushes any filesystem cache currently in memory
  ///
  /// DEFAULT IMPLEMENTATION: No op.
  TP_Filesystem_FlushCaches flush_caches;

  /// Returns pointer to an array of available configuration options and their
  /// current/default values in `options` and number of options in array in
  /// `num_options`. Ownership of the array is transferred to caller and the
  /// caller is responsible of freeing the buffers using respective file systems
  /// allocation API.
  ///
  /// Plugins:
  ///   * Must set `status` to `TF_OK` if `options` and `num_options` set.
  ///     If there is no configurable option, `num_options` should be 0.
  ///   * Might use any other error value for `status` to signal other errors.
  ///
  /// DEFAULT IMPLEMENTATION: return 0 options and `TF_OK`.
  TP_Filesystem_GetConfiguration get_filesystem_configuration;

  /// Updates filesystem configuration with options passed in `options`. It can
  /// contain full set of options supported by the filesystem or just a subset
  /// of them. Ownership of options and buffers therein belongs to the caller
  /// and any buffers need to be allocated through filesystem allocation API.
  /// Filesystems may choose to ignore configuration errors but should at least
  /// display a warning or error message to warn the users.
  ///
  /// Plugins:
  ///   * Must set `status` to `TF_OK` if options are updated.
  ///   * Might use any other error value for `status` to signal other errors.
  ///
  /// DEFAULT IMPLEMENTATION: return `TF_NOT_FOUND`.
  TP_Filesystem_SetConfiguration set_filesystem_configuration;

  /// Returns the value of the filesystem option given in `key` in `option`.
  /// Valid values of the `key` are returned by
  /// `get_file_system_configuration_keys` call. Ownership of the
  /// `option` is transferred to caller. Buffers therein should be allocated and
  /// freed by the relevant filesystems allocation API.
  ///
  /// Plugins:
  ///   * Must set `status` to `TF_OK` if `option` is set
  ///   * Must set `status` to `TF_NOT_FOUND` if the key is invalid
  ///   * Might use any other error value for `status` to signal other errors.
  ///
  /// DEFAULT IMPLEMENTATION: return `TF_NOT_FOUND`.
  TP_Filesystem_GetConfigurationOption get_filesystem_configuration_option;

  /// Sets the value of the filesystem option given in `key` to value in
  /// `option`. Valid values of the `key` are returned by
  /// `get_file_system_configuration_keys` call. Ownership of the `option` and
  /// the `key` belogs to the caller. Buffers therein should be allocated and
  /// freed by the filesystems allocation API.
  ///
  /// Plugins:
  ///   * Must set `status` to `TF_OK` if `option` is set/updated
  ///   * Must set `status` to `TF_NOT_FOUND` if the key is invalid
  ///   * Might use any other error value for `status` to signal other errors.
  ///
  /// DEFAULT IMPLEMENTATION: return `TF_NOT_FOUND`.
  TP_Filesystem_SetConfigurationOption set_filesystem_configuration_option;

  /// Returns a list of valid configuration keys in `keys` array and number of
  /// keys in `num_keys`. Ownership of the buffers in `keys` are transferred to
  /// caller and needs to be freed using relevant filesystem allocation API.
  ///
  /// Plugins:
  ///   * Must set `status` to `TF_OK` on success. If there are no configurable
  ///     keys, `num_keys` should be set to 0
  ///   * Might use any other error value for `status` to signal other errors.
  ///
  /// DEFAULT IMPLEMENTATION: return `TF_OK` and `num_keys`=0.
  TP_Filesystem_GetConfigurationKeys get_filesystem_configuration_keys;
} TF_FilesystemOps;
// LINT.ThenChange(:filesystem_ops_version)

// Opaque handle alloc/dealloc


static inline TF_Filesystem* TF_FilesystemNew(void) {
  TF_Filesystem* ptr = (TF_Filesystem*)malloc(sizeof(TF_Filesystem));
  if (!ptr) return nullptr;
  ptr->plugin_filesystem = nullptr;
  return ptr;
}

static inline void TF_FilesystemDelete(TF_Filesystem* ptr) {
  if (!ptr) return;
  free(ptr);
}

static inline void TF_Filesystem_SetPluginFilesystem(TF_Filesystem* builder, void* plugin_filesystem) {
  builder->plugin_filesystem = plugin_filesystem;
}



// Vtable alloc/dealloc


static inline TF_FilesystemOps* TF_FilesystemOpsNew(void) {
  TF_FilesystemOps* ptr = (TF_FilesystemOps*)malloc(sizeof(TF_FilesystemOps));
  if (!ptr) return nullptr;
  ptr->init = nullptr;
  ptr->cleanup = nullptr;
  ptr->new_random_access_file = nullptr;
  ptr->new_writable_file = nullptr;
  ptr->new_appendable_file = nullptr;
  ptr->new_read_only_memory_region_from_file = nullptr;
  ptr->create_dir = nullptr;
  ptr->recursively_create_dir = nullptr;
  ptr->delete_file = nullptr;
  ptr->delete_dir = nullptr;
  ptr->delete_recursively = nullptr;
  ptr->rename_file = nullptr;
  ptr->copy_file = nullptr;
  ptr->path_exists = nullptr;
  ptr->paths_exist = nullptr;
  ptr->stat = nullptr;
  ptr->is_directory = nullptr;
  ptr->get_file_size = nullptr;
  ptr->translate_name = nullptr;
  ptr->get_children = nullptr;
  ptr->get_matching_paths = nullptr;
  ptr->flush_caches = nullptr;
  ptr->get_filesystem_configuration = nullptr;
  ptr->set_filesystem_configuration = nullptr;
  ptr->get_filesystem_configuration_option = nullptr;
  ptr->set_filesystem_configuration_option = nullptr;
  ptr->get_filesystem_configuration_keys = nullptr;
  return ptr;
}

static inline void TF_FilesystemOpsDelete(TF_FilesystemOps* ptr) {
  if (!ptr) return;
  free(ptr);
}

static inline void TF_FilesystemOps_SetInit(TF_FilesystemOps* builder, TP_Filesystem_Init init) {
  builder->init = init;
}

static inline void TF_FilesystemOps_SetCleanup(TF_FilesystemOps* builder, TP_Filesystem_Cleanup cleanup) {
  builder->cleanup = cleanup;
}

static inline void TF_FilesystemOps_SetNewRandomAccessFile(TF_FilesystemOps* builder, TP_Filesystem_NewRandomAccessFile new_random_access_file) {
  builder->new_random_access_file = new_random_access_file;
}

static inline void TF_FilesystemOps_SetNewWritableFile(TF_FilesystemOps* builder, TP_Filesystem_NewWritableFile new_writable_file) {
  builder->new_writable_file = new_writable_file;
}

static inline void TF_FilesystemOps_SetNewAppendableFile(TF_FilesystemOps* builder, TP_Filesystem_NewAppendableFile new_appendable_file) {
  builder->new_appendable_file = new_appendable_file;
}

static inline void TF_FilesystemOps_SetNewReadOnlyMemoryRegionFromFile(TF_FilesystemOps* builder, TP_Filesystem_NewReadOnlyMemoryRegionFromFile new_read_only_memory_region_from_file) {
  builder->new_read_only_memory_region_from_file = new_read_only_memory_region_from_file;
}

static inline void TF_FilesystemOps_SetCreateDir(TF_FilesystemOps* builder, TP_Filesystem_CreateDir create_dir) {
  builder->create_dir = create_dir;
}

static inline void TF_FilesystemOps_SetRecursivelyCreateDir(TF_FilesystemOps* builder, TP_Filesystem_RecursivelyCreateDir recursively_create_dir) {
  builder->recursively_create_dir = recursively_create_dir;
}

static inline void TF_FilesystemOps_SetDeleteFile(TF_FilesystemOps* builder, TP_Filesystem_DeleteFile delete_file) {
  builder->delete_file = delete_file;
}

static inline void TF_FilesystemOps_SetDeleteDir(TF_FilesystemOps* builder, TP_Filesystem_DeleteDir delete_dir) {
  builder->delete_dir = delete_dir;
}

static inline void TF_FilesystemOps_SetDeleteRecursively(TF_FilesystemOps* builder, TP_Filesystem_DeleteRecursively delete_recursively) {
  builder->delete_recursively = delete_recursively;
}

static inline void TF_FilesystemOps_SetRenameFile(TF_FilesystemOps* builder, TP_Filesystem_RenameFile rename_file) {
  builder->rename_file = rename_file;
}

static inline void TF_FilesystemOps_SetCopyFile(TF_FilesystemOps* builder, TP_Filesystem_CopyFile copy_file) {
  builder->copy_file = copy_file;
}

static inline void TF_FilesystemOps_SetPathExists(TF_FilesystemOps* builder, TP_Filesystem_PathExists path_exists) {
  builder->path_exists = path_exists;
}

static inline void TF_FilesystemOps_SetPathsExist(TF_FilesystemOps* builder, TP_Filesystem_PathsExist paths_exist) {
  builder->paths_exist = paths_exist;
}

static inline void TF_FilesystemOps_SetStat(TF_FilesystemOps* builder, TP_Filesystem_Stat stat) {
  builder->stat = stat;
}

static inline void TF_FilesystemOps_SetIsDirectory(TF_FilesystemOps* builder, TP_Filesystem_IsDirectory is_directory) {
  builder->is_directory = is_directory;
}

static inline void TF_FilesystemOps_SetGetFileSize(TF_FilesystemOps* builder, TP_Filesystem_GetFileSize get_file_size) {
  builder->get_file_size = get_file_size;
}

static inline void TF_FilesystemOps_SetTranslateName(TF_FilesystemOps* builder, TP_Filesystem_TranslateName translate_name) {
  builder->translate_name = translate_name;
}

static inline void TF_FilesystemOps_SetGetChildren(TF_FilesystemOps* builder, TP_Filesystem_GetChildren get_children) {
  builder->get_children = get_children;
}

static inline void TF_FilesystemOps_SetGetMatchingPaths(TF_FilesystemOps* builder, TP_Filesystem_GetMatchingPaths get_matching_paths) {
  builder->get_matching_paths = get_matching_paths;
}

static inline void TF_FilesystemOps_SetFlushCaches(TF_FilesystemOps* builder, TP_Filesystem_FlushCaches flush_caches) {
  builder->flush_caches = flush_caches;
}

static inline void TF_FilesystemOps_SetGetFilesystemConfiguration(TF_FilesystemOps* builder, TP_Filesystem_GetConfiguration get_filesystem_configuration) {
  builder->get_filesystem_configuration = get_filesystem_configuration;
}

static inline void TF_FilesystemOps_SetSetFilesystemConfiguration(TF_FilesystemOps* builder, TP_Filesystem_SetConfiguration set_filesystem_configuration) {
  builder->set_filesystem_configuration = set_filesystem_configuration;
}

static inline void TF_FilesystemOps_SetGetFilesystemConfigurationOption(TF_FilesystemOps* builder, TP_Filesystem_GetConfigurationOption get_filesystem_configuration_option) {
  builder->get_filesystem_configuration_option = get_filesystem_configuration_option;
}

static inline void TF_FilesystemOps_SetSetFilesystemConfigurationOption(TF_FilesystemOps* builder, TP_Filesystem_SetConfigurationOption set_filesystem_configuration_option) {
  builder->set_filesystem_configuration_option = set_filesystem_configuration_option;
}

static inline void TF_FilesystemOps_SetGetFilesystemConfigurationKeys(TF_FilesystemOps* builder, TP_Filesystem_GetConfigurationKeys get_filesystem_configuration_keys) {
  builder->get_filesystem_configuration_keys = get_filesystem_configuration_keys;
}// NOTE: TF_FilesystemPluginOps / TF_FilesystemPluginInfo struct definitions,
// their New/Delete/Set* inlines, and TF_SetFilesystemVersionMetadata live in
// registration.h to avoid a circular include dependency (registration.h
// includes this header before those structs are defined).

#ifdef __cplusplus
}  // end extern "C"
#endif  // __cplusplus

#endif  // CONGELADO_C_FILESYSTEM_FILESYSTEM_OPS_H_
