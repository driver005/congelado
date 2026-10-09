#include "include/c/extern/stream_executor/stream.h"

#include <gtest/gtest.h>

import std;
import cc_ice_intern_sonic;
import aten_xpu;
import aten_xpu_test;

namespace aten_xpu {

class SyclStreamTest : public SyclTestFixture
{};

TEST_F(SyclStreamTest, PriorityIsKept)
{
    for (const int32_t priority: {-1, 0, 1}) {
        auto stream = make_stream(priority);
        int32_t reported = 2;
        stream.get_priority(&reported);
        EXPECT_EQ(reported, priority);
        release_stream(stream);
    }
}

TEST_F(SyclStreamTest, QueryAndSynchronize)
{
    auto stream = make_stream();
    stream.synchronize(getStatus());
    _Bool idle = false;
    stream.query(&idle, getStatus());
    EXPECT_TRUE(ok());
    EXPECT_TRUE(idle);
    release_stream(stream);
}

TEST_F(SyclStreamTest, StreamPoolRoundRobin)
{
    constexpr std::size_t k_rounds = SyclStreamPool::k_streams_per_priority + 1;
    std::set<void*> handles;
    void* first = nullptr;
    for (std::size_t round = 0; round < k_rounds; ++round) {
        ice::sonic::TF_StreamOps stream{SyclOpsTable::getInstance().getStreamOps()};
        stream.create();
        getExecutor().get_stream_from_pool(getDevice(), 0, stream, getStatus());
        void* native = nullptr;
        stream.get_native_handle(&native);
        if (round == 0) {
            first = native;
        }
        handles.insert(native);
        if (round + 1 == k_rounds) {
            EXPECT_EQ(native, first);
        }
        stream.destroy();
    }
    EXPECT_EQ(handles.size(), SyclStreamPool::k_streams_per_priority);
}

TEST_F(SyclStreamTest, CurrentStreamIsPerThread)
{
    auto stream = make_stream();
    getExecutor().set_current_stream(getDevice(), stream, getStatus());

    ice::sonic::TF_StreamOps current{SyclOpsTable::getInstance().getStreamOps()};
    current.create();
    getExecutor().get_current_stream(getDevice(), current, getStatus());
    void* expected = nullptr;
    void* actual = nullptr;
    stream.get_native_handle(&expected);
    current.get_native_handle(&actual);
    EXPECT_EQ(expected, actual);

    void* other_thread = nullptr;
    std::thread worker{
        [&]()
        {
            ice::sonic::TF_StreamOps worker_stream{SyclOpsTable::getInstance().getStreamOps()};
            worker_stream.create();
            getExecutor().get_current_stream(getDevice(), worker_stream, getStatus());
            worker_stream.get_native_handle(&other_thread);
            worker_stream.destroy();
        }
    };
    worker.join();
    EXPECT_NE(other_thread, expected);

    current.destroy();
    release_stream(stream);
}

TEST_F(SyclStreamTest, ExternalQueue)
{
    auto stream = make_stream();
    void* native = nullptr;
    stream.get_native_handle(&native);

    ice::sonic::TF_StreamOps external{SyclOpsTable::getInstance().getStreamOps()};
    external.create();
    getExecutor().create_stream_from_native(getDevice(), native, external, getStatus());
    EXPECT_TRUE(ok());
    external.synchronize(getStatus());
    EXPECT_TRUE(ok());
    external.destroy();
    release_stream(stream);
}

} // namespace aten_xpu
