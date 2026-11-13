module;

#include "include/c/intern/file_statistics.h"
#include "include/c/intern/tstring.h"

export module aten_xpu_intern:file_statistics;

import std;
import cc_ice_intern_builder;
import cc_ice_intern_sonic;

export namespace aten_xpu {

// The three stat fields the filesystem reports.
class FileStatistics : public ice::builder::FileStatistics
{
public:
    explicit FileStatistics(const ::TF_StringOps* string_ops) noexcept :
        ice::builder::FileStatistics{string_ops}
    {
    }

    ~FileStatistics() override = default;
    FileStatistics(const FileStatistics&) = delete;
    FileStatistics& operator=(const FileStatistics&) = delete;
    FileStatistics(FileStatistics&&) = delete;
    FileStatistics& operator=(FileStatistics&&) = delete;

    void destroy() noexcept override
    {
        delete this;
    }

    void get_name(const ice::sonic::String& out_name) noexcept override
    {
        out_name.copy("FileStatistics");
    }

    void is_directory(int& out_is_directory) noexcept override
    {
        out_is_directory = m_is_directory;
    }

    void set_is_directory(int is_directory) noexcept override
    {
        m_is_directory = is_directory;
    }

    void length(std::int64_t& out_length) noexcept override
    {
        out_length = m_length;
    }

    void set_length(std::int64_t length) noexcept override
    {
        m_length = length;
    }

    void mtime_nsec(std::int64_t& out_mtime_nsec) noexcept override
    {
        out_mtime_nsec = m_mtime_nsec;
    }

    void set_mtime_nsec(std::int64_t mtime_nsec) noexcept override
    {
        m_mtime_nsec = mtime_nsec;
    }

private:
    int m_is_directory = 0;
    std::int64_t m_length = 0;
    std::int64_t m_mtime_nsec = 0;
};

} // namespace aten_xpu
