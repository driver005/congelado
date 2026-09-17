// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/intern/file_statistics.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/intern/file_statistics.h"

export module cc_abi_sonic_intern;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;
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

    [[nodiscard]] std::expected<void, ice::Status> is_directory(int* out_is_directory) noexcept
    {
        ice::Status status;
        m_ops->is_directory(get_handle(), out_is_directory status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> set_is_directory(int is_directory) noexcept
    {
        ice::Status status;
        m_ops->set_is_directory(get_handle(), is_directory status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> length(int64_t* out_length) noexcept
    {
        ice::Status status;
        m_ops->length(get_handle(), out_length status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> set_length(int64_t length) noexcept
    {
        ice::Status status;
        m_ops->set_length(get_handle(), length status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> mtime_nsec(int64_t* out_mtime_nsec) noexcept
    {
        ice::Status status;
        m_ops->mtime_nsec(get_handle(), out_mtime_nsec status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> set_mtime_nsec(int64_t mtime_nsec) noexcept
    {
        ice::Status status;
        m_ops->set_mtime_nsec(get_handle(), mtime_nsec status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    virtual ice::String get_name() const noexcept = 0;

    ice::String get_name() const noexcept
    {
        ice::String result;
        m_ops->get_name(get_handle(), result.get_handle());
        return result;
    }
};

} // namespace ice::sonic
