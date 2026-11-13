#include "include/c/extern/filesystem/option_types.h"
#include "include/c/extern/filesystem/tree.h"
#include "include/c/intern/file_statistics.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tensor.h"
#include "include/c/intern/tstring.h"

#include <cstdlib>
#include <gtest/gtest.h>

import std;
import cc_ice_intern_sonic;
import cc_ice_extern_filesystem_sonic;
import aten_xpu;
import aten_xpu_test;

namespace aten_xpu {

class FilesystemTest : public RegistryFixture
{};

class DiskText
{
public:
    explicit DiskText(std::string_view value = {}) :
        m_text{&SyclPluginRegistry::getInstance().getStringOps()}
    {
        m_text.create();
        m_text.copy(value);
    }

    ~DiskText()
    {
        m_text.destroy();
    }

    DiskText(const DiskText&) = delete;
    DiskText& operator=(const DiskText&) = delete;

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

class DiskStatus
{
public:
    DiskStatus() :
        m_status{&SyclPluginRegistry::getInstance().getStatusOps()}
    {
        m_status.create();
    }

    ~DiskStatus()
    {
        m_status.destroy();
    }

    DiskStatus(const DiskStatus&) = delete;
    DiskStatus& operator=(const DiskStatus&) = delete;

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

    // Clears a previous failure by writing an ok status.
    void reset() const
    {
        DiskText empty;
        m_status.set_status(ice::sonic::Code::k_ok, empty.get());
    }

private:
    ice::sonic::Status m_status;
};

// A scratch directory removed with the scope.
class DiskSandbox
{
public:
    DiskSandbox()
    {
        const char* base = std::getenv("TEST_TMPDIR");
        m_root = (base == nullptr ? std::filesystem::temp_directory_path()
                                  : std::filesystem::path{base}) /
                 "filesystem_test";
        std::filesystem::remove_all(m_root);
        std::filesystem::create_directories(m_root);
    }

    ~DiskSandbox()
    {
        std::filesystem::remove_all(m_root);
    }

    DiskSandbox(const DiskSandbox&) = delete;
    DiskSandbox& operator=(const DiskSandbox&) = delete;

