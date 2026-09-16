// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from c/extern/filesystem/filesystem.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "c/extern/filesystem/filesystem.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"

export module cc_abi_builder_filesystem;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;

export namespace ice::builder {

class Filesystem
{
public:
    static Filesystem* create(void* ctx) noexcept
    {
        return static_cast<Filesystem*>(ctx);
    }

    template<typename HandleT>
    static Filesystem* create(HandleT* handle) noexcept
    {
        return static_cast<Filesystem*>(handle->plugin_data);
    }

    virtual ~Filesystem() = default;
    [[nodiscard]] std::expected<void, ice::Status>
    free_options(TF_Filesystem_Option* options, int num_options) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    create_dir(const ice::sonic::String& path) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    recursively_create_dir(const ice::sonic::String& path) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    delete_file(const ice::sonic::String& path) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    delete_dir(const ice::sonic::String& path) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> delete_recursively(
        const ice::sonic::String& path,
        uint64_t* undeleted_files,
        uint64_t* undeleted_dirs
    ) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    rename_file(const ice::sonic::String& src, const ice::sonic::String& dst) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    copy_file(const ice::sonic::String& src, const ice::sonic::String& dst) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    path_exists(const ice::sonic::String& path) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    paths_exist(const ice::sonic::String& paths, int num_paths) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    stat(const ice::sonic::String& path, const ice::sonic::Filestatistics& out_stats) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    is_directory(const ice::sonic::String& path) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    get_file_size(const ice::sonic::String& path) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    translate_name(const ice::sonic::String& uri, const ice::sonic::String& out) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    get_children(const ice::sonic::String& path) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    get_matching_paths(const ice::sonic::String& glob) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> flush_caches() noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> get_filesystem_configuration() noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    set_filesystem_configuration(const ice::sonic::Tensor& options) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> get_filesystem_configuration_option(
        const ice::sonic::String& key,
        TF_Filesystem_Option* out_option
    ) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    set_filesystem_configuration_option(const TF_Filesystem_Option* option) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> get_filesystem_configuration_keys() noexcept = 0;
    virtual ice::String get_name() const noexcept = 0;

