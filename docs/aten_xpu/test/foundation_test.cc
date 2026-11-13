#include "include/c/intern/buffer.h"
#include "include/c/intern/registration.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"

#include <cerrno>
#include <gtest/gtest.h>

import std;
import cc_ice_intern_sonic;
import aten_xpu;
import aten_xpu_test;

namespace aten_xpu {

class StringTest : public RegistryFixture
{};

class StatusTest : public RegistryFixture
{};

class BufferTest : public RegistryFixture
{};

class RegistrationTest : public RegistryFixture
{};

// A created string that is destroyed with the scope.
class ScopedText
{
public:
    explicit ScopedText(std::string_view value = {}) :
        m_text{&SyclPluginRegistry::getInstance().getStringOps()}
    {
        m_text.create();
        m_text.copy(value);
    }

    ~ScopedText()
    {
        m_text.destroy();
    }

    ScopedText(const ScopedText&) = delete;
    ScopedText& operator=(const ScopedText&) = delete;

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

TEST_F(StringTest, CopyOwnsTheBytes)
{
    ScopedText text{"hello"};
    std::size_t size = 0;
    text.get().get_size(size);

    EXPECT_EQ(size, 5u);
    EXPECT_EQ(text.value(), "hello");

    ice::sonic::TStringType type;
    text.get().get_type(type);
    EXPECT_EQ(type.get_value(), TF_TSTR_SMALL);
}

TEST_F(StringTest, LargeStringsReportLargeType)
{
    ScopedText text{std::string(64, 'x')};
    ice::sonic::TStringType type;
    text.get().get_type(type);

    EXPECT_EQ(type.get_value(), TF_TSTR_LARGE);
}

TEST_F(StringTest, ViewDoesNotCopy)
{
    const std::string source = "viewed";
    ScopedText text;
    text.get().assign_view(source);
    const char* data = nullptr;
    text.get().get_data_pointer(&data);
    ice::sonic::TStringType type;
    text.get().get_type(type);

    EXPECT_EQ(data, source.data());
    EXPECT_EQ(type.get_value(), TF_TSTR_VIEW);

    text.get().init();
    EXPECT_EQ(text.value(), "");
}

TEST_F(StatusTest, StoresCodeMessageAndPayloads)
{
    ice::sonic::Status status{&SyclPluginRegistry::getInstance().getStatusOps()};
    status.create();
    ScopedText message{"boom"};
    status.set_status(ice::sonic::Code::k_not_found, message.get());
    ScopedText key{"url"};
    ScopedText value{"file:///x"};
    status.set_payload(key.get(), value.get());

    ice::sonic::Code code;
    status.get_code(code);
    ScopedText out;
    status.message(out.get());
    std::vector<std::string> seen;
    status.for_each_payload(
        [](const char* key_data,
           std::size_t key_size,
           const char* value_data,
           std::size_t value_size,
           void* capture)
        {
            static_cast<std::vector<std::string>*>(capture)->push_back(
                std::string{key_data, key_size} + "=" + std::string{value_data, value_size}
            );
        },
        &seen
    );

    EXPECT_EQ(code.get_value(), TF_NOT_FOUND);
    EXPECT_EQ(out.value(), "boom");
    ASSERT_EQ(seen.size(), 1u);
    EXPECT_EQ(seen.front(), "url=file:///x");
    status.destroy();
}

TEST_F(StatusTest, MapsErrno)
{
    ice::sonic::Status status{&SyclPluginRegistry::getInstance().getStatusOps()};
    status.create();
    ScopedText context{"open"};
    status.set_status_from_io_error(ENOENT, context.get());
    ice::sonic::Code code;
    status.get_code(code);
    ScopedText out;
    status.message(out.get());

    EXPECT_EQ(code.get_value(), TF_NOT_FOUND);
    EXPECT_TRUE(out.value().starts_with("open: "));

    status.set_status_from_io_error(EACCES, context.get());
    status.get_code(code);
    EXPECT_EQ(code.get_value(), TF_PERMISSION_DENIED);
    status.delete_status();
}

TEST_F(BufferTest, CopiesBytesAndViewsThem)
{
    ice::sonic::Buffer buffer{&SyclPluginRegistry::getInstance().getBufferOps()};
    buffer.create();
    const std::array<std::byte, 3> bytes{std::byte{1}, std::byte{2}, std::byte{3}};
    buffer.assign_from_string(bytes.data(), bytes.size());
    ::TFBufferData view{};
    buffer.get_buffer(&view);

    ASSERT_EQ(view.length, 3u);
    EXPECT_NE(view.data, bytes.data());
    EXPECT_EQ(static_cast<const std::byte*>(view.data)[2], std::byte{3});
    EXPECT_TRUE(view.data_deallocator == nullptr);
    buffer.destroy();
}

TEST_F(RegistrationTest, RegistersLooksUpAndDefaults)
{
    ice::sonic::Registration registry{&SyclPluginRegistry::getInstance().getRegistrationOps()};
    registry.create();
    int first = 1;
    int second = 2;
    ScopedText type{"kernel"};
    ScopedText first_name{"alpha"};
    ScopedText second_name{"beta"};
    ScopedText empty;
    registry.register_op(type.get(), first_name.get(), &first);
    registry.register_op(type.get(), second_name.get(), &second);

    void* found = nullptr;
    registry.get(type.get(), second_name.get(), &found);
    EXPECT_EQ(found, &second);

    registry.get(type.get(), empty.get(), &found);
    EXPECT_EQ(found, &first);

    registry.set_default(type.get(), second_name.get());
    registry.get(type.get(), empty.get(), &found);
    EXPECT_EQ(found, &second);

    registry.unregister(type.get(), second_name.get());
    registry.get(type.get(), second_name.get(), &found);
    EXPECT_TRUE(found == nullptr);
    registry.get(type.get(), empty.get(), &found);
    EXPECT_EQ(found, &first);

    ScopedText unknown{"missing"};
    registry.get(unknown.get(), empty.get(), &found);
    EXPECT_TRUE(found == nullptr);
    registry.destroy();
}

} // namespace aten_xpu
