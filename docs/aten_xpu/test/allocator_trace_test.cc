#include "include/c/extern/stream_executor/allocator.h"

#include <gtest/gtest.h>

import std;
import cc_ice_intern_sonic;
import cc_ice_extern_stream_executor_sonic;
import aten_xpu;
import aten_xpu_test;

namespace aten_xpu {

class SyclAllocatorTraceTest : public SyclTestFixture
{};

TEST_F(SyclAllocatorTraceTest, TrackAllocateAndFree)
{
    auto stream = make_stream();
    auto allocator = make_allocator();
    std::map<SyclAllocatorTrace::Action, int> seen;
    SyclHandle::resolve<SyclAllocator>(allocator).getTrace().addTracker(
        [&seen](const SyclAllocatorTrace::Entry& entry)
        {
            ++seen[std::get<0>(entry)];
        }
    );

    TF_DeviceMemoryBase memory{};
    allocator.allocate(1'024, TF_MEMORY_SPACE_DEVICE, stream, &memory, getStatus());
    allocator.deallocate(&memory);
    EXPECT_EQ(seen[SyclAllocatorTrace::Action::segment_alloc], 1);
    EXPECT_EQ(seen[SyclAllocatorTrace::Action::alloc], 1);
    EXPECT_EQ(seen[SyclAllocatorTrace::Action::free_requested], 1);
    EXPECT_EQ(seen[SyclAllocatorTrace::Action::free_completed], 1);

    release_allocator(allocator);
    release_stream(stream);
}

} // namespace aten_xpu
