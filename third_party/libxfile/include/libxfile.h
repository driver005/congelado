/*
 * Copyright (c) 2026, xio-sig (Accelerated IO Special Interest Group)
 * Open-source libxFile wrapper and platform abstraction.
 */

#ifndef LIBXFILE_H_
#define LIBXFILE_H_

#include "cufile.h"

#ifdef __cplusplus
extern "C" {
#endif

// libxFile vendor agnosticism helper to inspect backend device (NVIDIA, AMD, Intel Level Zero, SYCL)
typedef enum XFileBackendType {
    XFILE_BACKEND_UNKNOWN = 0,
    XFILE_BACKEND_LEVEL_ZERO = 1,
    XFILE_BACKEND_CUDA = 2,
    XFILE_BACKEND_ROCM = 3,
    XFILE_BACKEND_GENERIC_P2PDMA = 4
} XFileBackendType_t;

XFileBackendType_t xFileGetBackendType(void);

#ifdef __cplusplus
}
#endif

#endif // LIBXFILE_H_
