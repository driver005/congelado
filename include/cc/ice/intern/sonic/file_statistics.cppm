// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/intern/file_statistics.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/intern/file_statistics.h"

export module cc_ice_intern_sonic:file_statistics;

import std;
import :runtime;
import :tstring;

export namespace ice::sonic {

class TF_FileStatisticsOps : public ice::sonic::Runtime<::TF_FileStatisticsOps, ::TF_FileStatistics>
{
public:
    template<typename Registry>
    TF_FileStatisticsOps(
        Registry& registry,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry, type, provider)
    {
    }

    template<typename Registry>
    TF_FileStatisticsOps(
        Registry& registry,
        ::TF_FileStatistics* handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry, handle, type, provider)
    {
    }

    explicit TF_FileStatisticsOps(const ::TF_FileStatisticsOps* ops) noexcept :
        Runtime(ops)
    {
    }

    TF_FileStatisticsOps(const ::TF_FileStatisticsOps* ops, ::TF_FileStatistics* handle) noexcept :
        Runtime(ops, handle)
    {
    }

    void create() const noexcept
    {
        m_ops->create(get_handle());
    }

    void destroy() const noexcept
    {
        m_ops->destroy(get_handle());
    }

    void get_name(const ice::sonic::String& out_name) const noexcept
    {
        m_ops->get_name(get_handle(), out_name.get_handle());
    }

    void is_directory(int* out_is_directory) const noexcept
    {
        m_ops->is_directory(get_handle(), out_is_directory);
    }

    void set_is_directory(int is_directory) const noexcept
    {
        m_ops->set_is_directory(get_handle(), is_directory);
    }

    void length(int64_t* out_length) const noexcept
    {
        m_ops->length(get_handle(), out_length);
    }

    void set_length(int64_t length) const noexcept
    {
        m_ops->set_length(get_handle(), length);
    }

    void mtime_nsec(int64_t* out_mtime_nsec) const noexcept
    {
        m_ops->mtime_nsec(get_handle(), out_mtime_nsec);
    }

    void set_mtime_nsec(int64_t mtime_nsec) const noexcept
    {
        m_ops->set_mtime_nsec(get_handle(), mtime_nsec);
    }
};

} // namespace ice::sonic
