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

#include "c/macros.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    // TF_FileStatistics — plugin vtable for one file/directory's stat() result.
    typedef struct TF_FileStatisticsOps TF_FileStatisticsOps;

    typedef struct TF_FileStatistics
    {
        void* plugin_data;
        const TF_FileStatisticsOps* ops;
    } TF_FileStatistics;

    // Plugin-facing vtable registered via create_file_statistics.
    typedef struct TF_FileStatisticsOps
    {
        size_t struct_size;

        // Return the backend's name (e.g. "file_statistics") into *out.
        void (*get_name)(TF_FileStatistics* stats, TF_String* out);

        // Non-zero if the entry is a directory.
        int (*is_directory)(const TF_FileStatistics* stats);
        void (*set_is_directory)(TF_FileStatistics* stats, int is_directory);

        // File length in bytes.
        int64_t (*length)(const TF_FileStatistics* stats);
        void (*set_length)(TF_FileStatistics* stats, int64_t length);

        // Last modification time, in nanoseconds since the epoch.
        int64_t (*mtime_nsec)(const TF_FileStatistics* stats);
        void (*set_mtime_nsec)(TF_FileStatistics* stats, int64_t mtime_nsec);

        void (*destroy)(TF_FileStatistics* stats);

    } TF_FileStatisticsOps;

#define TF_FILE_STATISTICS_STRUCT_SIZE TF_OFFSET_OF_END(TF_FileStatisticsOps, destroy)

    TF_CAPI_EXPORT void
    create_file_statistics(TF_FileStatisticsOps** ops, void** plugin_context, TF_Status* status);
    TF_CAPI_EXPORT void destroy_file_statistics(void* plugin_context);

    // Real implementation, not declared-only — calls create_file_statistics and fills in stats->ops.
    static inline void init_file_statistics(TF_FileStatistics* stats, TF_Status* status)
    {
        TF_FileStatisticsOps* ops = NULL;
        create_file_statistics(&ops, &stats->plugin_data, status);
        stats->ops = ops;
    }

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // TENSORFLOW_C_TF_FILE_STATISTICS_H_
