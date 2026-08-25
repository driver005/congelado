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

#ifndef CONGELADO_C_FILESYSTEM_RANDOM_ACCESS_FILE_H_
#define CONGELADO_C_FILESYSTEM_RANDOM_ACCESS_FILE_H_

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#include "c/intern/tf_status.h"

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

/// Opaque data structure to hold plugin-specific data for a random access file.
///
/// The wrapper data structure is owned by core TensorFlow. The data pointed to
/// by the `void*` member is always owned by the plugin.
///
/// Plugins will never receive a `TF_RandomAccessFile*` that is `nullptr`. Core
/// TensorFlow will never touch the `void*` wrapped by this structure, except
/// to initialize it as `nullptr`.

typedef struct TF_RandomAccessFile {
  void* plugin_file;
} TF_RandomAccessFile;

// --------------------------------------------------------------------------
// RandomAccessFile callback types

/// Releases resources associated with `*file`.
///
/// Requires that `*file` is not used in any concurrent or subsequent
/// operations.
typedef void (*TP_RandomAccessFile_Cleanup)(TF_RandomAccessFile* file);

/// Reads up to `n` bytes from `*file` starting at `offset`.
///
/// The output is in `buffer`, core TensorFlow owns the buffer and guarantees
/// that at least `n` bytes are available.
///
/// Returns number of bytes read or -1 in case of error. Because of this
/// constraint and the fact that `ssize_t` is not defined in `stdint.h`/C++
/// standard, the return type is `int64_t`.
///
/// This is thread safe.
///
/// Note: the `buffer` argument is NOT a null terminated string!
///
/// Plugins:
///   * Must set `status` to `TF_OK` if exactly `n` bytes have been read.
///   * Must set `status` to `TF_OUT_OF_RANGE` if fewer than `n` bytes have
///     been read due to EOF.
///   * Must return -1 for any other error and must set `status` to any
///     other value to provide more information about the error.
typedef int64_t (*TP_RandomAccessFile_Read)(const TF_RandomAccessFile* file,
                                           uint64_t offset, size_t n,
                                           char* buffer, TF_Status* status);

// --------------------------------------------------------------------------
// RandomAccessFile vtable

// LINT.IfChange
typedef struct TF_RandomAccessFileOps {
  /// Releases resources associated with `*file`.
  ///
  /// Requires that `*file` is not used in any concurrent or subsequent
  /// operations.
  ///
  /// This operation must be provided. See "REQUIRED OPERATIONS" above.
  TP_RandomAccessFile_Cleanup cleanup;

  /// Reads up to `n` bytes from `*file` starting at `offset`.
  ///
  /// The output is in `buffer`, core TensorFlow owns the buffer and guarantees
  /// that at least `n` bytes are available.
  ///
  /// Returns number of bytes read or -1 in case of error. Because of this
  /// constraint and the fact that `ssize_t` is not defined in `stdint.h`/C++
  /// standard, the return type is `int64_t`.
  ///
  /// This is thread safe.
  ///
  /// Note: the `buffer` argument is NOT a null terminated string!
  ///
  /// Plugins:
  ///   * Must set `status` to `TF_OK` if exactly `n` bytes have been read.
  ///   * Must set `status` to `TF_OUT_OF_RANGE` if fewer than `n` bytes have
  ///     been read due to EOF.
  ///   * Must return -1 for any other error and must set `status` to any
  ///     other value to provide more information about the error.
  TP_RandomAccessFile_Read read;
} TF_RandomAccessFileOps;
// LINT.ThenChange(:random_access_file_ops_version)

// Opaque handle alloc/dealloc


static inline TF_RandomAccessFile* TF_RandomAccessFileNew(void) {
  TF_RandomAccessFile* ptr = (TF_RandomAccessFile*)malloc(sizeof(TF_RandomAccessFile));
  if (!ptr) return nullptr;
  ptr->plugin_file = nullptr;
  return ptr;
}

static inline void TF_RandomAccessFileDelete(TF_RandomAccessFile* ptr) {
  if (!ptr) return;
  free(ptr);
}

static inline void TF_RandomAccessFile_SetPluginFile(TF_RandomAccessFile* builder, void* plugin_file) {
  builder->plugin_file = plugin_file;
}



// Vtable alloc/dealloc (value type, no alloc needed but for consistency)


static inline TF_RandomAccessFileOps* TF_RandomAccessFileOpsNew(void) {
  TF_RandomAccessFileOps* ptr = (TF_RandomAccessFileOps*)malloc(sizeof(TF_RandomAccessFileOps));
  if (!ptr) return nullptr;
  ptr->cleanup = nullptr;
  ptr->read = nullptr;
  return ptr;
}

static inline void TF_RandomAccessFileOpsDelete(TF_RandomAccessFileOps* ptr) {
  if (!ptr) return;
  free(ptr);
}

static inline void TF_RandomAccessFileOps_SetCleanup(TF_RandomAccessFileOps* builder, TP_RandomAccessFile_Cleanup cleanup) {
  builder->cleanup = cleanup;
}

static inline void TF_RandomAccessFileOps_SetRead(TF_RandomAccessFileOps* builder, TP_RandomAccessFile_Read read) {
  builder->read = read;
}

#ifdef __cplusplus
}  // end extern "C"
#endif  // __cplusplus

#endif  // CONGELADO_C_FILESYSTEM_RANDOM_ACCESS_FILE_H_