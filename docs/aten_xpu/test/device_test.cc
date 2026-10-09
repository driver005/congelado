#include "include/c/extern/stream_executor/device.h"

#include <gtest/gtest.h>

import std;
import cc_ice_intern_sonic;
import aten_xpu;
import aten_xpu_test;

namespace aten_xpu {

class SyclDeviceTest : public SyclTestFixture
{};

TEST_F(SyclDeviceTest, CurrentDeviceRoundTrip)
{
    int current = -1;
    getPlatform().get_current_device(&current, getStatus());
    EXPECT_EQ(current, 0);

    getPlatform().set_current_device(getDeviceCount() - 1, getStatus());
    getPlatform().get_current_device(&current, getStatus());
    EXPECT_EQ(current, getDeviceCount() - 1);
    getPlatform().set_current_device(0, getStatus());
    EXPECT_TRUE(ok());
}

TEST_F(SyclDeviceTest, DeviceProperties)
{
    TF_DeviceProperties properties{};
    getDevice().get_device_properties(&properties, getStatus());
    EXPECT_TRUE(ok());
    EXPECT_GT(properties.global_mem_size, 0U);
    EXPECT_GT(properties.max_compute_units, 0U);
    EXPECT_GT(properties.num_sub_group_sizes, 0U);
}

TEST_F(SyclDeviceTest, PointerGetDevice)
{
    auto allocator = make_allocator();
    TF_DeviceMemoryBase memory{};
    ice::sonic::TF_StreamOps no_stream{SyclOpsTable::getInstance().getStreamOps()};
    allocator.allocate(1'024, TF_MEMORY_SPACE_DEVICE, no_stream, &memory, getStatus());
    ASSERT_TRUE(ok());

    int owner = -1;
    getPlatform().get_device_for_pointer(memory.opaque, &owner, getStatus());
    EXPECT_EQ(owner, 0);

    allocator.deallocate(&memory);
    release_allocator(allocator);
}

} // namespace aten_xpu