    static TF_FilesystemOps* get_generic_vtable()
    {
        static TF_FilesystemOps vtable = {
            .struct_size = TF_FILESYSTEM_STRUCT_SIZE,

            .destroy =
                [](void* plugin_context) noexcept
            {
                delete Filesystem::create(plugin_context);
            },

            .get_name =
                [](void* plugin_context, TF_String* out) noexcept
            {
                auto* self = Filesystem::create(plugin_context);
                auto result = self->get_name();
                result.to_c(out);
            },
            .free_options =
                [](void* plugin_context, TF_Filesystem_Option* options, int num_options) noexcept
            {
                auto* self = Filesystem::create(plugin_context);
                auto res = self->free_options(options, num_options);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .create_dir =
                [](void* plugin_context, const TF_String* path, TF_Status* status) noexcept
            {
                auto* self = Filesystem::create(plugin_context);
                auto res = self->create_dir(ice::sonic::String::wrap(path));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .recursively_create_dir =
                [](void* plugin_context, const TF_String* path, TF_Status* status) noexcept
            {
                auto* self = Filesystem::create(plugin_context);
                auto res = self->recursively_create_dir(ice::sonic::String::wrap(path));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .delete_file =
                [](void* plugin_context, const TF_String* path, TF_Status* status) noexcept
            {
                auto* self = Filesystem::create(plugin_context);
                auto res = self->delete_file(ice::sonic::String::wrap(path));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .delete_dir =
                [](void* plugin_context, const TF_String* path, TF_Status* status) noexcept
            {
                auto* self = Filesystem::create(plugin_context);
                auto res = self->delete_dir(ice::sonic::String::wrap(path));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .delete_recursively =
                [](void* plugin_context,
                   const TF_String* path,
                   uint64_t* undeleted_files,
                   uint64_t* undeleted_dirs,
                   TF_Status* status) noexcept
            {
                auto* self = Filesystem::create(plugin_context);
                auto res = self->delete_recursively(
                    ice::sonic::String::wrap(path),
                    undeleted_files,
                    undeleted_dirs
                );
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .rename_file =
                [](void* plugin_context,
                   const TF_String* src,
                   const TF_String* dst,
                   TF_Status* status) noexcept
            {
                auto* self = Filesystem::create(plugin_context);
                auto res =
                    self->rename_file(ice::sonic::String::wrap(src), ice::sonic::String::wrap(dst));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .copy_file =
                [](void* plugin_context,
                   const TF_String* src,
                   const TF_String* dst,
                   TF_Status* status) noexcept
            {
                auto* self = Filesystem::create(plugin_context);
                auto res =
                    self->copy_file(ice::sonic::String::wrap(src), ice::sonic::String::wrap(dst));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .path_exists =
                [](void* plugin_context, const TF_String* path, TF_Status* status) noexcept
            {
                auto* self = Filesystem::create(plugin_context);
                auto res = self->path_exists(ice::sonic::String::wrap(path));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .paths_exist =
                [](void* plugin_context,
                   const TF_String* paths,
                   int num_paths,
                   TF_Status* status) noexcept
            {
                auto* self = Filesystem::create(plugin_context);
                auto res = self->paths_exist(ice::sonic::String::wrap(paths), num_paths);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .stat =
                [](void* plugin_context,
                   const TF_String* path,
                   TF_FileStatistics* out_stats,
                   TF_Status* status) noexcept
            {
                auto* self = Filesystem::create(plugin_context);
                auto res = self->stat(
                    ice::sonic::String::wrap(path),
                    ice::sonic::Filestatistics::wrap(out_stats)
                );
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .is_directory =
                [](void* plugin_context, const TF_String* path, TF_Status* status) noexcept
            {
                auto* self = Filesystem::create(plugin_context);
                auto res = self->is_directory(ice::sonic::String::wrap(path));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_file_size =
                [](void* plugin_context, const TF_String* path, TF_Status* status) noexcept
            {
                auto* self = Filesystem::create(plugin_context);
                auto res = self->get_file_size(ice::sonic::String::wrap(path));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .translate_name =
                [](void* plugin_context, const TF_String* uri, TF_String* out) noexcept
            {
                auto* self = Filesystem::create(plugin_context);
                auto res = self->translate_name(
                    ice::sonic::String::wrap(uri),
                    ice::sonic::String::wrap(out)
                );
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_children =
                [](void* plugin_context, const TF_String* path, TF_Status* status) noexcept
            {
                auto* self = Filesystem::create(plugin_context);
                auto res = self->get_children(ice::sonic::String::wrap(path));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_matching_paths =
                [](void* plugin_context, const TF_String* glob, TF_Status* status) noexcept
            {
                auto* self = Filesystem::create(plugin_context);
                auto res = self->get_matching_paths(ice::sonic::String::wrap(glob));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .flush_caches =
                [](void* plugin_context) noexcept
            {
                auto* self = Filesystem::create(plugin_context);
                auto res = self->flush_caches();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_filesystem_configuration =
                [](void* plugin_context, TF_Status* status) noexcept
            {
                auto* self = Filesystem::create(plugin_context);
                auto res = self->get_filesystem_configuration();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .set_filesystem_configuration =
                [](void* plugin_context, const TF_Tensor* options, TF_Status* status) noexcept
            {
                auto* self = Filesystem::create(plugin_context);
                auto res = self->set_filesystem_configuration(ice::sonic::Tensor::wrap(options));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_filesystem_configuration_option =
                [](void* plugin_context,
                   const TF_String* key,
                   TF_Filesystem_Option* out_option,
                   TF_Status* status) noexcept
            {
                auto* self = Filesystem::create(plugin_context);
                auto res = self->get_filesystem_configuration_option(
                    ice::sonic::String::wrap(key),
                    out_option
                );
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .set_filesystem_configuration_option =
                [](void* plugin_context,
                   const TF_Filesystem_Option* option,
                   TF_Status* status) noexcept
            {
                auto* self = Filesystem::create(plugin_context);
                auto res = self->set_filesystem_configuration_option(option);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_filesystem_configuration_keys =
                [](void* plugin_context, TF_Status* status) noexcept
            {
                auto* self = Filesystem::create(plugin_context);
                auto res = self->get_filesystem_configuration_keys();
                if (!res) {
                    res.error().to_c(status);
                }
            },

        };

        return &vtable;
    }
};

} // namespace ice::builder
