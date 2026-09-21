/*
 * Copyright (c) 2026, xio-sig (Accelerated IO Special Interest Group)
 * Open-source, vendor-agnostic cuFile API definition for direct GPU-to-storage DMA.
 * Licensed under the Apache License, Version 2.0.
 */

#ifndef CUFILE_H_
#define CUFILE_H_

#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct CUfileDriverStatus {
    int is_supported;
    int is_initialized;
    size_t max_direct_io_size;
    size_t nvme_poll_thresh_size;
} CUfileDriverStatus_t;

typedef enum CUfileOpcode {
    CU_FILE_READ = 0,
    CU_FILE_WRITE = 1
} CUfileOpcode_t;

typedef enum CUfileStatusType {
    CU_FILE_SUCCESS = 0,
    CU_FILE_DRIVER_NOT_INITIALIZED = 1,
    CU_FILE_IO_NOT_SUPPORTED = 2,
    CU_FILE_MEMORY_NOT_REGISTERED = 3,
    CU_FILE_INVALID_VALUE = 4,
    CU_FILE_IO_FAILED = 5
} CUfileStatusType_t;

typedef struct CUfileError {
    CUfileStatusType_t err;
    int error_code;
} CUfileError_t;

typedef struct CUfileDescr {
    int type; // 0 = OS file descriptor
    union {
        int fd;
    } handle;
} CUfileDescr_t;

typedef void* CUfileHandle_t;
typedef void* CUfileBatchHandle_t;

typedef struct CUfileIOParams {
    CUfileOpcode_t mode;
    void* devPtr_base;
    size_t devPtr_offset;
    off_t file_offset;
    size_t size;
} CUfileIOParams_t;

typedef struct CUfileBatchIORequest {
    CUfileHandle_t fh;
    CUfileIOParams_t io_params;
    ssize_t bytes_transferred;
    CUfileError_t status;
} CUfileBatchIORequest_t;

// Driver session lifecycle
CUfileError_t cuFileDriverOpen(void);
CUfileError_t cuFileDriverClose(void);
CUfileError_t cuFileDriverGetProperties(CUfileDriverStatus_t* props);

// Buffer registration for DMA (GPU VRAM / USM pointer)
CUfileError_t cuFileBufRegister(const void* devPtr_base, size_t length, int flags);
CUfileError_t cuFileBufDeregister(const void* devPtr_base);

// File handle registration
CUfileError_t cuFileHandleRegister(CUfileHandle_t* fh, CUfileDescr_t* descr);
void cuFileHandleDeregister(CUfileHandle_t fh);

// Synchronous Direct GPU-to-Disk I/O (DMA, zero CPU bounce buffer)
ssize_t cuFileRead(CUfileHandle_t fh, void* devPtr_base, size_t size, off_t file_offset, off_t devPtr_offset);
ssize_t cuFileWrite(CUfileHandle_t fh, const void* devPtr_base, size_t size, off_t file_offset, off_t devPtr_offset);

// Asynchronous Batched Direct GPU-to-Disk I/O
CUfileError_t cuFileBatchIOSubmit(CUfileBatchHandle_t batch_id, size_t num_requests, CUfileBatchIORequest_t* reqs, unsigned int flags);
CUfileError_t cuFileBatchIOGetStatus(CUfileBatchHandle_t batch_id, size_t min_nr, size_t* nr, CUfileBatchIORequest_t* reqs, const struct timespec* timeout);
CUfileError_t cuFileBatchIOCancel(CUfileBatchHandle_t batch_id);
void cuFileBatchIODestroy(CUfileBatchHandle_t batch_id);

#ifdef __cplusplus
}
#endif

#endif // CUFILE_H_