    std::string path(std::string_view name) const
    {
        return (m_root / name).string();
    }

private:
    std::filesystem::path m_root;
};

// Reads a returned string tensor and frees it.
class DiskLines
{
public:
    static std::vector<std::string> take(::TF_Tensor* tensor)
    {
        ice::sonic::Tensor wrapper{
            SyclPluginRegistry::getInstance().getOps().getTensorOps(),
            tensor
        };
        auto lines = TextTensor::lines_of(wrapper);
        wrapper.delete_tensor();

        return lines;
    }
};

TEST_F(FilesystemTest, FilesRoundTrip)
{
    DiskSandbox sandbox;
    DiskStatus status;
    const std::string path = sandbox.path("data.txt");

    WritableFile writer{
        &SyclPluginRegistry::getInstance().getStatusOps(),
        &SyclPluginRegistry::getInstance().getStringOps(),
        path,
        true
    };
    DiskText text{"hello world"};
    writer.append(text.get(), status.get());
    std::int64_t position = 0;
    writer.tell(position, status.get());
    writer.sync(status.get());
    writer.close(status.get());

    EXPECT_EQ(status.code(), TF_OK);
    EXPECT_EQ(position, 11);

    RandomAccessFile reader{
        &SyclPluginRegistry::getInstance().getStatusOps(),
        &SyclPluginRegistry::getInstance().getStringOps(),
        path
    };
    std::array<char, 5> buffer{};
    std::int64_t read = 0;
    reader.read(6, buffer.size(), buffer.data(), read, status.get());
    EXPECT_EQ(read, 5);
    EXPECT_EQ(std::string(buffer.data(), buffer.size()), "world");
    EXPECT_EQ(status.code(), TF_OK);

    reader.read(8, 10, buffer.data(), read, status.get());
    EXPECT_EQ(read, 3);
    EXPECT_EQ(status.code(), TF_OUT_OF_RANGE);

    ReadOnlyMemoryRegion region{&SyclPluginRegistry::getInstance().getStringOps(), path};
    const void* mapped = nullptr;
    std::uint64_t length = 0;
    region.data(&mapped);
    region.length(length);
    EXPECT_EQ(region.getOpenError(), 0);
    EXPECT_EQ(length, 11u);
    EXPECT_EQ(std::string(static_cast<const char*>(mapped), 5), "hello");
}

TEST_F(FilesystemTest, MissingFilesReportErrno)
{
    DiskSandbox sandbox;
    DiskStatus status;
    RandomAccessFile reader{
        &SyclPluginRegistry::getInstance().getStatusOps(),
        &SyclPluginRegistry::getInstance().getStringOps(),
        sandbox.path("absent")
    };
    std::array<char, 4> buffer{};
    std::int64_t read = -1;
    reader.read(0, buffer.size(), buffer.data(), read, status.get());

    EXPECT_EQ(status.code(), TF_NOT_FOUND);
    EXPECT_EQ(read, 0);
    EXPECT_EQ(reader.getOpenError(), ENOENT);
}

TEST_F(FilesystemTest, TreeManagesDirectoriesAndFiles)
{
    DiskSandbox sandbox;
    DiskStatus status;
    ice::sonic::FilesystemTree tree{&SyclPluginRegistry::getInstance().getFilesystemTreeOps()};
    tree.create();

    DiskText nested{sandbox.path("a/b/c")};
    tree.recursively_create_dir(nested.get(), status.get());
    EXPECT_EQ(status.code(), TF_OK);

    DiskText top{sandbox.path("a")};
    tree.create_dir(top.get(), status.get());
    EXPECT_EQ(status.code(), TF_ALREADY_EXISTS);
    status.reset();

    const std::string file = sandbox.path("a/one.txt");
    std::ofstream{file} << "12345";
    DiskText file_text{file};
    int is_directory = 1;
    std::int64_t size = 0;
    tree.is_directory(file_text.get(), is_directory, status.get());
    tree.get_file_size(file_text.get(), size, status.get());
    EXPECT_EQ(is_directory, 0);
    EXPECT_EQ(size, 5);

    ice::sonic::FileStatistics stats{&SyclPluginRegistry::getInstance().getFileStatisticsOps()};
    stats.create();
    tree.stat(file_text.get(), stats, status.get());
    std::int64_t stat_length = 0;
    std::int64_t mtime = 0;
    stats.length(stat_length);
    stats.mtime_nsec(mtime);
    EXPECT_EQ(stat_length, 5);
    EXPECT_LT(0, mtime);
    stats.destroy();

    DiskText copy{sandbox.path("a/two.txt")};
    tree.copy_file(file_text.get(), copy.get(), status.get());
    DiskText moved{sandbox.path("a/three.txt")};
    tree.rename_file(copy.get(), moved.get(), status.get());
    ::TF_Tensor* children = nullptr;
    tree.get_children(top.get(), &children, status.get());
    ASSERT_TRUE(children != nullptr);
    const auto names = DiskLines::take(children);
    ASSERT_EQ(names.size(), 3u);
    EXPECT_EQ(names[0], "b");
    EXPECT_EQ(names[1], "one.txt");
    EXPECT_EQ(names[2], "three.txt");

    DiskText pattern{sandbox.path("a/*.txt")};
    ::TF_Tensor* matches = nullptr;
    tree.get_matching_paths(pattern.get(), &matches, status.get());
    ASSERT_TRUE(matches != nullptr);
    EXPECT_EQ(DiskLines::take(matches).size(), 2u);

    tree.delete_file(file_text.get(), status.get());
    tree.path_exists(file_text.get(), status.get());
    EXPECT_EQ(status.code(), TF_NOT_FOUND);
    status.reset();

    tree.delete_dir(top.get(), status.get());
    EXPECT_NE(status.code(), TF_OK);
    status.reset();

    std::uint64_t undeleted_files = 7;
    std::uint64_t undeleted_dirs = 7;
    tree.delete_recursively(top.get(), &undeleted_files, &undeleted_dirs, status.get());
    EXPECT_EQ(status.code(), TF_OK);
    EXPECT_EQ(undeleted_files, 0u);
    EXPECT_EQ(undeleted_dirs, 0u);
    tree.destroy();
}

TEST_F(FilesystemTest, TranslatesFileUris)
{
    ice::sonic::FilesystemTree tree{&SyclPluginRegistry::getInstance().getFilesystemTreeOps()};
    tree.create();
    DiskText uri{"file:///tmp/x"};
    DiskText name;
    tree.translate_name(uri.get(), name.get());

    EXPECT_EQ(name.value(), "/tmp/x");
    tree.destroy();
}

TEST_F(FilesystemTest, ConfigurationOptionsRoundTrip)
{
    DiskStatus status;
    ice::sonic::FilesystemTree tree{&SyclPluginRegistry::getInstance().getFilesystemTreeOps()};
    tree.create();

    ice::sonic::FilesystemOption option;
    option.setTypeTag(TF_FILESYSTEM_OPTION_TYPE_INT);
    option.setName("block_size");
    option.getValue().type = TF_FILESYSTEM_OPTION_TYPE_INT;
    option.getValue().int_value = 4'096;
    tree.set_filesystem_configuration_option(&option, status.get());
    EXPECT_EQ(status.code(), TF_OK);

    DiskText key{"block_size"};
    ice::sonic::FilesystemOption found;
    tree.get_filesystem_configuration_option(key.get(), &found, status.get());
    EXPECT_EQ(found.getValue().int_value, 4'096);
    EXPECT_STREQ(found.getName(), "block_size");

    DiskText missing{"nope"};
    tree.get_filesystem_configuration_option(missing.get(), &found, status.get());
    EXPECT_EQ(status.code(), TF_NOT_FOUND);
    status.reset();

    ::TF_Tensor* keys = nullptr;
    tree.get_filesystem_configuration_keys(&keys, status.get());
    ASSERT_TRUE(keys != nullptr);
    const auto names = DiskLines::take(keys);
    ASSERT_EQ(names.size(), 1u);
    EXPECT_EQ(names[0], "block_size");

    ::TF_Tensor* config = nullptr;
    tree.get_filesystem_configuration(&config, status.get());
    ASSERT_TRUE(config != nullptr);
    const auto lines = DiskLines::take(config);
    ASSERT_EQ(lines.size(), 1u);
    EXPECT_EQ(lines[0], "block_size=4096");
    tree.destroy();
}

} // namespace aten_xpu
