#include "include/c/extern/filesystem/filesystem.h"

#include "include/c/extern/filesystem/random_access_file.h"
#include "include/c/extern/filesystem/read_only_memory_region.h"
#include "include/c/extern/filesystem/tree.h"
#include "include/c/extern/filesystem/writable_file.h"
#include "include/c/extern/plugin/registration.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"
#include "include/c/macros.h"

#define ATEN_XPU_DOMAIN aten_xpu::SyclPluginDomain::k_filesystem
#include "docs/aten_xpu/plugin_entry.h"

import std;
import aten_xpu;

extern "C"
{
    ATEN_XPU_PLUGIN_ENTRY("aten_xpu.filesystem")

    ATEN_XPU_EXPORT_OPS(filesystem, TF_FilesystemOps, &registry.getFilesystemOps())
    ATEN_XPU_EXPORT_OPS(
        random_access_file,
        TF_RandomAccessFileOps,
        &registry.getRandomAccessFileOps()
    )
    ATEN_XPU_EXPORT_OPS(writable_file, TF_WritableFileOps, &registry.getWritableFileOps())
    ATEN_XPU_EXPORT_OPS(
        read_only_memory_region,
        TF_ReadOnlyMemoryRegionOps,
        &registry.getReadOnlyMemoryRegionOps()
    )
    ATEN_XPU_EXPORT_OPS(filesystem_tree, TFFilesystemTreeOps, &registry.getFilesystemTreeOps())
}
