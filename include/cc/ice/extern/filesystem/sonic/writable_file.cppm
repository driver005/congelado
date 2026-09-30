// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/filesystem/writable_file.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/filesystem/writable_file.h"

export module cc_ice_extern_filesystem_sonic:writable_file;

import std;
import cc_abi_sonic_registration;

export namespace ice::sonic {

class TF_WritableFileOps : public ice::sonic::Runtime<TF_WritableFileOps, TF_WritableFileOps>
{
public:
    explicit TF_WritableFileOps(TF_WritableFileOps* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "filesystem";

    void destroy() noexcept
    {
        m_ops->destroy(get_handle());
    }

    void get_name(const ice::sonic::String& out_name) noexcept
    {
        m_ops->get_name(get_handle(), out_name.get_handle());
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    append(const ice::sonic::String& buffer) noexcept
    {
        ice::sonic::Status status;
        m_ops->append(get_handle(), buffer.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status> tell(int64_t* out_position) noexcept
    {
        ice::sonic::Status status;
        m_ops->tell(get_handle(), out_position, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status> flush() noexcept
    {
        ice::sonic::Status status;
        m_ops->flush(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status> sync() noexcept
    {
        ice::sonic::Status status;
        m_ops->sync(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status> close() noexcept
    {
        ice::sonic::Status status;
        m_ops->close(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }
};

} // namespace ice::sonic
