#include "include/c/extern/profiler/profiler.h"

#include <cstdlib>
#include <gtest/gtest.h>

import std;
import cc_ice_intern_sonic;
import cc_ice_extern_stream_executor_sonic;
import aten_xpu;
import aten_xpu_test;

namespace aten_xpu {

class SyclProfilerTest : public SyclTestFixture
{
protected:
    SyclProfilerTest()
    {
        // Queues get SYCL profiling only if this is set before the fixture creates them.
        ::setenv("ATEN_XPU_PROFILING", "1", 1);
    }
};

TEST_F(SyclProfilerTest, ReportsNameAndDeviceType)
{
    auto profiler = new SyclProfiler{SyclOpsTable::getInstance()};
    ice::sonic::String name{SyclOpsTable::getInstance().getStringOps()};
    name.create();
    profiler->get_name(name);
    const char* data = nullptr;
    name.get_data_pointer(&data);
    EXPECT_EQ(std::string_view{data}, "aten_xpu");
    name.destroy();
    profiler->destroy();
}

TEST_F(SyclProfilerTest, CollectsCopyIntoXSpace)
{
    auto stream = make_stream();
    auto allocator = make_allocator();
    ice::sonic::DeviceMemoryBase memory;
    allocator.allocate(4'096, ice::sonic::MemorySpace::k_device, stream, &memory, getStatus());

    auto profiler = new SyclProfiler{SyclOpsTable::getInstance()};
    profiler->start(getStatus());
    ASSERT_TRUE(ok());

    std::array<std::byte, 4'096> host{};
    getExecutor().memcpy_htod(getDevice(), stream, &memory, host.data(), host.size(), getStatus());
    stream.synchronize(getStatus());
    profiler->stop(getStatus());

    ::TF_Tensor* tensor = nullptr;
    profiler->collect_data_xspace(&tensor, getStatus());
    ASSERT_TRUE(ok());
    ASSERT_NE(tensor, nullptr);
    auto& bytes = static_cast<SyclTensor&>(SyclTensor::from_handle(tensor));
    std::size_t size = 0;
    bytes.tensor_byte_size(size);
    EXPECT_GT(size, 0U);

    bytes.delete_tensor();
    profiler->destroy();
    allocator.deallocate(&memory);
    release_allocator(allocator);
    release_stream(stream);
}

} // namespace aten_xpu
