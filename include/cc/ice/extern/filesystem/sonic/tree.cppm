// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/filesystem/tree.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/filesystem/tree.h"

export module cc_ice_extern_filesystem_sonic:tree;

import std;
import cc_ice_intern_sonic;

export namespace ice::sonic {

class TFFilesystemTreeOps : public ice::sonic::Runtime<::TFFilesystemTreeOps, ::TFFilesystemTree>
{
public:
    template<typename Registry>
    TFFilesystemTreeOps(
        Registry& registry,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry, type, provider)
    {
    }

    template<typename Registry>
    TFFilesystemTreeOps(
        Registry& registry,
        ::TFFilesystemTree* handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry, handle, type, provider)
    {
    }

    explicit TFFilesystemTreeOps(const ::TFFilesystemTreeOps* ops) noexcept :
        Runtime(ops)
    {
    }

    TFFilesystemTreeOps(const ::TFFilesystemTreeOps* ops, ::TFFilesystemTree* handle) noexcept :
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

    void free_options(TFFilesystemOption* options, int num_options) const noexcept
    {
        m_ops->free_options(get_handle(), options, num_options);
    }

    void create_dir(
        const ice::sonic::String& path,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->create_dir(get_handle(), path.get_handle(), out_status.get_handle());
    }

    void recursively_create_dir(
        const ice::sonic::String& path,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->recursively_create_dir(get_handle(), path.get_handle(), out_status.get_handle());
    }

    void delete_file(
        const ice::sonic::String& path,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->delete_file(get_handle(), path.get_handle(), out_status.get_handle());
    }

    void delete_dir(
        const ice::sonic::String& path,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->delete_dir(get_handle(), path.get_handle(), out_status.get_handle());
    }

    void delete_recursively(
        const ice::sonic::String& path,
        uint64_t* undeleted_files,
        uint64_t* undeleted_dirs,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->delete_recursively(
            get_handle(),
            path.get_handle(),
            undeleted_files,
            undeleted_dirs,
            out_status.get_handle()
        );
    }

    void rename_file(
        const ice::sonic::String& src,
        const ice::sonic::String& dst,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->rename_file(
            get_handle(),
            src.get_handle(),
            dst.get_handle(),
            out_status.get_handle()
        );
    }

    void copy_file(
        const ice::sonic::String& src,
        const ice::sonic::String& dst,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->copy_file(get_handle(), src.get_handle(), dst.get_handle(), out_status.get_handle());
    }

    void path_exists(
        const ice::sonic::String& path,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->path_exists(get_handle(), path.get_handle(), out_status.get_handle());
    }

    void paths_exist(
        const ice::sonic::String& paths,
        int num_paths,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->paths_exist(get_handle(), paths.get_handle(), num_paths, out_status.get_handle());
    }

    void stat(
        const ice::sonic::String& path,
        const ice::sonic::TF_FileStatisticsOps& out_stats,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->stat(
            get_handle(),
            path.get_handle(),
            out_stats.get_handle(),
            out_status.get_handle()
        );
    }

    void is_directory(
        const ice::sonic::String& path,
        int* out_is_directory,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->is_directory(
            get_handle(),
            path.get_handle(),
            out_is_directory,
            out_status.get_handle()
        );
    }

    void get_file_size(
        const ice::sonic::String& path,
        int64_t* out_size,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->get_file_size(get_handle(), path.get_handle(), out_size, out_status.get_handle());
    }

    void translate_name(
        const ice::sonic::String& uri,
        const ice::sonic::String& out_name
    ) const noexcept
    {
        m_ops->translate_name(get_handle(), uri.get_handle(), out_name.get_handle());
    }

    void get_children(
        const ice::sonic::String& path,
        TF_Tensor** out_children,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->get_children(get_handle(), path.get_handle(), out_children, out_status.get_handle());
    }

    void get_matching_paths(
        const ice::sonic::String& glob,
        TF_Tensor** out_matches,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->get_matching_paths(
            get_handle(),
            glob.get_handle(),
            out_matches,
            out_status.get_handle()
        );
    }

    void flush_caches() const noexcept
    {
        m_ops->flush_caches(get_handle());
    }

    void get_filesystem_configuration(
        TF_Tensor** out_config,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->get_filesystem_configuration(get_handle(), out_config, out_status.get_handle());
    }

    void set_filesystem_configuration(
        const ice::sonic::TF_TensorOps& options,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->set_filesystem_configuration(
            get_handle(),
            options.get_handle(),
            out_status.get_handle()
        );
    }

    void get_filesystem_configuration_option(
        const ice::sonic::String& key,
        TFFilesystemOption* out_option,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->get_filesystem_configuration_option(
            get_handle(),
            key.get_handle(),
            out_option,
            out_status.get_handle()
        );
    }

    void set_filesystem_configuration_option(
        const TFFilesystemOption* option,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->set_filesystem_configuration_option(get_handle(), option, out_status.get_handle());
    }

    void get_filesystem_configuration_keys(
        TF_Tensor** out_keys,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->get_filesystem_configuration_keys(get_handle(), out_keys, out_status.get_handle());
    }
};

} // namespace ice::sonic
