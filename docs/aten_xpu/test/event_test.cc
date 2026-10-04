#include "include/c/extern/stream_executor/event.h"

#include <gtest/gtest.h>
#include <sycl/sycl.hpp>

import std;
import cc_ice_intern_sonic;
import cc_ice_extern_stream_executor_sonic;
import aten_xpu;
import aten_xpu_test;

namespace aten_xpu {

class SyclEventTest : public SyclTestFixture
{
protected:
    ice::sonic::TF_EventOps make_event(bool enable_timing, bool enable_ipc)
    {

        ice::sonic::TF_EventOps event{SyclOpsTable::getInstance().getEventOps()};
        event.create();
        const TF_EventOptions options{
            .struct_size = sizeof(TF_EventOptions),
            .enable_timing = enable_timing,
            .enable_ipc = enable_ipc,
            .reusable = enable_ipc
        };
        getExecutor().create_event_with_options_internal(getDevice(), &options, event, getStatus());
        return event;

    }
};

TEST_F(SyclEventTest, RecordAndQuery)
{

    auto stream = make_stream();
    auto event = make_event(false, false);
    getExecutor().record_event(getDevice(), stream, event, getStatus());
    getExecutor().block_host_for_event(getDevice(), event, getStatus());

    TF_EventStatus status = TF_EVENT_PENDING;
    getExecutor().get_event_status(getDevice(), event, &status);
    EXPECT_EQ(status, TF_EVENT_COMPLETE);
    EXPECT_TRUE(ok());

    event.destroy();
    release_stream(stream);

}

TEST_F(SyclEventTest, ElapsedTimeNeedsTiming)
{

    auto stream = make_stream();
    auto start = make_event(true, false);
    auto stop = make_event(true, false);
    getExecutor().record_event(getDevice(), stream, start, getStatus());
    getExecutor().record_event(getDevice(), stream, stop, getStatus());

    float milliseconds = -1.0F;
    start.elapsed_time(stop, &milliseconds, getStatus());
    EXPECT_TRUE(ok());
    EXPECT_GE(milliseconds, 0.0F);

    stop.destroy();
    start.destroy();
    release_stream(stream);

}

TEST_F(SyclEventTest, IpcRoundTrip)
{

    void* native = nullptr;
    getDevice().get_native_handle(&native);
    if (!static_cast<sycl::device*>(native)->has(sycl::aspect::ext_oneapi_ipc_event)) {
        GTEST_SKIP() << "device does not support IPC events";
    }

    auto stream = make_stream();
    auto event = make_event(false, true);
    getExecutor().record_event(getDevice(), stream, event, getStatus());
    TF_IpcEventHandle handle{};
    event.export_ipc(&handle, getStatus());
    ASSERT_TRUE(ok());

    ice::sonic::TF_EventOps imported{SyclOpsTable::getInstance().getEventOps()};
    imported.create();
    getExecutor().create_event_from_ipc_internal(getDevice(), &handle, imported, getStatus());
    EXPECT_TRUE(ok());

    imported.destroy();
    event.destroy();
    release_stream(stream);

}

} // namespace aten_xpu
