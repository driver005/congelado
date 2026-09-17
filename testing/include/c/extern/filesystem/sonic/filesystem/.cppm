// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/filesystem/filesystem.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/filesystem/filesystem.h"

export module cc_abi_sonic_filesystem;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;
import cc_abi_sonic_registration;

export namespace ice::sonic {

class TF_FilesystemOps : public ice::sonic::Runtime<TF_FilesystemOps, TF_FilesystemOps>
{
public:
    explicit TF_FilesystemOps(TF_FilesystemOps* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "filesystem";

    [[nodiscard]] std::expected<void, ice::Status>
    free_options(TFFilesystemOption* options, int num_options) noexcept
    {
        ice::Status status;
        m_ops->free_options(get_handle(), options, num_options status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    create_dir(const ice::sonic::TF_StringOps& path) noexcept
    {
        ice::Status status;
        m_ops->create_dir(get_handle(), path.get_handle() status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    recursively_create_dir(const ice::sonic::TF_StringOps& path) noexcept
    {
        ice::Status status;
        m_ops->recursively_create_dir(get_handle(), path.get_handle() status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    delete_file(const ice::sonic::TF_StringOps& path) noexcept
    {
        ice::Status status;
        m_ops->delete_file(get_handle(), path.get_handle() status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    delete_dir(const ice::sonic::TF_StringOps& path) noexcept
    {
        ice::Status status;
        m_ops->delete_dir(get_handle(), path.get_handle() status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> delete_recursively(
        const ice::sonic::TF_StringOps& path,
        uint64_t* undeleted_files,
        uint64_t* undeleted_dirs
    ) noexcept
    {
        ice::Status status;
        m_ops->delete_recursively(
            get_handle(),
            path.get_handle(),
            undeleted_files,
            undeleted_dirs status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    rename_file(const ice::sonic::TF_StringOps& src, const ice::sonic::TF_StringOps& dst) noexcept
    {
        ice::Status status;
        m_ops->rename_file(get_handle(), src.get_handle(), dst.get_handle() status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    copy_file(const ice::sonic::TF_StringOps& src, const ice::sonic::TF_StringOps& dst) noexcept
    {
        ice::Status status;
        m_ops->copy_file(get_handle(), src.get_handle(), dst.get_handle() status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    path_exists(const ice::sonic::TF_StringOps& path) noexcept
    {
        ice::Status status;
        m_ops->path_exists(get_handle(), path.get_handle() status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    paths_exist(const ice::sonic::TF_StringOps& paths, int num_paths) noexcept
    {
        ice::Status status;
        m_ops->paths_exist(get_handle(), paths.get_handle(), num_paths status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> stat(
        const ice::sonic::TF_StringOps& path,
        const ice::sonic::TF_FileStatisticsOps& out_stats
    ) noexcept
    {
        ice::Status status;
        m_ops->stat(get_handle(), path.get_handle(), out_stats.get_handle() status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    is_directory(const ice::sonic::TF_StringOps& path, int* out_is_directory) noexcept
    {
        ice::Status status;
        m_ops->is_directory(get_handle(), path.get_handle(), out_is_directory status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    get_file_size(const ice::sonic::TF_StringOps& path, int64_t* out_size) noexcept
    {
        ice::Status status;
        m_ops->get_file_size(get_handle(), path.get_handle(), out_size status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> translate_name(
        const ice::sonic::TF_StringOps& uri,
        const ice::sonic::TF_StringOps& out_name
    ) noexcept
    {
        ice::Status status;
        m_ops->translate_name(
            get_handle(),
            uri.get_handle(),
            out_name.get_handle() status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    get_children(const ice::sonic::TF_StringOps& path, TF_Tensor** out_children) noexcept
    {
        ice::Status status;
        m_ops->get_children(get_handle(), path.get_handle(), out_children status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    get_matching_paths(const ice::sonic::TF_StringOps& glob, TF_Tensor** out_matches) noexcept
    {
        ice::Status status;
        m_ops->get_matching_paths(get_handle(), glob.get_handle(), out_matches status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> flush_caches() noexcept
    {
        ice::Status status;
        m_ops->flush_caches(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    get_filesystem_configuration(TF_Tensor** out_config) noexcept
    {
        ice::Status status;
        m_ops->get_filesystem_configuration(get_handle(), out_config status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    set_filesystem_configuration(const ice::sonic::TF_TensorOps& options) noexcept
    {
        ice::Status status;
        m_ops->set_filesystem_configuration(get_handle(), options.get_handle() status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> get_filesystem_configuration_option(
        const ice::sonic::TF_StringOps& key,
        TFFilesystemOption* out_option
    ) noexcept
    {
        ice::Status status;
        m_ops->get_filesystem_configuration_option(
            get_handle(),
            key.get_handle(),
            out_option status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    set_filesystem_configuration_option(const TFFilesystemOption* option) noexcept
    {
        ice::Status status;
        m_ops->set_filesystem_configuration_option(get_handle(), option status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    get_filesystem_configuration_keys(TF_Tensor** out_keys) noexcept
    {
        ice::Status status;
        m_ops->get_filesystem_configuration_keys(get_handle(), out_keys status.get_handle());

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
