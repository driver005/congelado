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

#ifndef TENSORFLOW_C_TF_FILE_STATISTICS_H_
#define TENSORFLOW_C_TF_FILE_STATISTICS_H_

#include <stddef.h>
#include <stdint.h>

#include "c/abi/macros.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct TF_FileStatistics {
  size_t struct_size;
  int is_directory;
  int64_t length;
  int64_t mtime_nsec;
} TF_FileStatistics;

#define TF_FILE_STATISTICS_STRUCT_SIZE TF_OFFSET_OF_END(TF_FileStatistics, mtime_nsec)

static inline void TF_FileStatisticsInit(TF_FileStatistics* stats) {
  stats->struct_size = TF_FILE_STATISTICS_STRUCT_SIZE;
  stats->is_directory = 0;
  stats->length = 0;
  stats->mtime_nsec = 0;
}

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif  // TENSORFLOW_C_TF_FILE_STATISTICS_H_