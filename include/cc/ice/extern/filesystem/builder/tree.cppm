// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/filesystem/tree.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/filesystem/tree.h"

export module cc_ice_extern_filesystem_builder:tree;

import std;

export namespace ice::builder {

class TFFilesystemTreeOps
{
public:
    TFFilesystemTreeOps() noexcept :
        m_handle{.plugin_data = this}
    {
    }

    TFFilesystemTreeOps(const TFFilesystemTreeOps&) = delete;
    TFFilesystemTreeOps& operator=(const TFFilesystemTreeOps&) = delete;

    static TFFilesystemTreeOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TFFilesystemTreeOps*>(ctx);
    }

    template<typename HandleT>
    static TFFilesystemTreeOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TFFilesystemTreeOps*>(handle->plugin_data);
    }

    virtual ~TFFilesystemTreeOps() = default;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    free_options(TFFilesystemOption* options, int num_options) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    create_dir(const ice::sonic::String& path) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    recursively_create_dir(const ice::sonic::String& path) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    delete_file(const ice::sonic::String& path) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    delete_dir(const ice::sonic::String& path) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> delete_recursively(
        const ice::sonic::String& path,
        uint64_t* undeleted_files,
        uint64_t* undeleted_dirs
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    rename_file(const ice::sonic::String& src, const ice::sonic::String& dst) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    copy_file(const ice::sonic::String& src, const ice::sonic::String& dst) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    path_exists(const ice::sonic::String& path) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    paths_exist(const ice::sonic::String& paths, int num_paths) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> stat(
        const ice::sonic::String& path,
        const ice::sonic::TF_FileStatisticsOps& out_stats
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    is_directory(const ice::sonic::String& path, int* out_is_directory) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_file_size(const ice::sonic::String& path, int64_t* out_size) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    translate_name(const ice::sonic::String& uri, const ice::sonic::String& out_name) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_children(const ice::sonic::String& path, TF_Tensor** out_children) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_matching_paths(const ice::sonic::String& glob, TF_Tensor** out_matches) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> flush_caches() noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_filesystem_configuration(TF_Tensor** out_config) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    set_filesystem_configuration(const ice::sonic::TF_TensorOps& options) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> get_filesystem_configuration_option(
        const ice::sonic::String& key,
        TFFilesystemOption* out_option
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    set_filesystem_configuration_option(const TFFilesystemOption* option) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_filesystem_configuration_keys(TF_Tensor** out_keys) noexcept = 0;

    void get_generic_vtable() noexcept
    {
        m_vtable = ::TFFilesystemTreeOps{
            .struct_size = TF_ILESYSTEMTREE_STRUCT_SIZE,
            .free_options =
                [](TFFilesystemTree* manager, TFFilesystemOption* options, int num_options) noexcept
            {
                auto res =
                    TFFilesystemTreeOps::from_handle(manager).free_options(options, num_options);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .create_dir =
                [](TFFilesystemTree* manager, const TF_String* path, TF_Status* out_status) noexcept
            {
                auto res = TFFilesystemTreeOps::from_handle(manager).create_dir(
                    ice::sonic::String::wrap(path)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .recursively_create_dir =
                [](TFFilesystemTree* manager, const TF_String* path, TF_Status* out_status) noexcept
            {
                auto res = TFFilesystemTreeOps::from_handle(manager).recursively_create_dir(
                    ice::sonic::String::wrap(path)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .delete_file =
                [](TFFilesystemTree* manager, const TF_String* path, TF_Status* out_status) noexcept
            {
                auto res = TFFilesystemTreeOps::from_handle(manager).delete_file(
                    ice::sonic::String::wrap(path)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .delete_dir =
                [](TFFilesystemTree* manager, const TF_String* path, TF_Status* out_status) noexcept
            {
                auto res = TFFilesystemTreeOps::from_handle(manager).delete_dir(
                    ice::sonic::String::wrap(path)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .delete_recursively =
                [](TFFilesystemTree* manager,
                   const TF_String* path,
                   uint64_t* undeleted_files,
                   uint64_t* undeleted_dirs,
                   TF_Status* out_status) noexcept
            {
                auto res = TFFilesystemTreeOps::from_handle(manager).delete_recursively(
                    ice::sonic::String::wrap(path),
                    undeleted_files,
                    undeleted_dirs
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .rename_file =
                [](TFFilesystemTree* manager,
                   const TF_String* src,
                   const TF_String* dst,
                   TF_Status* out_status) noexcept
            {
                auto res = TFFilesystemTreeOps::from_handle(manager).rename_file(
                    ice::sonic::String::wrap(src),
                    ice::sonic::String::wrap(dst)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .copy_file =
                [](TFFilesystemTree* manager,
                   const TF_String* src,
                   const TF_String* dst,
                   TF_Status* out_status) noexcept
            {
                auto res = TFFilesystemTreeOps::from_handle(manager).copy_file(
                    ice::sonic::String::wrap(src),
                    ice::sonic::String::wrap(dst)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .path_exists =
                [](TFFilesystemTree* manager, const TF_String* path, TF_Status* out_status) noexcept
            {
                auto res = TFFilesystemTreeOps::from_handle(manager).path_exists(
                    ice::sonic::String::wrap(path)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .paths_exist =
                [](TFFilesystemTree* manager,
                   const TF_String* paths,
                   int num_paths,
                   TF_Status* out_status) noexcept
            {
                auto res = TFFilesystemTreeOps::from_handle(manager).paths_exist(
                    ice::sonic::String::wrap(paths),
                    num_paths
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .stat =
                [](TFFilesystemTree* manager,
                   const TF_String* path,
                   TF_FileStatistics* out_stats,
                   TF_Status* out_status) noexcept
            {
                auto res = TFFilesystemTreeOps::from_handle(manager).stat(
                    ice::sonic::String::wrap(path),
                    ice::sonic::TF_FileStatisticsOps::wrap(out_stats)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .is_directory =
                [](TFFilesystemTree* manager,
                   const TF_String* path,
                   int* out_is_directory,
                   TF_Status* out_status) noexcept
            {
                auto res = TFFilesystemTreeOps::from_handle(manager).is_directory(
                    ice::sonic::String::wrap(path),
                    out_is_directory
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_file_size =
                [](TFFilesystemTree* manager,
                   const TF_String* path,
                   int64_t* out_size,
                   TF_Status* out_status) noexcept
            {
                auto res = TFFilesystemTreeOps::from_handle(manager).get_file_size(
                    ice::sonic::String::wrap(path),
                    out_size
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .translate_name =
                [](TFFilesystemTree* manager, const TF_String* uri, TF_String* out_name) noexcept
            {
                auto res = TFFilesystemTreeOps::from_handle(manager).translate_name(
                    ice::sonic::String::wrap(uri),
                    ice::sonic::String::wrap(out_name)
                );
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_children =
                [](TFFilesystemTree* manager,
                   const TF_String* path,
                   TF_Tensor** out_children,
                   TF_Status* out_status) noexcept
            {
                auto res = TFFilesystemTreeOps::from_handle(manager).get_children(
                    ice::sonic::String::wrap(path),
                    out_children
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_matching_paths =
                [](TFFilesystemTree* manager,
                   const TF_String* glob,
                   TF_Tensor** out_matches,
                   TF_Status* out_status) noexcept
            {
                auto res = TFFilesystemTreeOps::from_handle(manager).get_matching_paths(
                    ice::sonic::String::wrap(glob),
                    out_matches
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .flush_caches =
                [](TFFilesystemTree* manager) noexcept
            {
                auto res = TFFilesystemTreeOps::from_handle(manager).flush_caches();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_filesystem_configuration =
                [](TFFilesystemTree* manager,
                   TF_Tensor** out_config,
                   TF_Status* out_status) noexcept
            {
                auto res = TFFilesystemTreeOps::from_handle(manager).get_filesystem_configuration(
                    out_config
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .set_filesystem_configuration =
                [](TFFilesystemTree* manager,
                   const TF_Tensor* options,
                   TF_Status* out_status) noexcept
            {
                auto res = TFFilesystemTreeOps::from_handle(manager).set_filesystem_configuration(
                    ice::sonic::TF_TensorOps::wrap(options)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_filesystem_configuration_option =
                [](TFFilesystemTree* manager,
                   const TF_String* key,
                   TFFilesystemOption* out_option,
                   TF_Status* out_status) noexcept
            {
                auto res =
                    TFFilesystemTreeOps::from_handle(manager).get_filesystem_configuration_option(
                        ice::sonic::String::wrap(key),
                        out_option
                    );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .set_filesystem_configuration_option =
                [](TFFilesystemTree* manager,
                   const TFFilesystemOption* option,
                   TF_Status* out_status) noexcept
            {
                auto res =
                    TFFilesystemTreeOps::from_handle(manager).set_filesystem_configuration_option(
                        option
                    );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_filesystem_configuration_keys =
                [](TFFilesystemTree* manager, TF_Tensor** out_keys, TF_Status* out_status) noexcept
            {
                auto res =
                    TFFilesystemTreeOps::from_handle(manager).get_filesystem_configuration_keys(
                        out_keys
                    );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },

        };
    }

    const ::TFFilesystemTreeOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const TFFilesystemTree& get_handle() const noexcept
    {
        return m_handle;
    }


private:
    ::TFFilesystemTreeOps m_vtable;
    TFFilesystemTree m_handle;
};

} // namespace ice::builder
