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

#include "include/c/macros.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    // TF_FileStatistics — plugin vtable for one file/directory's stat() result.

    typedef struct TF_FileStatistics
    {
        void* plugin_data;
    } TF_FileStatistics;

    // Plugin-facing vtable registered via create_file_statistics.
    typedef struct TF_FileStatisticsOps
    {
        size_t struct_size;

        // Return the backend's name (e.g. "file_statistics") into *out.
        void (*get_name)(TF_FileStatistics* stats, TF_String* out_name);

        // Non-zero if the entry is a directory.
        void (*is_directory)(const TF_FileStatistics* stats, int* out_is_directory);
        void (*set_is_directory)(TF_FileStatistics* stats, int is_directory);

        // File length in bytes.
        void (*length)(const TF_FileStatistics* stats, int64_t* out_length);
        void (*set_length)(TF_FileStatistics* stats, int64_t length);

        // Last modification time, in nanoseconds since the epoch.
        void (*mtime_nsec)(const TF_FileStatistics* stats, int64_t* out_mtime_nsec);
        void (*set_mtime_nsec)(TF_FileStatistics* stats, int64_t mtime_nsec);

        void (*destroy)(TF_FileStatistics* stats);

    } TF_FileStatisticsOps;

#define TF_FILE_STATISTICS_STRUCT_SIZE TF_OFFSET_OF_END(TF_FileStatisticsOps, destroy)

    TF_CAPI_EXPORT void
    create_file_statistics(TF_FileStatisticsOps** ops, void** plugin_context, TF_Status* out_status);
    TF_CAPI_EXPORT void destroy_file_statistics(void* plugin_context);

    // Real implementation, not declared-only — calls create_file_statistics
    static inline void init_file_statistics(TF_FileStatistics* stats, TF_Status* out_status)
    {
        TF_FileStatisticsOps* ops = NULL;
        create_file_statistics(&ops, &stats->plugin_data, out_status);
    }

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // TENSORFLOW_C_TF_FILE_STATISTICS_H_
