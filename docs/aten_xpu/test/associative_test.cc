#include "include/c/intern/map.h"
#include "include/c/intern/set.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"

#include <gtest/gtest.h>

import std;
import cc_ice_intern_sonic;
import aten_xpu;
import aten_xpu_test;

namespace aten_xpu {

class AssociativeTest : public RegistryFixture
{};

TEST_F(AssociativeTest, MapInsertFindEraseAndOrder)
{
    ice::sonic::Map map{&SyclPluginRegistry::getInstance().getMapOps()};
    map.create();
    ice::sonic::Status status{&SyclPluginRegistry::getInstance().getStatusOps()};
    status.create();
    for (const std::int64_t key: {30, 10, 20}) {
        const double value = key / 10.0;
        map.insert(&key, &value, status);
    }
    const std::int64_t replaced_key = 20;
    const double replaced = 9.5;
    map.insert(&replaced_key, &replaced, status);

    const void* found = nullptr;
    map.find(&replaced_key, &found, status);
    int has_ten = 0;
    const std::int64_t missing = 99;
    int has_missing = 1;
    map.contains(&replaced_key, has_ten, status);
    map.contains(&missing, has_missing, status);
    std::size_t size = 0;
    map.size(size);
    std::vector<std::int64_t> order;
    map.for_each(
        [](void* capture, const void* key, const void* value)
        {
            static_cast<void>(value);
            std::int64_t copy = 0;
            std::memcpy(&copy, key, sizeof(copy));
            static_cast<std::vector<std::int64_t>*>(capture)->push_back(copy);
        },
        &order
    );

    ASSERT_TRUE(found != nullptr);
    double stored = 0.0;
    std::memcpy(&stored, found, sizeof(stored));
    EXPECT_EQ(stored, 9.5);
    EXPECT_EQ(has_ten, 1);
    EXPECT_EQ(has_missing, 0);
    EXPECT_EQ(size, 3u);
    ASSERT_EQ(order.size(), 3u);

    map.erase(&missing, status);
    ice::sonic::Code code;
    status.get_code(code);
    EXPECT_EQ(code.get_value(), TF_NOT_FOUND);
    status.destroy();
    map.destroy();
}

TEST_F(AssociativeTest, SetDeduplicatesAndFinds)
{
    ice::sonic::Set set{&SyclPluginRegistry::getInstance().getSetOps()};
    set.create();
    ice::sonic::Status status{&SyclPluginRegistry::getInstance().getStatusOps()};
    status.create();
    const std::int64_t first = 4;
    const std::int64_t second = 8;
    set.insert(&first, status);
    set.insert(&second, status);
    set.insert(&first, status);
    std::size_t size = 0;
    set.size(size);
    const void* found = nullptr;
    set.find(&second, &found, status);
    int has_first = 0;
    set.contains(&first, has_first, status);
    set.erase(&first, status);
    set.contains(&first, has_first, status);

    EXPECT_EQ(size, 2u);
    EXPECT_TRUE(found != nullptr);
    EXPECT_EQ(has_first, 0);
    status.destroy();
    set.destroy();
}

} // namespace aten_xpu
