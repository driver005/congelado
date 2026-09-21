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
    static TFFilesystemTreeOps* create(void* ctx) noexcept
    {
        return static_cast<TFFilesystemTreeOps*>(ctx);
    }

    template<typename HandleT>
    static TFFilesystemTreeOps* create(HandleT* handle) noexcept
    {
        return static_cast<TFFilesystemTreeOps*>(handle->plugin_data);
    }

    virtual ~TFFilesystemTreeOps() = default;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    free_options(TFFilesystemOption* options, int num_options) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    create_dir(const ice::sonic::TF_StringOps& path) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    recursively_create_dir(const ice::sonic::TF_StringOps& path) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    delete_file(const ice::sonic::TF_StringOps& path) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    delete_dir(const ice::sonic::TF_StringOps& path) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> delete_recursively(
        const ice::sonic::TF_StringOps& path,
        uint64_t* undeleted_files,
        uint64_t* undeleted_dirs
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> rename_file(
        const ice::sonic::TF_StringOps& src,
        const ice::sonic::TF_StringOps& dst
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> copy_file(
        const ice::sonic::TF_StringOps& src,
        const ice::sonic::TF_StringOps& dst
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    path_exists(const ice::sonic::TF_StringOps& path) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    paths_exist(const ice::sonic::TF_StringOps& paths, int num_paths) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> stat(
        const ice::sonic::TF_StringOps& path,
        const ice::sonic::TF_FileStatisticsOps& out_stats
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    is_directory(const ice::sonic::TF_StringOps& path, int* out_is_directory) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_file_size(const ice::sonic::TF_StringOps& path, int64_t* out_size) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> translate_name(
        const ice::sonic::TF_StringOps& uri,
        const ice::sonic::TF_StringOps& out_name
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_children(const ice::sonic::TF_StringOps& path, TF_Tensor** out_children) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_matching_paths(const ice::sonic::TF_StringOps& glob, TF_Tensor** out_matches) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> flush_caches() noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_filesystem_configuration(TF_Tensor** out_config) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    set_filesystem_configuration(const ice::sonic::TF_TensorOps& options) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> get_filesystem_configuration_option(
        const ice::sonic::TF_StringOps& key,
        TFFilesystemOption* out_option
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    set_filesystem_configuration_option(const TFFilesystemOption* option) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_filesystem_configuration_keys(TF_Tensor** out_keys) noexcept = 0;

    static TFFilesystemTreeOps* get_generic_vtable()
    {
        static TFFilesystemTreeOps vtable = {
            .struct_size = TF_ILESYSTEMTREE_STRUCT_SIZE,
            .free_options =
                [](TFFilesystemTree* manager, TFFilesystemOption* options, int num_options) noexcept
            {
                auto* self = TFFilesystemTreeOps::create(manager);
                auto res = self->free_options(options, num_options);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .create_dir =
                [](TFFilesystemTree* manager, const TF_String* path, TF_Status* out_status) noexcept
            {
                auto* self = TFFilesystemTreeOps::create(manager);
                auto res = self->create_dir(ice::sonic::TF_StringOps::wrap(path));
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .recursively_create_dir =
                [](TFFilesystemTree* manager, const TF_String* path, TF_Status* out_status) noexcept
            {
                auto* self = TFFilesystemTreeOps::create(manager);
                auto res = self->recursively_create_dir(ice::sonic::TF_StringOps::wrap(path));
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .delete_file =
                [](TFFilesystemTree* manager, const TF_String* path, TF_Status* out_status) noexcept
            {
                auto* self = TFFilesystemTreeOps::create(manager);
                auto res = self->delete_file(ice::sonic::TF_StringOps::wrap(path));
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .delete_dir =
                [](TFFilesystemTree* manager, const TF_String* path, TF_Status* out_status) noexcept
            {
                auto* self = TFFilesystemTreeOps::create(manager);
                auto res = self->delete_dir(ice::sonic::TF_StringOps::wrap(path));
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
                auto* self = TFFilesystemTreeOps::create(manager);
                auto res = self->delete_recursively(
                    ice::sonic::TF_StringOps::wrap(path),
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
                auto* self = TFFilesystemTreeOps::create(manager);
                auto res = self->rename_file(
                    ice::sonic::TF_StringOps::wrap(src),
                    ice::sonic::TF_StringOps::wrap(dst)
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
                auto* self = TFFilesystemTreeOps::create(manager);
                auto res = self->copy_file(
                    ice::sonic::TF_StringOps::wrap(src),
                    ice::sonic::TF_StringOps::wrap(dst)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .path_exists =
                [](TFFilesystemTree* manager, const TF_String* path, TF_Status* out_status) noexcept
            {
                auto* self = TFFilesystemTreeOps::create(manager);
                auto res = self->path_exists(ice::sonic::TF_StringOps::wrap(path));
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
                auto* self = TFFilesystemTreeOps::create(manager);
                auto res = self->paths_exist(ice::sonic::TF_StringOps::wrap(paths), num_paths);
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
                auto* self = TFFilesystemTreeOps::create(manager);
                auto res = self->stat(
                    ice::sonic::TF_StringOps::wrap(path),
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
                auto* self = TFFilesystemTreeOps::create(manager);
                auto res =
                    self->is_directory(ice::sonic::TF_StringOps::wrap(path), out_is_directory);
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
                auto* self = TFFilesystemTreeOps::create(manager);
                auto res = self->get_file_size(ice::sonic::TF_StringOps::wrap(path), out_size);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .translate_name =
                [](TFFilesystemTree* manager, const TF_String* uri, TF_String* out_name) noexcept
            {
                auto* self = TFFilesystemTreeOps::create(manager);
                auto res = self->translate_name(
                    ice::sonic::TF_StringOps::wrap(uri),
                    ice::sonic::TF_StringOps::wrap(out_name)
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
                auto* self = TFFilesystemTreeOps::create(manager);
                auto res = self->get_children(ice::sonic::TF_StringOps::wrap(path), out_children);
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
                auto* self = TFFilesystemTreeOps::create(manager);
                auto res =
                    self->get_matching_paths(ice::sonic::TF_StringOps::wrap(glob), out_matches);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .flush_caches =
                [](TFFilesystemTree* manager) noexcept
            {
                auto* self = TFFilesystemTreeOps::create(manager);
                auto res = self->flush_caches();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_filesystem_configuration =
                [](TFFilesystemTree* manager,
                   TF_Tensor** out_config,
                   TF_Status* out_status) noexcept
            {
                auto* self = TFFilesystemTreeOps::create(manager);
                auto res = self->get_filesystem_configuration(out_config);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .set_filesystem_configuration =
                [](TFFilesystemTree* manager,
                   const TF_Tensor* options,
                   TF_Status* out_status) noexcept
            {
                auto* self = TFFilesystemTreeOps::create(manager);
                auto res =
                    self->set_filesystem_configuration(ice::sonic::TF_TensorOps::wrap(options));
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
                auto* self = TFFilesystemTreeOps::create(manager);
                auto res = self->get_filesystem_configuration_option(
                    ice::sonic::TF_StringOps::wrap(key),
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
                auto* self = TFFilesystemTreeOps::create(manager);
                auto res = self->set_filesystem_configuration_option(option);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_filesystem_configuration_keys =
                [](TFFilesystemTree* manager, TF_Tensor** out_keys, TF_Status* out_status) noexcept
            {
                auto* self = TFFilesystemTreeOps::create(manager);
                auto res = self->get_filesystem_configuration_keys(out_keys);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },

        };

        return &vtable;
    }

    builder::String get_name() const noexcept
    {
        builder::String result;
        m_ops->get_name(get_handle(), result.get_handle());
        return result;
    }
};

} // namespace ice::builder
