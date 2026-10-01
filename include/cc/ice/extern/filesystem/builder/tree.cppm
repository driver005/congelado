// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/filesystem/tree.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/filesystem/tree.h"
#include "include/c/intern/file_statistics.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tensor.h"
#include "include/c/intern/tstring.h"

export module cc_ice_extern_filesystem_builder:tree;

import std;
import cc_ice_intern_sonic;

export namespace ice::builder {

class TFFilesystemTreeOps
{
public:
    explicit TFFilesystemTreeOps(
        const ::TF_FileStatisticsOps* TF_FileStatisticsOps_ops,
        const ::TF_StatusOps* Status_ops,
        const ::TF_StringOps* String_ops,
        const ::TF_TensorOps* TF_TensorOps_ops
    ) noexcept :
        m_handle{.plugin_data = this}
    {
        m_TF_FileStatisticsOps_ops = TF_FileStatisticsOps_ops;
        m_Status_ops = Status_ops;
        m_String_ops = String_ops;
        m_TF_TensorOps_ops = TF_TensorOps_ops;
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
    virtual void destroy() noexcept = 0;
    virtual void free_options(TFFilesystemOption* options, int num_options) noexcept = 0;
    virtual void
    create_dir(const ice::sonic::String& path, const ice::sonic::Status& out_status) noexcept = 0;
    virtual void recursively_create_dir(
        const ice::sonic::String& path,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void
    delete_file(const ice::sonic::String& path, const ice::sonic::Status& out_status) noexcept = 0;
    virtual void
    delete_dir(const ice::sonic::String& path, const ice::sonic::Status& out_status) noexcept = 0;
    virtual void delete_recursively(
        const ice::sonic::String& path,
        uint64_t* undeleted_files,
        uint64_t* undeleted_dirs,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void rename_file(
        const ice::sonic::String& src,
        const ice::sonic::String& dst,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void copy_file(
        const ice::sonic::String& src,
        const ice::sonic::String& dst,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void
    path_exists(const ice::sonic::String& path, const ice::sonic::Status& out_status) noexcept = 0;
    virtual void paths_exist(
        const ice::sonic::String& paths,
        int num_paths,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void stat(
        const ice::sonic::String& path,
        const ice::sonic::TF_FileStatisticsOps& out_stats,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void is_directory(
        const ice::sonic::String& path,
        int* out_is_directory,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void get_file_size(
        const ice::sonic::String& path,
        int64_t* out_size,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void
    translate_name(const ice::sonic::String& uri, const ice::sonic::String& out_name) noexcept = 0;
    virtual void get_children(
        const ice::sonic::String& path,
        TF_Tensor** out_children,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void get_matching_paths(
        const ice::sonic::String& glob,
        TF_Tensor** out_matches,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void flush_caches() noexcept = 0;
    virtual void get_filesystem_configuration(
        TF_Tensor** out_config,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void set_filesystem_configuration(
        const ice::sonic::TF_TensorOps& options,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void get_filesystem_configuration_option(
        const ice::sonic::String& key,
        TFFilesystemOption* out_option,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void set_filesystem_configuration_option(
        const TFFilesystemOption* option,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void get_filesystem_configuration_keys(
        TF_Tensor** out_keys,
        const ice::sonic::Status& out_status
    ) noexcept = 0;

    void get_generic_vtable(void (*create)(::TFFilesystemTree*)) noexcept
    {
        m_vtable = ::TFFilesystemTreeOps{
            .struct_size =
                TF_OFFSET_OF_END(::TFFilesystemTreeOps, get_filesystem_configuration_keys),

            .create = create,
            .destroy =
                [](TFFilesystemTree* handle) noexcept
            {
                auto& self = TFFilesystemTreeOps::from_handle(handle);
                self.destroy();
            },
            .free_options =
                [](TFFilesystemTree* manager, TFFilesystemOption* options, int num_options) noexcept
            {
                auto& self = TFFilesystemTreeOps::from_handle(manager);
                self.free_options(options, num_options);
            },
            .create_dir =
                [](TFFilesystemTree* manager, const TF_String* path, TF_Status* out_status) noexcept
            {
                auto& self = TFFilesystemTreeOps::from_handle(manager);
                self.create_dir(
                    self.wrap(std::type_identity<ice::sonic::String>{}, path),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .recursively_create_dir =
                [](TFFilesystemTree* manager, const TF_String* path, TF_Status* out_status) noexcept
            {
                auto& self = TFFilesystemTreeOps::from_handle(manager);
                self.recursively_create_dir(
                    self.wrap(std::type_identity<ice::sonic::String>{}, path),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .delete_file =
                [](TFFilesystemTree* manager, const TF_String* path, TF_Status* out_status) noexcept
            {
                auto& self = TFFilesystemTreeOps::from_handle(manager);
                self.delete_file(
                    self.wrap(std::type_identity<ice::sonic::String>{}, path),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .delete_dir =
                [](TFFilesystemTree* manager, const TF_String* path, TF_Status* out_status) noexcept
            {
                auto& self = TFFilesystemTreeOps::from_handle(manager);
                self.delete_dir(
                    self.wrap(std::type_identity<ice::sonic::String>{}, path),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .delete_recursively =
                [](TFFilesystemTree* manager,
                   const TF_String* path,
                   uint64_t* undeleted_files,
                   uint64_t* undeleted_dirs,
                   TF_Status* out_status) noexcept
            {
                auto& self = TFFilesystemTreeOps::from_handle(manager);
                self.delete_recursively(
                    self.wrap(std::type_identity<ice::sonic::String>{}, path),
                    undeleted_files,
                    undeleted_dirs,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .rename_file =
                [](TFFilesystemTree* manager,
                   const TF_String* src,
                   const TF_String* dst,
                   TF_Status* out_status) noexcept
            {
                auto& self = TFFilesystemTreeOps::from_handle(manager);
                self.rename_file(
                    self.wrap(std::type_identity<ice::sonic::String>{}, src),
                    self.wrap(std::type_identity<ice::sonic::String>{}, dst),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .copy_file =
                [](TFFilesystemTree* manager,
                   const TF_String* src,
                   const TF_String* dst,
                   TF_Status* out_status) noexcept
            {
                auto& self = TFFilesystemTreeOps::from_handle(manager);
                self.copy_file(
                    self.wrap(std::type_identity<ice::sonic::String>{}, src),
                    self.wrap(std::type_identity<ice::sonic::String>{}, dst),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .path_exists =
                [](TFFilesystemTree* manager, const TF_String* path, TF_Status* out_status) noexcept
            {
                auto& self = TFFilesystemTreeOps::from_handle(manager);
                self.path_exists(
                    self.wrap(std::type_identity<ice::sonic::String>{}, path),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .paths_exist =
                [](TFFilesystemTree* manager,
                   const TF_String* paths,
                   int num_paths,
                   TF_Status* out_status) noexcept
            {
                auto& self = TFFilesystemTreeOps::from_handle(manager);
                self.paths_exist(
                    self.wrap(std::type_identity<ice::sonic::String>{}, paths),
                    num_paths,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .stat =
                [](TFFilesystemTree* manager,
                   const TF_String* path,
                   TF_FileStatistics* out_stats,
                   TF_Status* out_status) noexcept
            {
                auto& self = TFFilesystemTreeOps::from_handle(manager);
                self.stat(
                    self.wrap(std::type_identity<ice::sonic::String>{}, path),
                    self.wrap(std::type_identity<ice::sonic::TF_FileStatisticsOps>{}, out_stats),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .is_directory =
                [](TFFilesystemTree* manager,
                   const TF_String* path,
                   int* out_is_directory,
                   TF_Status* out_status) noexcept
            {
                auto& self = TFFilesystemTreeOps::from_handle(manager);
                self.is_directory(
                    self.wrap(std::type_identity<ice::sonic::String>{}, path),
                    out_is_directory,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_file_size =
                [](TFFilesystemTree* manager,
                   const TF_String* path,
                   int64_t* out_size,
                   TF_Status* out_status) noexcept
            {
                auto& self = TFFilesystemTreeOps::from_handle(manager);
                self.get_file_size(
                    self.wrap(std::type_identity<ice::sonic::String>{}, path),
                    out_size,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .translate_name =
                [](TFFilesystemTree* manager, const TF_String* uri, TF_String* out_name) noexcept
            {
                auto& self = TFFilesystemTreeOps::from_handle(manager);
                self.translate_name(
                    self.wrap(std::type_identity<ice::sonic::String>{}, uri),
                    self.wrap(std::type_identity<ice::sonic::String>{}, out_name)
                );
            },
            .get_children =
                [](TFFilesystemTree* manager,
                   const TF_String* path,
                   TF_Tensor** out_children,
                   TF_Status* out_status) noexcept
            {
                auto& self = TFFilesystemTreeOps::from_handle(manager);
                self.get_children(
                    self.wrap(std::type_identity<ice::sonic::String>{}, path),
                    out_children,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_matching_paths =
                [](TFFilesystemTree* manager,
                   const TF_String* glob,
                   TF_Tensor** out_matches,
                   TF_Status* out_status) noexcept
            {
                auto& self = TFFilesystemTreeOps::from_handle(manager);
                self.get_matching_paths(
                    self.wrap(std::type_identity<ice::sonic::String>{}, glob),
                    out_matches,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .flush_caches =
                [](TFFilesystemTree* manager) noexcept
            {
                auto& self = TFFilesystemTreeOps::from_handle(manager);
                self.flush_caches();
            },
            .get_filesystem_configuration =
                [](TFFilesystemTree* manager,
                   TF_Tensor** out_config,
                   TF_Status* out_status) noexcept
            {
                auto& self = TFFilesystemTreeOps::from_handle(manager);
                self.get_filesystem_configuration(
                    out_config,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .set_filesystem_configuration =
                [](TFFilesystemTree* manager,
                   const TF_Tensor* options,
                   TF_Status* out_status) noexcept
            {
                auto& self = TFFilesystemTreeOps::from_handle(manager);
                self.set_filesystem_configuration(
                    self.wrap(std::type_identity<ice::sonic::TF_TensorOps>{}, options),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_filesystem_configuration_option =
                [](TFFilesystemTree* manager,
                   const TF_String* key,
                   TFFilesystemOption* out_option,
                   TF_Status* out_status) noexcept
            {
                auto& self = TFFilesystemTreeOps::from_handle(manager);
                self.get_filesystem_configuration_option(
                    self.wrap(std::type_identity<ice::sonic::String>{}, key),
                    out_option,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .set_filesystem_configuration_option =
                [](TFFilesystemTree* manager,
                   const TFFilesystemOption* option,
                   TF_Status* out_status) noexcept
            {
                auto& self = TFFilesystemTreeOps::from_handle(manager);
                self.set_filesystem_configuration_option(
                    option,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_filesystem_configuration_keys =
                [](TFFilesystemTree* manager, TF_Tensor** out_keys, TF_Status* out_status) noexcept
            {
                auto& self = TFFilesystemTreeOps::from_handle(manager);
                self.get_filesystem_configuration_keys(
                    out_keys,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },

        };
    }

    ice::sonic::TF_FileStatisticsOps wrap(
        std::type_identity<ice::sonic::TF_FileStatisticsOps>,
        const ::TF_FileStatistics* handle
    ) const noexcept
    {
        return ice::sonic::TF_FileStatisticsOps{
            m_TF_FileStatisticsOps_ops,
            const_cast<::TF_FileStatistics*>(handle)
        };
    }

    ice::sonic::Status
    wrap(std::type_identity<ice::sonic::Status>, const ::TF_Status* handle) const noexcept
    {
        return ice::sonic::Status{m_Status_ops, const_cast<::TF_Status*>(handle)};
    }

    ice::sonic::String
    wrap(std::type_identity<ice::sonic::String>, const ::TF_String* handle) const noexcept
    {
        return ice::sonic::String{m_String_ops, const_cast<::TF_String*>(handle)};
    }

    ice::sonic::TF_TensorOps
    wrap(std::type_identity<ice::sonic::TF_TensorOps>, const ::TF_Tensor* handle) const noexcept
    {
        return ice::sonic::TF_TensorOps{m_TF_TensorOps_ops, const_cast<::TF_Tensor*>(handle)};
    }

    const ::TFFilesystemTreeOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const ::TFFilesystemTree& get_handle() const noexcept
    {
        return m_handle;
    }

    template<typename Registry, typename StringType>
    void register_ops(
        Registry& registry,
        const StringType& type,
        const StringType& provider
    ) const noexcept
    {
        registry.register_op(type, provider, const_cast<::TFFilesystemTreeOps*>(&m_vtable));
    }

private:
    ::TFFilesystemTreeOps m_vtable;
    ::TFFilesystemTree m_handle;

    const ::TF_FileStatisticsOps* m_TF_FileStatisticsOps_ops{nullptr};

    const ::TF_StatusOps* m_Status_ops{nullptr};

    const ::TF_StringOps* m_String_ops{nullptr};

    const ::TF_TensorOps* m_TF_TensorOps_ops{nullptr};
};

} // namespace ice::builder
