// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/intern/file_statistics.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/intern/file_statistics.h"
#include "include/c/intern/registration.h"

export module cc_ice_intern_sonic:file_statistics;

import std;
import :registration;
import :runtime;
import :tstring;

export namespace ice::sonic {

class FileStatistics : public ice::sonic::Runtime<::TF_FileStatisticsOps, ::TF_FileStatistics>
{
public:
    FileStatistics(
        const ice::sonic::Registration& registry,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(
            *registry.get_ops(),
            *registry.get_handle(),
            *type.get_handle(),
            *provider.get_handle()
        )
    {
    }

    FileStatistics(
        const ice::sonic::Registration& registry,
        const ::TF_FileStatistics& handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(
            *registry.get_ops(),
            *registry.get_handle(),
            handle,
            *type.get_handle(),
            *provider.get_handle()
        )
    {
    }

    explicit FileStatistics(const ::TF_FileStatisticsOps* ops) noexcept :
        Runtime(ops)
    {
    }

    FileStatistics(const ::TF_FileStatisticsOps* ops, const ::TF_FileStatistics* handle) noexcept :
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

    void is_directory(int& out_is_directory) const noexcept
    {
        m_ops->is_directory(get_handle(), &out_is_directory);
    }

    void set_is_directory(int is_directory) const noexcept
    {
        m_ops->set_is_directory(get_handle(), is_directory);
    }

    void length(std::int64_t& out_length) const noexcept
    {
        m_ops->length(get_handle(), &out_length);
    }

    void set_length(int64_t length) const noexcept
    {
        m_ops->set_length(get_handle(), length);
    }

    void mtime_nsec(std::int64_t& out_mtime_nsec) const noexcept
    {
        m_ops->mtime_nsec(get_handle(), &out_mtime_nsec);
    }

    void set_mtime_nsec(int64_t mtime_nsec) const noexcept
    {
        m_ops->set_mtime_nsec(get_handle(), mtime_nsec);
    }
};

} // namespace ice::sonic
