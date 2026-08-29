module;

#include "c/intern/tf_file_statistics.h"

export module cc_abi_builder_intern:file_statistics;

import std;

export namespace ice::builder {

class FileStatistics
{
public:
    FileStatistics()
    {
        m_stats = new TF_FileStatistics;
        TF_FileStatisticsInit(m_stats);
    }

    FileStatistics(int64_t length, int64_t mtime_nsec, bool is_directory)
    {
        m_stats = new TF_FileStatistics;
        TF_FileStatisticsInit(m_stats);
        m_stats->length = length;
        m_stats->mtime_nsec = mtime_nsec;
        m_stats->is_directory = is_directory ? 1 : 0;
    }
    
    FileStatistics(const FileStatistics& other) {
        m_stats = new TF_FileStatistics;
        *m_stats = *other.m_stats;
    }
    
    FileStatistics& operator=(const FileStatistics& other) {
        if (this != &other) {
            *m_stats = *other.m_stats;
        }
        return *this;
    }
    
    FileStatistics(FileStatistics&& other) noexcept : m_stats(other.m_stats) {
        other.m_stats = nullptr;
    }
    
    FileStatistics& operator=(FileStatistics&& other) noexcept {
        if (this != &other) {
            delete m_stats;
            m_stats = other.m_stats;
            other.m_stats = nullptr;
        }
        return *this;
    }
    
    ~FileStatistics() {
        delete m_stats;
    }

    int64_t get_length() const
    {
        return m_stats ? m_stats->length : 0;
    }

    void set_length(int64_t length)
    {
        if (m_stats) m_stats->length = length;
    }

    int64_t get_mtime_nsec() const
    {
        return m_stats ? m_stats->mtime_nsec : 0;
    }

    void set_mtime_nsec(int64_t t)
    {
        if (m_stats) m_stats->mtime_nsec = t;
    }

    bool get_is_directory() const
    {
        return m_stats ? (m_stats->is_directory != 0) : false;
    }

    void set_is_directory(bool d)
    {
        if (m_stats) m_stats->is_directory = d ? 1 : 0;
    }

    static FileStatistics create(const TF_FileStatistics* s)
    {
        if (!s) {
            return FileStatistics();
        }
        return FileStatistics(s->length, s->mtime_nsec, s->is_directory != 0);
    }

    void to_c(TF_FileStatistics* s) const
    {
        if (s && m_stats) {
            s->struct_size = m_stats->struct_size;
            s->length = m_stats->length;
            s->mtime_nsec = m_stats->mtime_nsec;
            s->is_directory = m_stats->is_directory;
        }
    }

    // Underlying handle — pass directly to the C ABI
    TF_FileStatistics* get_handle()
    {
        return m_stats;
    }

    const TF_FileStatistics* get_handle() const
    {
        return m_stats;
    }

private:
    TF_FileStatistics* m_stats;
};

} // namespace ice::builder
