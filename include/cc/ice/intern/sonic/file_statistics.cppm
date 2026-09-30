// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/intern/file_statistics.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/intern/file_statistics.h"

export module cc_ice_intern_sonic:file_statistics;

import std;
import cc_abi_sonic_registration;

export namespace ice::sonic {

class TF_FileStatisticsOps : public ice::sonic::Runtime<TF_FileStatisticsOps, TF_FileStatisticsOps>
{
public:
    explicit TF_FileStatisticsOps(TF_FileStatisticsOps* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "intern";

    void get_name(const ice::sonic::String& out_name) noexcept
    {
        m_ops->get_name(get_handle(), out_name.get_handle());
    }

    void is_directory(int* out_is_directory) noexcept
    {
        m_ops->is_directory(get_handle(), out_is_directory);
    }

    void set_is_directory(int is_directory) noexcept
    {
        m_ops->set_is_directory(get_handle(), is_directory);
    }

    void length(int64_t* out_length) noexcept
    {
        m_ops->length(get_handle(), out_length);
    }

    void set_length(int64_t length) noexcept
    {
        m_ops->set_length(get_handle(), length);
    }

    void mtime_nsec(int64_t* out_mtime_nsec) noexcept
    {
        m_ops->mtime_nsec(get_handle(), out_mtime_nsec);
    }

    void set_mtime_nsec(int64_t mtime_nsec) noexcept
    {
        m_ops->set_mtime_nsec(get_handle(), mtime_nsec);
    }

    void destroy() noexcept
    {
        m_ops->destroy(get_handle());
    }
};

} // namespace ice::sonic
