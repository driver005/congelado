#include "include/c/extern/stream_executor/allocator.h"

#include <gtest/gtest.h>

import std;
import cc_ice_intern_sonic;
import cc_ice_extern_stream_executor_sonic;
import aten_xpu;
import aten_xpu_test;

namespace aten_xpu {

class SyclAllocatorIpcTest : public SyclTestFixture
{
};

TEST_F(SyclAllocatorIpcTest, ShareOffsetSubAllocation)
{

    auto stream = make_stream();
    auto allocator = make_allocator();
    TF_DeviceMemoryBase first{};
    TF_DeviceMemoryBase second{};
    allocator.allocate(1024, TF_MEMORY_SPACE_DEVICE, stream, &first, getStatus());
    allocator.allocate(1024, TF_MEMORY_SPACE_DEVICE, stream, &second, getStatus());

    TF_IpcMemoryHandle handle{};
    allocator.export_memory(&second, &handle, getStatus());
    if (!ok()) {
        GTEST_SKIP() << "IPC memory is not supported here";
    }

    TF_DeviceMemoryBase opened{};
    allocator.open_memory(&handle, &opened, getStatus());
    EXPECT_TRUE(ok());
    EXPECT_NE(opened.opaque, nullptr);
    allocator.close_memory(&opened, getStatus());
    EXPECT_TRUE(ok());

    allocator.deallocate(&second);
    allocator.deallocate(&first);
    release_allocator(allocator);
    release_stream(stream);

}

TEST_F(SyclAllocatorIpcTest, CloseUnknownPointerFails)
{

    auto allocator = make_allocator();
    int marker = 0;
    TF_DeviceMemoryBase memory{.struct_size = sizeof(TF_DeviceMemoryBase), .opaque = &marker};
    allocator.close_memory(&memory, getStatus());
    EXPECT_FALSE(ok());
    release_allocator(allocator);

}

} // namespace aten_xpu
