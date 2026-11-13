#include "include/c/intern/attrtype.h"
#include "include/c/intern/complex.h"
#include "include/c/intern/datatype.h"
#include "include/c/intern/duration.h"
#include "include/c/intern/file_statistics.h"
#include "include/c/intern/hash.h"
#include "include/c/intern/shape.h"
#include "include/c/intern/status.h"
#include "include/c/intern/time_point.h"
#include "include/c/intern/tstring.h"

#include <gtest/gtest.h>

import std;
import cc_ice_intern_sonic;
import aten_xpu;
import aten_xpu_test;

namespace aten_xpu {

class ValueTest : public RegistryFixture
{};

class ValueText
{
public:
    ValueText() :
        m_text{&SyclPluginRegistry::getInstance().getStringOps()}
    {
        m_text.create();
    }

    ~ValueText()
    {
        m_text.destroy();
    }

    ValueText(const ValueText&) = delete;
    ValueText& operator=(const ValueText&) = delete;

    const ice::sonic::String& get() const noexcept
    {
        return m_text;
    }

    std::string value() const
    {
        return std::string{String::view_of(m_text)};
    }

private:
    ice::sonic::String m_text;
};

TEST_F(ValueTest, ComplexStoresBothParts)
{
    ice::sonic::Complex number{&SyclPluginRegistry::getInstance().getComplexOps()};
    number.create();
    number.set_real(1.5);
    number.set_imag(-2.0);
    double real = 0.0;
    double imag = 0.0;
    number.get_real(real);
    number.get_imag(imag);

    EXPECT_EQ(real, 1.5);
    EXPECT_EQ(imag, -2.0);
    number.destroy();
}

TEST_F(ValueTest, TimePointFillsADuration)
{
    TimePoint point{&SyclPluginRegistry::getInstance().getDurationOps()};
    point.set(5, 1, 1'000);
    ice::sonic::Duration duration{&SyclPluginRegistry::getInstance().getDurationOps()};
    duration.create();
    point.get_duration_since_epoch(duration);
    std::int64_t ticks = 0;
    std::int64_t num = 0;
    std::int64_t den = 0;
    duration.get_ticks(ticks);
    duration.get_ratio_num(num);
    duration.get_ratio_den(den);

    EXPECT_EQ(ticks, 5);
    EXPECT_EQ(num, 1);
    EXPECT_EQ(den, 1'000);
    duration.destroy();
}

TEST_F(ValueTest, TimePointThroughItsVtableStartsAtTheEpoch)
{
    ice::sonic::TimePoint point{&SyclPluginRegistry::getInstance().getTimePointOps()};
    point.create();
    ice::sonic::Duration duration{&SyclPluginRegistry::getInstance().getDurationOps()};
    duration.create();
    point.get_duration_since_epoch(duration);
    std::int64_t ticks = -1;
    duration.get_ticks(ticks);

    EXPECT_EQ(ticks, 0);
    duration.destroy();
    point.destroy();
}

TEST_F(ValueTest, DataTypeReportsElementSizes)
{
    ice::sonic::DataType types{&SyclPluginRegistry::getInstance().getDataTypeOps()};
    types.create();
    std::size_t half = 0;
    std::size_t floating = 0;
    types.datatype_size(ice::sonic::DataTypeEnum::k_half, half);
    types.datatype_size(ice::sonic::DataTypeEnum::k_float, floating);

    EXPECT_EQ(half, 2u);
    EXPECT_EQ(floating, 4u);
    types.destroy();
}

TEST_F(ValueTest, AttrTypeNamesTheEnum)
{
    ice::sonic::AttrType types{&SyclPluginRegistry::getInstance().getAttrTypeOps()};
    types.create();
    ValueText name;
    types.attrtype_name(ice::sonic::AttrTypeEnum::k_shape, name.get());

    EXPECT_EQ(name.value(), "shape");
    types.destroy();
}

TEST_F(ValueTest, ShapeKeepsDimsAndReportsUnknownRank)
{
    ice::sonic::Shape shape{&SyclPluginRegistry::getInstance().getShapeOps()};
    shape.create();
    const std::vector<std::int64_t> dims{2, 3, 4};
    shape.set_dims(dims);
    int rank = 0;
    std::int64_t second = 0;
    std::int64_t missing = 0;
    shape.shape_num_dims(rank);
    shape.shape_dim(1, second);
    shape.shape_dim(7, missing);

    EXPECT_EQ(rank, 3);
    EXPECT_EQ(second, 3);
    EXPECT_EQ(missing, -1);
    shape.delete_shape();
}

TEST_F(ValueTest, HashMatchesFnvAndCombines)
{
    ice::sonic::Hash hash{&SyclPluginRegistry::getInstance().getHashOps()};
    hash.create();
    ice::sonic::Status status{&SyclPluginRegistry::getInstance().getStatusOps()};
    status.create();
    std::size_t empty = 0;
    std::size_t letter = 0;
    std::size_t combined = 0;
    const char letter_a = 'a';
    hash.hash_bytes(nullptr, 0, empty, status);
    hash.hash_bytes(&letter_a, 1, letter, status);
    hash.hash_combine(1, 2, combined, status);
    ice::sonic::Code code;
    status.get_code(code);

    EXPECT_EQ(empty, 14'695'981'039'346'656'037ULL);
    EXPECT_EQ(letter, 0xAF'63'DC'4C'86'01'EC'8CULL);
    EXPECT_NE(combined, 0u);
    EXPECT_EQ(code.get_value(), TF_OK);

    std::size_t ignored = 0;
    hash.hash_bytes(nullptr, 4, ignored, status);
    status.get_code(code);
    EXPECT_EQ(code.get_value(), TF_INVALID_ARGUMENT);
    status.destroy();
    hash.destroy();
}

TEST_F(ValueTest, FileStatisticsRoundTrips)
{
    ice::sonic::FileStatistics stats{&SyclPluginRegistry::getInstance().getFileStatisticsOps()};
    stats.create();
    stats.set_is_directory(1);
    stats.set_length(4'096);
    stats.set_mtime_nsec(77);
    int is_directory = 0;
    std::int64_t length = 0;
    std::int64_t mtime = 0;
    stats.is_directory(is_directory);
    stats.length(length);
    stats.mtime_nsec(mtime);

    EXPECT_EQ(is_directory, 1);
    EXPECT_EQ(length, 4'096);
    EXPECT_EQ(mtime, 77);
    stats.destroy();
}

} // namespace aten_xpu
