module;

#include "c/intern/tf_file_statistics.h"

export module cc_abi_builder_intern:file_statistics;

import std;

export namespace ice::builder {

// FileStatistics — RAII owner of a TF_FileStatistics value (unique_ptr + delete), same
// modernization as ice::sonic::FileStatistics. Every member is noexcept: allocation uses
// a guarded nothrow factory (a failed allocation yields an empty/default stats object,
// never an exception crossing a noexcept boundary).
class FileStatistics
{
public:
    FileStatistics() noexcept
    {
    }

    FileStatistics(int64_t length, int64_t mtime_nsec, bool is_directory) noexcept :
        FileStatistics()
    {
        if (m_stats) {
            m_stats->length = length;
            m_stats->mtime_nsec = mtime_nsec;
            m_stats->is_directory = is_directory ? 1 : 0;
        }
    }

    FileStatistics(const FileStatistics& other) noexcept :
        m_stats{new_tf_file_statistics()}
    {
        if (m_stats && other.m_stats) {
            *m_stats = *other.m_stats;
        }
    }

    FileStatistics& operator=(const FileStatistics& other) noexcept
    {
        if (this != &other && m_stats && other.m_stats) {
            *m_stats = *other.m_stats;
        }
        return *this;
    }

    FileStatistics(FileStatistics&& other) noexcept = default;
    FileStatistics& operator=(FileStatistics&& other) noexcept = default;

    ~FileStatistics() noexcept = default;

    int64_t get_length() const noexcept
    {
        return m_stats ? m_stats->length : 0;
    }

    void set_length(int64_t length) noexcept
    {
        if (m_stats) {
            m_stats->length = length;
        }
    }

    int64_t get_mtime_nsec() const noexcept
    {
        return m_stats ? m_stats->mtime_nsec : 0;
    }

    void set_mtime_nsec(int64_t t) noexcept
    {
        if (m_stats) {
            m_stats->mtime_nsec = t;
        }
    }

    bool get_is_directory() const noexcept
    {
        return m_stats ? (m_stats->is_directory != 0) : false;
    }

    void set_is_directory(bool d) noexcept
    {
        if (m_stats) {
            m_stats->is_directory = d ? 1 : 0;
        }
    }

    static FileStatistics create(const TF_FileStatistics* s) noexcept
    {
        if (!s) {
            return FileStatistics();
        }
        return FileStatistics(s->length, s->mtime_nsec, s->is_directory != 0);
    }

    void to_c(TF_FileStatistics* s) const noexcept
    {
        if (s && m_stats) {
            s->struct_size = m_stats->struct_size;
            s->length = m_stats->length;
            s->mtime_nsec = m_stats->mtime_nsec;
            s->is_directory = m_stats->is_directory;
        }
    }

    // Underlying handle — pass directly to the C ABI.
    TF_FileStatistics* get_handle() noexcept
    {
        return m_stats.get();
    }

    const TF_FileStatistics* get_handle() const noexcept
    {
        return m_stats.get();
    }

private:
    static TF_FileStatistics* new_tf_file_statistics() noexcept
    {
        try {
            auto* s = new TF_FileStatistics;
            TF_FileStatisticsInit(s);
            return s;
        } catch (...) {
            return nullptr;
        }
    }

    std::unique_ptr<TF_FileStatistics> m_stats{new_tf_file_statistics()};
};

} // namespace ice::builder
