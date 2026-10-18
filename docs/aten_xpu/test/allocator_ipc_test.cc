#include "include/c/extern/stream_executor/allocator.h"

#include <gtest/gtest.h>

import std;
import cc_ice_intern_sonic;
import cc_ice_extern_stream_executor_sonic;
import aten_xpu;
import aten_xpu_test;

namespace aten_xpu {

class SyclAllocatorIpcTest : public SyclTestFixture
{};

TEST_F(SyclAllocatorIpcTest, ShareOffsetSubAllocation)
{
    auto stream = make_stream();
    auto allocator = make_allocator();
    ice::sonic::DeviceMemoryBase first;
    ice::sonic::DeviceMemoryBase second;
    allocator.allocate(1'024, ice::sonic::MemorySpace::k_device, stream, &first, getStatus());
    allocator.allocate(1'024, ice::sonic::MemorySpace::k_device, stream, &second, getStatus());

    ice::sonic::IpcMemoryHandle handle;
    allocator.export_memory(&second, &handle, getStatus());
    if (!ok()) {
        GTEST_SKIP() << "IPC memory is not supported here";
    }

    ice::sonic::DeviceMemoryBase opened;
    allocator.open_memory(&handle, &opened, getStatus());
    EXPECT_TRUE(ok());
    EXPECT_NE(opened.getOpaque(), nullptr);
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
    ice::sonic::DeviceMemoryBase memory;
    memory.setOpaque(&marker);
    allocator.close_memory(&memory, getStatus());
    EXPECT_FALSE(ok());
    release_allocator(allocator);
}

} // namespace aten_xpu
