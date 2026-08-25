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

#ifndef CONGELADO_C_FILESYSTEM_WRITABLE_FILE_H_
#define CONGELADO_C_FILESYSTEM_WRITABLE_FILE_H_

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#include "c/intern/tf_status.h"

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

/// Opaque data structure to hold plugin-specific data for a writable file.
///
/// The wrapper data structure is owned by core TensorFlow. The data pointed to
/// by the `void*` member is always owned by the plugin.
///
/// Plugins will never receive a `TF_WritableFile*` that is `nullptr`. Core
/// TensorFlow will never touch the `void*` wrapped by this structure, except
/// to initialize it as `nullptr`.

typedef struct TF_WritableFile {
  void* plugin_file;
} TF_WritableFile;

// --------------------------------------------------------------------------
// WritableFile callback types

/// Releases resources associated with `*file`.
///
/// Requires that `*file` is not used in any concurrent or subsequent
/// operations.
typedef void (*TP_WritableFile_Cleanup)(TF_WritableFile* file);

/// Appends `buffer` of size `n` to `*file`.
///
/// Core TensorFlow owns `buffer` and guarantees at least `n` bytes of storage
/// that can be used to write data.
///
/// Note: the `buffer` argument is NOT a null terminated string!
///
/// Plugins:
///   * Must set `status` to `TF_OK` if exactly `n` bytes have been written.
///   * Must set `status` to `TF_RESOURCE_EXHAUSTED` if fewer than `n` bytes
///     have been written, potentially due to quota/disk space.
///   * Might use any other error value for `status` to signal other errors.
typedef void (*TP_WritableFile_Append)(const TF_WritableFile* file,
                                      const char* buffer, size_t n,
                                      TF_Status* status);

/// Returns the current write position in `*file`.
///
/// Plugins should ensure that the implementation is idempotent, 2 identical
/// calls result in the same answer.
///
/// Plugins:
///   * Must set `status` to `TF_OK` and return current position if no error.
///   * Must set `status` to any other value and return -1 in case of error.
typedef int64_t (*TP_WritableFile_Tell)(const TF_WritableFile* file,
                                       TF_Status* status);

/// Flushes `*file` and syncs contents to filesystem.
///
/// This call might not block, and when it returns the contents might not have
/// been fully persisted.
///
/// DEFAULT IMPLEMENTATION: No op.
typedef void (*TP_WritableFile_Flush)(const TF_WritableFile* file,
                                     TF_Status* status);

/// Syncs contents of `*file` with the filesystem.
///
/// This call should block until filesystem confirms that all buffers have
/// been flushed and persisted.
///
/// DEFAULT IMPLEMENTATION: No op.
typedef void (*TP_WritableFile_Sync)(const TF_WritableFile* file,
                                    TF_Status* status);

/// Closes `*file`.
///
/// Flushes all buffers and deallocates all resources.
///
/// Calling `close` must not result in calling `cleanup`.
///
/// Core TensorFlow will never call `close` twice.
typedef void (*TP_WritableFile_Close)(const TF_WritableFile* file,
                                     TF_Status* status);

// --------------------------------------------------------------------------
// WritableFile vtable

