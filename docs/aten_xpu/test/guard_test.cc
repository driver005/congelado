#include <gtest/gtest.h>

import std;
import cc_ice_intern_sonic;
import aten_xpu;
import aten_xpu_test;

namespace aten_xpu {

class SyclGuardTest : public SyclTestFixture
{};

TEST_F(SyclGuardTest, RestoresOriginalDevice)
{
    auto& platform = SyclHandle::resolve<SyclPlatform>(getPlatform());
    const int target = getDeviceCount() - 1;
    {
        SyclDeviceGuard guard{platform, target};
        int current = -1;
        getPlatform().get_current_device(&current, getStatus());
        EXPECT_EQ(current, target);
        EXPECT_EQ(guard.getOriginalIndex(), 0);
    }
    int restored = -1;
    getPlatform().get_current_device(&restored, getStatus());
    EXPECT_EQ(restored, 0);
}

TEST_F(SyclGuardTest, InvalidIndexIsIgnored)
{
    auto& platform = SyclHandle::resolve<SyclPlatform>(getPlatform());
    SyclDeviceGuard guard{platform, getDeviceCount() + 4};
    int current = -1;
    getPlatform().get_current_device(&current, getStatus());
    EXPECT_EQ(current, 0);
}

} // namespace aten_xpu
