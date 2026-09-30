// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/intern/file_statistics.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/intern/file_statistics.h"

export module cc_ice_intern_builder:file_statistics;

import std;

export namespace ice::builder {

class TF_FileStatisticsOps
{
public:
    TF_FileStatisticsOps() noexcept :
        m_handle{.plugin_data = this}
    {
    }

    TF_FileStatisticsOps(const TF_FileStatisticsOps&) = delete;
    TF_FileStatisticsOps& operator=(const TF_FileStatisticsOps&) = delete;

    static TF_FileStatisticsOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TF_FileStatisticsOps*>(ctx);
    }

    template<typename HandleT>
    static TF_FileStatisticsOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TF_FileStatisticsOps*>(handle->plugin_data);
    }

    virtual ~TF_FileStatisticsOps() = default;
    virtual void get_name(const ice::sonic::String& out_name) noexcept = 0;
    virtual void is_directory(int* out_is_directory) noexcept = 0;
    virtual void set_is_directory(int is_directory) noexcept = 0;
    virtual void length(int64_t* out_length) noexcept = 0;
    virtual void set_length(int64_t length) noexcept = 0;
    virtual void mtime_nsec(int64_t* out_mtime_nsec) noexcept = 0;
    virtual void set_mtime_nsec(int64_t mtime_nsec) noexcept = 0;
    virtual void destroy() noexcept = 0;

    void get_generic_vtable() noexcept
    {
        m_vtable = ::TF_FileStatisticsOps{
            .struct_size = TF_FILESTATISTICS_STRUCT_SIZE,
            .get_name =
                [](TF_FileStatistics* stats, TF_String* out_name) noexcept
            {
                TF_FileStatisticsOps::from_handle(stats).get_name(
                    ice::sonic::String::wrap(out_name)
                );
            },
            .is_directory =
                [](const TF_FileStatistics* stats, int* out_is_directory) noexcept
            {
                TF_FileStatisticsOps::from_handle(stats).is_directory(out_is_directory);
            },
            .set_is_directory =
                [](TF_FileStatistics* stats, int is_directory) noexcept
            {
                TF_FileStatisticsOps::from_handle(stats).set_is_directory(is_directory);
            },
            .length =
                [](const TF_FileStatistics* stats, int64_t* out_length) noexcept
            {
                TF_FileStatisticsOps::from_handle(stats).length(out_length);
            },
            .set_length =
                [](TF_FileStatistics* stats, int64_t length) noexcept
            {
                TF_FileStatisticsOps::from_handle(stats).set_length(length);
            },
            .mtime_nsec =
                [](const TF_FileStatistics* stats, int64_t* out_mtime_nsec) noexcept
            {
                TF_FileStatisticsOps::from_handle(stats).mtime_nsec(out_mtime_nsec);
            },
            .set_mtime_nsec =
                [](TF_FileStatistics* stats, int64_t mtime_nsec) noexcept
            {
                TF_FileStatisticsOps::from_handle(stats).set_mtime_nsec(mtime_nsec);
            },
            .destroy =
                [](TF_FileStatistics* stats) noexcept
            {
                TF_FileStatisticsOps::from_handle(stats).destroy();
            },

        };
    }

    const ::TF_FileStatisticsOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const TF_FileStatistics& get_handle() const noexcept
    {
        return m_handle;
    }


private:
    ::TF_FileStatisticsOps m_vtable;
    TF_FileStatistics m_handle;
};

} // namespace ice::builder
