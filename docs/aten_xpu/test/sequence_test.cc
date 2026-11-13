#include "include/c/intern/array.h"
#include "include/c/intern/deque.h"
#include "include/c/intern/forward_list/forward_list.h"
#include "include/c/intern/hive/hive.h"
#include "include/c/intern/list/list.h"
#include "include/c/intern/span.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"
#include "include/c/intern/vector.h"

#include <gtest/gtest.h>

import std;
import cc_ice_intern_sonic;
import cc_ice_intern_list_sonic;
import cc_ice_intern_forward_list_sonic;
import cc_ice_intern_hive_sonic;
import aten_xpu;
import aten_xpu_test;

namespace aten_xpu {

class SequenceTest : public RegistryFixture
{};

// A created status that is destroyed with the scope.
class ScopedStatus
{
public:
    ScopedStatus() :
        m_status{&SyclPluginRegistry::getInstance().getStatusOps()}
    {
        m_status.create();
    }

    ~ScopedStatus()
    {
        m_status.destroy();
    }

    ScopedStatus(const ScopedStatus&) = delete;
    ScopedStatus& operator=(const ScopedStatus&) = delete;

    const ice::sonic::Status& get() const noexcept
    {
        return m_status;
    }

    TF_Code code() const
    {
        ice::sonic::Code code;
        m_status.get_code(code);

        return code.get_value();
    }

private:
    ice::sonic::Status m_status;
};

class ElementReader
{
public:
    static std::int32_t read_int(const void* element)
    {
        std::int32_t value = 0;
        std::memcpy(&value, element, sizeof(value));

        return value;
    }
};

TEST_F(SequenceTest, ArraySetsAndGets)
{
    ice::sonic::Array array{&SyclPluginRegistry::getInstance().getArrayOps()};
    array.create();
    array.set_element_size(sizeof(std::int32_t));
    array.set_count(3);
    ScopedStatus status;
    const std::int32_t value = 42;
    array.set(1, &value, status.get());
    const void* found = nullptr;
    array.get(1, &found, status.get());
    std::size_t size = 0;
    array.size(size);

    EXPECT_EQ(ElementReader::read_int(found), 42);
    EXPECT_EQ(size, 3u);
    EXPECT_EQ(status.code(), TF_OK);

    array.get(3, &found, status.get());
    EXPECT_EQ(status.code(), TF_OUT_OF_RANGE);
    array.destroy();
}

TEST_F(SequenceTest, VectorGrowsAndReserves)
{
    ice::sonic::Vector vector{&SyclPluginRegistry::getInstance().getVectorOps()};
    vector.create();
    vector.set_element_size(sizeof(std::int32_t));
    ScopedStatus status;
    vector.reserve(16, status.get());
    for (std::int32_t value = 10; value < 14; ++value) {
        vector.push_back(&value);
    }
    std::size_t size = 0;
    std::size_t capacity = 0;
    vector.size(size);
    vector.capacity(capacity);
    const void* found = nullptr;
    vector.get(3, &found, status.get());

    EXPECT_EQ(size, 4u);
    EXPECT_GE(capacity, 16u);
    EXPECT_EQ(ElementReader::read_int(found), 13);
    EXPECT_EQ(status.code(), TF_OK);

    vector.get(4, &found, status.get());
    EXPECT_EQ(status.code(), TF_OUT_OF_RANGE);
    vector.destroy();
}

TEST_F(SequenceTest, DequePushesAndPopsBothEnds)
{
    ice::sonic::Deque deque{&SyclPluginRegistry::getInstance().getDequeOps()};
    deque.create();
    deque.set_element_size(sizeof(std::int32_t));
    const std::int32_t middle = 2;
    const std::int32_t front = 1;
    const std::int32_t back = 3;
    deque.push_back(&middle);
    deque.push_front(&front);
    deque.push_back(&back);
    ScopedStatus status;
    const void* found = nullptr;
    deque.get(0, &found, status.get());
    EXPECT_EQ(ElementReader::read_int(found), 1);
    deque.pop_front();
    deque.pop_back();
    std::size_t size = 0;
    deque.size(size);
    deque.get(0, &found, status.get());

    EXPECT_EQ(size, 1u);
    EXPECT_EQ(ElementReader::read_int(found), 2);
    deque.destroy();
}

TEST_F(SequenceTest, BitSetFlipsAndCounts)
{
    ScopedStatus status;
    BitSet bits{
        &SyclPluginRegistry::getInstance().getStatusOps(),
        &SyclPluginRegistry::getInstance().getStringOps(),
        10
    };
    bits.set(2);
    bits.set(7);
    bits.clear(7);
    bits.flip(4, status.get());
    int result = 0;
    bits.test(4, result, status.get());
    std::size_t count = 0;
    bits.count(count);

    EXPECT_EQ(result, 1);
    EXPECT_EQ(count, 2u);
    EXPECT_EQ(status.code(), TF_OK);

    bits.test(10, result, status.get());
    EXPECT_EQ(status.code(), TF_OUT_OF_RANGE);
}

TEST_F(SequenceTest, SpanViewsAndSlices)
{
    ScopedStatus status;
    const std::array<std::int32_t, 4> source{5, 6, 7, 8};
    Span span{
        nullptr,
        &SyclPluginRegistry::getInstance().getStatusOps(),
        &SyclPluginRegistry::getInstance().getStringOps()
    };
    span.bind(source.data(), source.size(), sizeof(std::int32_t));
    const void* found = nullptr;
    span.get(2, &found, status.get());
    std::size_t size = 0;
    span.size(size);

    EXPECT_EQ(ElementReader::read_int(found), 7);
    EXPECT_EQ(size, 4u);

    span.get(4, &found, status.get());
    EXPECT_EQ(status.code(), TF_OUT_OF_RANGE);
}

TEST_F(SequenceTest, SubspanBindsASpan)
{
    ScopedStatus status;
    const std::array<std::int32_t, 4> source{5, 6, 7, 8};
    Span span{
        nullptr,
        &SyclPluginRegistry::getInstance().getStatusOps(),
        &SyclPluginRegistry::getInstance().getStringOps()
    };
    span.bind(source.data(), source.size(), sizeof(std::int32_t));
    ice::sonic::Span slice{&SyclPluginRegistry::getInstance().getSpanOps()};
    slice.create();
    span.subspan(1, 2, slice, status.get());
    std::size_t size = 0;
    slice.size(size);
    const void* found = nullptr;
    slice.get(0, &found, status.get());

    EXPECT_EQ(status.code(), TF_OK);
    EXPECT_EQ(size, 2u);
    EXPECT_EQ(ElementReader::read_int(found), 6);

    span.subspan(3, 2, slice, status.get());
    EXPECT_EQ(status.code(), TF_OUT_OF_RANGE);
    slice.destroy();
}

TEST_F(SequenceTest, ListKeepsNodesStable)
{
    ice::sonic::List list{&SyclPluginRegistry::getInstance().getListOps()};
    list.create();
    list.set_element_size(sizeof(std::int32_t));
    ScopedStatus status;
    ::TFListNode first{};
    ::TFListNode second{};
    const std::int32_t one = 1;
    const std::int32_t two = 2;
    list.push_back(&one, &first, status.get());
    list.push_back(&two, &second, status.get());
    std::vector<std::int32_t> seen;
    list.for_each(
        [](void* capture, const void* element)
        {
            static_cast<std::vector<std::int32_t>*>(capture)->push_back(
                ElementReader::read_int(element)
            );
        },
        &seen
    );
    list.erase(&first);
    std::size_t size = 0;
    list.size(size);

    ASSERT_EQ(seen.size(), 2u);
    EXPECT_EQ(seen[1], 2);
    EXPECT_EQ(size, 1u);
    list.destroy();
}

TEST_F(SequenceTest, ForwardListErasesAfterANode)
{
    ice::sonic::ForwardList list{&SyclPluginRegistry::getInstance().getForwardListOps()};
    list.create();
    list.set_element_size(sizeof(std::int32_t));
    ScopedStatus status;
    ::TFForwardListNode node{};
    for (std::int32_t value = 3; value >= 1; --value) {
        list.push_front(&value, &node, status.get());
    }
    ::TFForwardListNode head{};
    const std::int32_t zero = 0;
    list.push_front(&zero, &head, status.get());
    list.erase_after(&head);
    std::vector<std::int32_t> seen;
    list.for_each(
        [](void* capture, const void* element)
        {
            static_cast<std::vector<std::int32_t>*>(capture)->push_back(
                ElementReader::read_int(element)
            );
        },
        &seen
    );

    ASSERT_EQ(seen.size(), 3u);
    EXPECT_EQ(seen[0], 0);
    EXPECT_EQ(seen[1], 2);
    list.destroy();
}

TEST_F(SequenceTest, HiveSlotsStayValidUntilErased)
{
    ice::sonic::Hive hive{&SyclPluginRegistry::getInstance().getHiveOps()};
    hive.create();
    hive.set_element_size(sizeof(std::int32_t));
    ScopedStatus status;
    ::TFHiveSlot first{};
    ::TFHiveSlot second{};
    const std::int32_t one = 1;
    const std::int32_t two = 2;
    hive.insert(&one, &first, status.get());
    hive.insert(&two, &second, status.get());
    const void* found = nullptr;
    hive.get(&second, &found, status.get());
    EXPECT_EQ(ElementReader::read_int(found), 2);

    ::TFHiveSlot stale = first;
    hive.erase(&first, status.get());
    EXPECT_EQ(status.code(), TF_OK);
    hive.get(&stale, &found, status.get());
    EXPECT_EQ(status.code(), TF_NOT_FOUND);
    std::size_t size = 0;
    hive.size(size);
    EXPECT_EQ(size, 1u);
    hive.destroy();
}

} // namespace aten_xpu