// LINT.IfChange
typedef struct TF_WritableFileOps {
  /// Releases resources associated with `*file`.
  ///
  /// Requires that `*file` is not used in any concurrent or subsequent
  /// operations.
  ///
  /// This operation must be provided. See "REQUIRED OPERATIONS" above.
  TP_WritableFile_Cleanup cleanup;

  /// Appends `buffer` of size `n` to `*file`.
  ///
  /// Core TensorFlow owns `buffer` and guarantees at least `n` bytes of storage
  /// that can be used to write data.
  ///
  /// Note: the `buffer` argument is NOT a null terminated string!
  ///
  /// Plugins:
  ///   * Must set `status` to `TF_OK` if exactly `n` bytes have been written.
  ///   * Must set `status` to `TF_RESOURCE_EXHAUSTED` if fewer than `n` bytes
  ///     have been written, potentially due to quota/disk space.
  ///   * Might use any other error value for `status` to signal other errors.
  TP_WritableFile_Append append;

  /// Returns the current write position in `*file`.
  ///
  /// Plugins should ensure that the implementation is idempotent, 2 identical
  /// calls result in the same answer.
  ///
  /// Plugins:
  ///   * Must set `status` to `TF_OK` and return current position if no error.
  ///   * Must set `status` to any other value and return -1 in case of error.
  TP_WritableFile_Tell tell;

  /// Flushes `*file` and syncs contents to filesystem.
  ///
  /// This call might not block, and when it returns the contents might not have
  /// been fully persisted.
  ///
  /// DEFAULT IMPLEMENTATION: No op.
  TP_WritableFile_Flush flush;

  /// Syncs contents of `*file` with the filesystem.
  ///
  /// This call should block until filesystem confirms that all buffers have
  /// been flushed and persisted.
  ///
  /// DEFAULT IMPLEMENTATION: No op.
  TP_WritableFile_Sync sync;

  /// Closes `*file`.
  ///
  /// Flushes all buffers and deallocates all resources.
  ///
  /// Calling `close` must not result in calling `cleanup`.
  ///
  /// Core TensorFlow will never call `close` twice.
  TP_WritableFile_Close close;
} TF_WritableFileOps;
// LINT.ThenChange(:writable_file_ops_version)

// Opaque handle alloc/dealloc


static inline TF_WritableFile* TF_WritableFileNew(void) {
  TF_WritableFile* ptr = (TF_WritableFile*)malloc(sizeof(TF_WritableFile));
  if (!ptr) return nullptr;
  ptr->plugin_file = nullptr;
  return ptr;
}

static inline void TF_WritableFileDelete(TF_WritableFile* ptr) {
  if (!ptr) return;
  free(ptr);
}

static inline void TF_WritableFile_SetPluginFile(TF_WritableFile* builder, void* plugin_file) {
  builder->plugin_file = plugin_file;
}



// Vtable alloc/dealloc (value type)


static inline TF_WritableFileOps* TF_WritableFileOpsNew(void) {
  TF_WritableFileOps* ptr = (TF_WritableFileOps*)malloc(sizeof(TF_WritableFileOps));
  if (!ptr) return nullptr;
  ptr->cleanup = nullptr;
  ptr->append = nullptr;
  ptr->tell = nullptr;
  ptr->flush = nullptr;
  ptr->sync = nullptr;
  ptr->close = nullptr;
  return ptr;
}

static inline void TF_WritableFileOpsDelete(TF_WritableFileOps* ptr) {
  if (!ptr) return;
  free(ptr);
}

static inline void TF_WritableFileOps_SetCleanup(TF_WritableFileOps* builder, TP_WritableFile_Cleanup cleanup) {
  builder->cleanup = cleanup;
}

static inline void TF_WritableFileOps_SetAppend(TF_WritableFileOps* builder, TP_WritableFile_Append append) {
  builder->append = append;
}

static inline void TF_WritableFileOps_SetTell(TF_WritableFileOps* builder, TP_WritableFile_Tell tell) {
  builder->tell = tell;
}

static inline void TF_WritableFileOps_SetFlush(TF_WritableFileOps* builder, TP_WritableFile_Flush flush) {
  builder->flush = flush;
}

static inline void TF_WritableFileOps_SetSync(TF_WritableFileOps* builder, TP_WritableFile_Sync sync) {
  builder->sync = sync;
}

static inline void TF_WritableFileOps_SetClose(TF_WritableFileOps* builder, TP_WritableFile_Close close) {
  builder->close = close;
}

#ifdef __cplusplus
}  // end extern "C"
#endif  // __cplusplus

#endif  // CONGELADO_C_FILESYSTEM_WRITABLE_FILE_H_