module;

#include "c/intern/tf_file_statistics.h"

export module cc_abi_value:file_statistics;

import std;

export namespace ice {

class FileStatistics
{
public:
    FileStatistics()
    {

        TF_FileStatisticsInit(&m_stats);
    }

    FileStatistics(int64_t length, int64_t mtime_nsec, bool is_directory)
    {

        TF_FileStatisticsInit(&m_stats);
        m_stats.length = length;
        m_stats.mtime_nsec = mtime_nsec;
        m_stats.is_directory = is_directory ? 1 : 0;
    }

    int64_t get_length() const
    {
        return m_stats.length;
    }

    void set_length(int64_t length)
    {
        m_stats.length = length;
    }

    int64_t get_mtime_nsec() const
    {
        return m_stats.mtime_nsec;
    }

    void set_mtime_nsec(int64_t t)
    {
        m_stats.mtime_nsec = t;
    }

    bool get_is_directory() const
    {
        return m_stats.is_directory != 0;
    }

    void set_is_directory(bool d)
    {
        m_stats.is_directory = d ? 1 : 0;
    }

    static FileStatistics from_c(const TF_FileStatistics* s)
    {

        if (!s) {
            return FileStatistics();
        }
        return FileStatistics(s->length, s->mtime_nsec, s->is_directory != 0);
    }

    void to_c(TF_FileStatistics* s) const
    {

        if (s) {
            s->struct_size = m_stats.struct_size;
            s->length = m_stats.length;
            s->mtime_nsec = m_stats.mtime_nsec;
            s->is_directory = m_stats.is_directory;
        }
    }

    // Underlying handle — pass directly to the C ABI
    TF_FileStatistics* get_handle()
    {
        return &m_stats;
    }

    const TF_FileStatistics* get_handle() const
    {
        return &m_stats;
    }

private:
    TF_FileStatistics m_stats;
};

} // namespace ice
