module;

#include "include/c/extern/stream_executor/executor.h"
#include "include/c/intern/status.h"

#include <gtest/gtest.h>

export module aten_xpu_test;

import std;
import cc_ice_intern_sonic;
import cc_ice_extern_stream_executor_sonic;
import aten_xpu;

export namespace aten_xpu {

class SyclTestFixture : public ::testing::Test
{
protected:
    void SetUp() override
    {
        auto& registry = SyclPluginRegistry::getInstance();
        registry.initialize();
        auto& ops = registry.getOps();

        m_status.emplace(ops.getStatusOps());
        m_status->create();
        m_platform.emplace(&registry.getPlatformOps());
        m_platform->create();
        m_platform->get_device_count(&m_device_count, *m_status);
        if (m_device_count == 0) {
            GTEST_SKIP() << "no XPU device available";
        }

        m_device.emplace(ops.getDeviceOps());
        m_device->create();
        m_platform->create_device_internal(*m_device, *m_status);
        m_executor.emplace(ops.getExecutorOps());
        m_executor->create();
        m_platform->create_executor_internal(*m_executor, *m_status);
        ASSERT_TRUE(ok());
    }

    void TearDown() override
    {
        if (m_executor) {
            m_platform->destroy_executor_internal(*m_executor);
            m_executor->destroy();
        }
        if (m_device) {
            m_platform->destroy_device_internal(*m_device);
            m_device->destroy();
        }
        if (m_platform) {
            m_platform->destroy();
        }
        if (m_status) {
            m_status->destroy();
        }
    }

    ice::sonic::TF_StreamOps make_stream(int32_t priority = 0)
    {
        ice::sonic::TF_StreamOps stream{SyclOpsTable::getInstance().getStreamOps()};
        stream.create();
        const TF_StreamOptions options{
            .struct_size = sizeof(TF_StreamOptions),
            .priority = priority
        };
        m_executor->create_stream_with_options(*m_device, &options, stream, *m_status);
        return stream;
    }

    ice::sonic::TF_AllocatorOps make_allocator()
    {
        ice::sonic::TF_AllocatorOps allocator{SyclOpsTable::getInstance().getAllocatorOps()};
        allocator.create();
        m_executor->create_allocator_internal(*m_device, allocator, *m_status);
        return allocator;
    }

    void release_stream(const ice::sonic::TF_StreamOps& stream)
    {
        m_executor->destroy_stream_internal(*m_device, stream);
        stream.destroy();
    }

    void release_allocator(const ice::sonic::TF_AllocatorOps& allocator)
    {
        m_executor->destroy_allocator_internal(*m_device, allocator);
        allocator.destroy();
    }

    bool ok() const noexcept
    {
        TF_Code code = TF_OK;
        m_status->get_code(&code);
        return code == TF_OK;
    }

    int getDeviceCount() const noexcept
    {
        return m_device_count;
    }

    const ice::sonic::Status& getStatus() const noexcept
    {
        return *m_status;
    }

    const ice::sonic::TF_PlatformOps& getPlatform() const noexcept
    {
        return *m_platform;
    }

    const ice::sonic::TF_DeviceOps& getDevice() const noexcept
    {
        return *m_device;
    }

    const ice::sonic::TF_ExecutorOps& getExecutor() const noexcept
    {
        return *m_executor;
    }

private:
    int m_device_count{0};
    std::optional<ice::sonic::Status> m_status;
    std::optional<ice::sonic::TF_PlatformOps> m_platform;
    std::optional<ice::sonic::TF_DeviceOps> m_device;
    std::optional<ice::sonic::TF_ExecutorOps> m_executor;
};

} // namespace aten_xpu
