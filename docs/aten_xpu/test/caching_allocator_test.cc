#include "include/c/extern/stream_executor/allocator.h"
#include "include/c/extern/stream_executor/mem_pool.h"

#include <gtest/gtest.h>

import std;
import cc_ice_intern_sonic;
import cc_ice_extern_stream_executor_sonic;
import aten_xpu;
import aten_xpu_test;

namespace aten_xpu {

class SyclCachingAllocatorTest : public SyclTestFixture
{
protected:
    TF_DeviceMemoryBase allocate(const ice::sonic::TF_AllocatorOps& allocator, const ice::sonic::TF_StreamOps& stream, uint64_t size)
    {

        TF_DeviceMemoryBase memory{};
        allocator.allocate(size, TF_MEMORY_SPACE_DEVICE, stream, &memory, getStatus());
        return memory;

    }

    TF_AllocatorStats stats(const ice::sonic::TF_AllocatorOps& allocator)
    {

        TF_AllocatorStats result{};
        _Bool success = false;
        allocator.get_stats(&result, &success);
        return result;

    }
};

TEST_F(SyclCachingAllocatorTest, ReusesCachedBlocks)
{

    auto stream = make_stream();
    auto allocator = make_allocator();
    auto first = allocate(allocator, stream, 4096);
    ASSERT_TRUE(ok());
    void* first_pointer = first.opaque;
    allocator.deallocate(&first);

    auto second = allocate(allocator, stream, 4096);
    EXPECT_EQ(second.opaque, first_pointer);
    EXPECT_EQ(stats(allocator).bytes_reserved, 2 << 20);

    allocator.deallocate(&second);
    allocator.empty_cache(getStatus());
    EXPECT_EQ(stats(allocator).bytes_reserved, 0);
    release_allocator(allocator);
    release_stream(stream);

}

TEST_F(SyclCachingAllocatorTest, RecordStreamDefersReuse)
{

    auto stream = make_stream();
    auto other = make_stream();
    auto allocator = make_allocator();
    auto memory = allocate(allocator, stream, 1024);
    allocator.record_stream(&memory, other, getStatus());
    allocator.deallocate(&memory);
    other.synchronize(getStatus());

    auto again = allocate(allocator, stream, 1024);
    EXPECT_TRUE(ok());
    allocator.deallocate(&again);
    release_allocator(allocator);
    release_stream(other);
    release_stream(stream);

}

TEST_F(SyclCachingAllocatorTest, NoSplitKeepsBlocksWhole)
{

    auto stream = make_stream();
    auto allocator = make_allocator();
    allocator.set_option(TF_ALLOCATOR_OPTION_NO_SPLIT, 1, getStatus());
    auto memory = allocate(allocator, stream, 1024);
    uint64_t size = 0;
    void* base = nullptr;
    allocator.get_base_allocation(memory.opaque, &base, &size, getStatus());
    EXPECT_EQ(base, memory.opaque);
    EXPECT_EQ(size, 2U << 20U);
    allocator.deallocate(&memory);
    release_allocator(allocator);
    release_stream(stream);

}

TEST_F(SyclCachingAllocatorTest, MemPoolCapturesAllocations)
{

    auto stream = make_stream();
    auto allocator = make_allocator();
    ice::sonic::TF_MemPoolOps pool{SyclOpsTable::getInstance().getMemPoolOps()};
    pool.create();
    allocator.create_mem_pool_internal(nullptr, true, pool, getStatus());
    pool.begin_allocate_to_pool(nullptr, nullptr, getStatus());
    auto memory = allocate(allocator, stream, 1024);
    pool.end_allocate_to_pool(getStatus());

    TF_PoolId pool_id{};
    pool.get_id(&pool_id);
    EXPECT_GT(pool_id.second, 0);
    allocator.deallocate(&memory);
    pool.release(getStatus());
    allocator.destroy_mem_pool_internal(pool);
    pool.destroy();
    release_allocator(allocator);
    release_stream(stream);

}

TEST_F(SyclCachingAllocatorTest, MemoryFractionLimitsReservation)
{

    auto stream = make_stream();
    auto allocator = make_allocator();
    allocator.set_memory_fraction(0.0, getStatus());
    auto memory = allocate(allocator, stream, 1024);
    EXPECT_FALSE(ok());
    EXPECT_EQ(memory.opaque, nullptr);
    release_allocator(allocator);
    release_stream(stream);

}

} // namespace aten_xpu
