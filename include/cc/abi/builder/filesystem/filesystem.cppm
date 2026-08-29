module;

#include "c/extern/filesystem/filesystem.h"
#include "c/extern/filesystem/option_types.h"
#include "c/intern/tf_file_statistics.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"

#include <cstdlib>
#include <cstring>

export module cc_abi_builder_filesystem;

import std;
export import :leaves;
import cc_abi_builder_intern;
import cc_abi_primitives;
import cc_abi_sonic_intern;

export namespace ice::builder {

class Filesystem
{
public:
    // Recover the Filesystem instance from the opaque void* context slot that every
    // C vtable callback receives.  Named accessor so the cast intent is explicit
    // at the call site and the static_cast appears exactly once, here.
    static Filesystem* create(void* ctx) noexcept
    {
        return static_cast<Filesystem*>(ctx);
    }

    // Tensor runtime injected at construction — all tensor allocation goes through it,
    // no raw TF_AllocateTensor calls anywhere in this class.
    explicit Filesystem(ice::sonic::Tensor& tensor_runtime) noexcept :
        m_tensor_runtime{tensor_runtime}
    {
    }

    virtual ~Filesystem() = default;

    // Exception contract: every member of this interface is noexcept — the
    // std::expected return is the only failure channel; a throwing implementation fails
    // fast at the ABI boundary instead of unwinding through C code.
    virtual ice::String get_name() const noexcept = 0;

    [[nodiscard]] virtual std::expected<std::unique_ptr<RandomAccessFile>, ice::Status>
    create_random_access_file(const ice::String& path) noexcept = 0;
    [[nodiscard]] virtual std::expected<std::unique_ptr<WritableFile>, ice::Status>
    create_writable_file(const ice::String& path) noexcept = 0;
    [[nodiscard]] virtual std::expected<std::unique_ptr<WritableFile>, ice::Status>
    create_appendable_file(const ice::String& path) noexcept = 0;
    [[nodiscard]] virtual std::expected<std::unique_ptr<ReadOnlyMemoryRegion>, ice::Status>
    create_read_only_memory_region_from_file(const ice::String& path) noexcept = 0;

    [[nodiscard]] virtual std::expected<void, ice::Status>
    create_dir(const ice::String& path) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    recursively_create_dir(const ice::String& path) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    delete_file(const ice::String& path) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    delete_dir(const ice::String& path) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> delete_recursively(
        const ice::String& path,
        DeleteRecursivelyResult& out
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    rename_file(const ice::String& src, const ice::String& dst) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    copy_file(const ice::String& src, const ice::String& dst) noexcept = 0;

    [[nodiscard]] virtual std::expected<void, ice::Status>
    path_exists(const ice::String& path) noexcept = 0;
    // paths_exist — `paths` is a 1-D string tensor (build it with make_string_tensor);
    // the vtable adapter decodes it to the C slot's (const TF_TString*, int) shape.
    [[nodiscard]] virtual std::expected<void, ice::Status>
    paths_exist(ice::TensorHandle paths) noexcept = 0;

    // Range-first on the C++ side too: the C ABI's raw TF_FileStatistics* out-param is
    // adapted into a typed value the implementation fills.
    [[nodiscard]] virtual std::expected<void, ice::Status>
    stat(const ice::String& path, ice::builder::FileStatistics& out_stats) noexcept = 0;
    [[nodiscard]] virtual std::expected<bool, ice::Status>
    is_directory(const ice::String& path) noexcept = 0;
    [[nodiscard]] virtual std::expected<std::int64_t, ice::Status>
    get_file_size(const ice::String& path) noexcept = 0;
    virtual ice::String translate_name(const ice::String& uri) noexcept = 0;

    // Children/matching-path results come back as string tensors the implementation
    // allocates through its injected tensor runtime.
    [[nodiscard]] virtual std::expected<ice::TensorHandle, ice::Status>
    get_children(const ice::String& path) noexcept = 0;
    [[nodiscard]] virtual std::expected<ice::TensorHandle, ice::Status>
    get_matching_paths(const ice::String& glob) noexcept = 0;

    virtual void flush_caches() noexcept = 0;

    // Configuration travels as a raw-bytes options tensor (make_options_tensor) and as
    // FilesystemOption values on the single-option accessors.
    [[nodiscard]] virtual std::expected<ice::TensorHandle, ice::Status>
    get_filesystem_configuration() noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    set_filesystem_configuration(ice::TensorHandle options) noexcept = 0;
    [[nodiscard]] virtual std::expected<ice::builder::FilesystemOption, ice::Status>
    get_filesystem_configuration_option(const ice::String& key) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    set_filesystem_configuration_option(const ice::builder::FilesystemOption& option) noexcept = 0;
    [[nodiscard]] virtual std::expected<ice::TensorHandle, ice::Status>
    get_filesystem_configuration_keys() noexcept = 0;

    // Allocate a 1-D string tensor via the injected runtime and fill it from a
    // span of ice::String values.  Returns an ice::TensorHandle — consistent
    // with the rest of the builder layer. Each row is a deep copy (String::to_c),
    // so the tensor owns its contents even after the source span's storage dies.
    [[nodiscard]] std::expected<ice::TensorHandle, ice::Status>
    make_string_tensor(std::span<const ice::String> strings) noexcept
    {
        int64_t count = static_cast<int64_t>(strings.size());
        auto res = m_tensor_runtime.allocate_tensor(
            ice::DataTypeEnum::String,
            std::span{&count, 1},
            static_cast<size_t>(count) * sizeof(TF_TString)
        );
        if (!res) {
            return std::unexpected{res.error()};
        }
        auto* raw = res.value();
        ice::TensorHandle handle{raw};
        auto dst = m_tensor_runtime.get_data_as<TF_TString>(raw);
        for (size_t i = 0; i < strings.size(); ++i) {
            strings[i].to_c(&dst[i]);
        }
        return handle;
    }

    // Allocate a raw-bytes tensor and memcpy each wrapped option's C value into it.
    [[nodiscard]] std::expected<ice::TensorHandle, ice::Status>
    make_options_tensor(std::span<const ice::builder::FilesystemOption> opts) noexcept
    {
        int64_t count = static_cast<int64_t>(opts.size());
        size_t bytes = static_cast<size_t>(count) * sizeof(TF_Filesystem_Option);
        auto res = m_tensor_runtime.allocate_tensor(ice::DataTypeEnum::Uint8, std::span{&count, 1}, bytes);
        if (!res) {
            return std::unexpected{res.error()};
        }
        auto* raw = res.value();
        auto* dst = static_cast<TF_Filesystem_Option*>(m_tensor_runtime.get_data(raw));
        for (size_t i = 0; i < opts.size(); ++i) {
            TF_Filesystem_Option c = opts[i].to_c();
            std::memcpy(&dst[i], &c, sizeof(TF_Filesystem_Option));
        }
        return ice::TensorHandle{raw};
    }

    static TF_Filesystem* get_generic_vtable()
    {
        static TF_Filesystem vtable = {
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
                auto name = self->get_name();
                name.to_c(out);
            },
            .free_options =
                [](void* plugin_context, TF_Filesystem_Option* options, int /*num_options*/) noexcept
            {
                free(options);
            },

            // RandomAccessFile
            .create_random_access_file =
                [](void* plugin_context, const TF_TString* path, TF_Status* status) noexcept
                -> TF_RandomAccessFile*
            {
                auto res = Filesystem::create(plugin_context)
                               ->create_random_access_file(ice::String::create(path));
                if (!res) {
                    res.error().to_c(status);
                    return nullptr;
                }
                return static_cast<TF_RandomAccessFile*>(static_cast<void*>(res->release()));
            },
            .random_access_file__destroy =
                [](TF_RandomAccessFile* file_context) noexcept
            {
                delete RandomAccessFile::create(file_context);
            },
            .random_access_file__read =
                [](TF_RandomAccessFile* file_context, uint64_t offset, size_t n, char* buffer, TF_Status* status) noexcept
                -> int64_t
            {
                auto res = RandomAccessFile::create(file_context)
                               ->read(offset, std::span{buffer, n});
                if (!res) {
                    res.error().to_c(status);
                    return -1;
                }
                return res.value();
            },

            // WritableFile
            .create_writable_file =
                [](void* plugin_context, const TF_TString* path, TF_Status* status) noexcept
                -> TF_WritableFile*
            {
                auto res = Filesystem::create(plugin_context)
                               ->create_writable_file(ice::String::create(path));
                if (!res) {
                    res.error().to_c(status);
                    return nullptr;
                }
                return static_cast<TF_WritableFile*>(static_cast<void*>(res->release()));
            },
            .create_appendable_file =
                [](void* plugin_context, const TF_TString* path, TF_Status* status) noexcept
                -> TF_WritableFile*
            {
                auto res = Filesystem::create(plugin_context)
                               ->create_appendable_file(ice::String::create(path));
                if (!res) {
                    res.error().to_c(status);
                    return nullptr;
                }
                return static_cast<TF_WritableFile*>(static_cast<void*>(res->release()));
            },
            .writable_file__destroy =
                [](TF_WritableFile* file_context) noexcept
            {
                delete WritableFile::create(file_context);
            },
            .writable_file__append =
                [](TF_WritableFile* file_context, const TF_TString* buffer, TF_Status* status) noexcept
            {
                auto res =
                    WritableFile::create(file_context)->append(ice::String::create(buffer));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .writable_file__tell = [](TF_WritableFile* file_context, TF_Status* status) noexcept -> int64_t
            {
                auto res = WritableFile::create(file_context)->tell();
                if (!res) {
                    res.error().to_c(status);
                    return -1;
                }
                return res.value();
            },
            .writable_file__flush =
                [](TF_WritableFile* file_context, TF_Status* status) noexcept
            {
                auto res = WritableFile::create(file_context)->flush();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .writable_file__sync =
                [](TF_WritableFile* file_context, TF_Status* status) noexcept
            {
                auto res = WritableFile::create(file_context)->sync();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .writable_file__close =
                [](TF_WritableFile* file_context, TF_Status* status) noexcept
            {
                auto res = WritableFile::create(file_context)->close();
                if (!res) {
                    res.error().to_c(status);
                }
            },

            // ReadOnlyMemoryRegion — the C ABI's __data/__length pair derives from the
            // C++ interface's single span.
            .create_read_only_memory_region_from_file =
                [](void* plugin_context, const TF_TString* path, TF_Status* status) noexcept
                -> TF_ReadOnlyMemoryRegion*
            {
                auto res = Filesystem::create(plugin_context)
                               ->create_read_only_memory_region_from_file(ice::String::create(path));
                if (!res) {
                    res.error().to_c(status);
                    return nullptr;
                }
                return static_cast<TF_ReadOnlyMemoryRegion*>(static_cast<void*>(res->release()));
            },
            .read_only_memory_region__destroy =
                [](TF_ReadOnlyMemoryRegion* region_context) noexcept
            {
                delete ReadOnlyMemoryRegion::create(region_context);
            },
            .read_only_memory_region__data = [](TF_ReadOnlyMemoryRegion* region_context) noexcept -> const void*
            {
                return ReadOnlyMemoryRegion::create(region_context)->data().data();
            },
            .read_only_memory_region__length = [](TF_ReadOnlyMemoryRegion* region_context) noexcept -> uint64_t
            {
                return ReadOnlyMemoryRegion::create(region_context)->data().size();
            },

            // Filesystem ops
            .create_dir =
                [](void* plugin_context, const TF_TString* path, TF_Status* status) noexcept
            {
                auto res =
                    Filesystem::create(plugin_context)->create_dir(ice::String::create(path));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .recursively_create_dir =
                [](void* plugin_context, const TF_TString* path, TF_Status* status) noexcept
            {
                auto res = Filesystem::create(plugin_context)
                               ->recursively_create_dir(ice::String::create(path));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .delete_file =
                [](void* plugin_context, const TF_TString* path, TF_Status* status) noexcept
            {
                auto res =
                    Filesystem::create(plugin_context)->delete_file(ice::String::create(path));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .delete_dir =
                [](void* plugin_context, const TF_TString* path, TF_Status* status) noexcept
            {
                auto res =
                    Filesystem::create(plugin_context)->delete_dir(ice::String::create(path));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .delete_recursively =
                [](void* plugin_context,
                   const TF_TString* path,
                   uint64_t* undeleted_files,
                   uint64_t* undeleted_dirs,
                   TF_Status* status) noexcept
            {
                ice::builder::DeleteRecursivelyResult result{};
                auto res = Filesystem::create(plugin_context)
                               ->delete_recursively(ice::String::create(path), result);
                if (!res) {
                    res.error().to_c(status);
                    return;
                }
                *undeleted_files = result.undeleted_files;
                *undeleted_dirs = result.undeleted_dirs;
            },
            .rename_file =
                [](void* plugin_context,
                   const TF_TString* src,
                   const TF_TString* dst,
                   TF_Status* status) noexcept
            {
                auto res = Filesystem::create(plugin_context)
                               ->rename_file(ice::String::create(src), ice::String::create(dst));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .copy_file =
                [](void* plugin_context,
                   const TF_TString* src,
                   const TF_TString* dst,
                   TF_Status* status) noexcept
            {
                auto res = Filesystem::create(plugin_context)
                               ->copy_file(ice::String::create(src), ice::String::create(dst));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .path_exists =
                [](void* plugin_context, const TF_TString* path, TF_Status* status) noexcept
            {
                auto res =
                    Filesystem::create(plugin_context)->path_exists(ice::String::create(path));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .paths_exist =
                [](void* plugin_context, const TF_TString* paths, int num_paths, TF_Status* status) noexcept
            {
                try {
                    auto* self = Filesystem::create(plugin_context);
                    // No heap-allocated container in the ABI tier: the raw C array is
                    // adapted into a string tensor; unique_ptr<T[]> is the only heap
                    // allocation here.
                    std::unique_ptr<ice::String[]> str_paths{new ice::String[num_paths]};
                    for (int i = 0; i < num_paths; ++i) {
                        str_paths[i] = ice::String::create(&paths[i]);
                    }
                    auto tensor = self->make_string_tensor(
                        {str_paths.get(), static_cast<size_t>(num_paths)}
                    );
                    if (!tensor) {
                        tensor.error().to_c(status);
                        return;
                    }
                    auto res = self->paths_exist(tensor.value());
                    // The interface method consumes the tensor synchronously; the
                    // adapter owns it and must free it with the tensor runtime.
                    self->m_tensor_runtime.delete_tensor(tensor.value().get_handle());
                    if (!res) {
                        res.error().to_c(status);
                    }
                } catch (const std::exception& e) {
                    // unique_ptr allocation inside a noexcept lambda must not terminate.
                    ice::Status{ice::StatusCode::ResourceExhausted, e.what()}.to_c(status);
                } catch (...) {
                    ice::Status{ice::StatusCode::ResourceExhausted, "path list allocation failed"}
                        .to_c(status);
                }
            },
            .stat =
                [](void* plugin_context,
                   const TF_TString* path,
                   TF_FileStatistics* out_stats,
                   TF_Status* status) noexcept
            {
                ice::builder::FileStatistics stats;
                auto res = Filesystem::create(plugin_context)
                               ->stat(ice::String::create(path), stats);
                if (!res) {
                    res.error().to_c(status);
                    return;
                }
                stats.to_c(out_stats);
            },
            .is_directory =
                [](void* plugin_context, const TF_TString* path, TF_Status* status) noexcept
                -> bool
            {
                auto res =
                    Filesystem::create(plugin_context)->is_directory(ice::String::create(path));
                if (!res) {
                    res.error().to_c(status);
                    return false;
                }
                return res.value();
            },
            .get_file_size =
                [](void* plugin_context, const TF_TString* path, TF_Status* status) noexcept
                -> int64_t
            {
                auto res =
                    Filesystem::create(plugin_context)->get_file_size(ice::String::create(path));
                if (!res) {
                    res.error().to_c(status);
                    return -1;
                }
                return res.value();
            },
            .translate_name =
                [](void* plugin_context, const TF_TString* uri, TF_String* out) noexcept
            {
                // to_c deep-copies, so the temporary result String may die here safely.
                Filesystem::create(plugin_context)
                    ->translate_name(ice::String::create(uri))
                    .to_c(out);
            },

            // get_children — the implementation returns a string tensor; unwrap it to
            // the C slot's TF_Tensor_Handle* (no re-encoding here).
            .get_children = [](void* plugin_context,
                               const TF_TString* path,
                               TF_Status* status) noexcept -> TF_Tensor_Handle*
            {
                auto res = Filesystem::create(plugin_context)
                               ->get_children(ice::String::create(path));
                if (!res) {
                    res.error().to_c(status);
                    return nullptr;
                }
                return res->get_handle();
            },

            // get_matching_paths — same contract as get_children.
            .get_matching_paths = [](void* plugin_context,
                                     const TF_TString* glob,
                                     TF_Status* status) noexcept -> TF_Tensor_Handle*
            {
                auto res = Filesystem::create(plugin_context)
                               ->get_matching_paths(ice::String::create(glob));
                if (!res) {
                    res.error().to_c(status);
                    return nullptr;
                }
                return res->get_handle();
            },

            .flush_caches =
                [](void* plugin_context) noexcept
            {
                Filesystem::create(plugin_context)->flush_caches();
            },

            // get_filesystem_configuration — the implementation returns an options
            // tensor (raw TF_Filesystem_Option structs); unwrap it directly.
            .get_filesystem_configuration = [](void* plugin_context,
                                               TF_Status* status) noexcept -> TF_Tensor_Handle*
            {
                auto res = Filesystem::create(plugin_context)->get_filesystem_configuration();
                if (!res) {
                    res.error().to_c(status);
                    return nullptr;
                }
                return res->get_handle();
            },

            // set_filesystem_configuration — the options tensor is passed through
            // opaquely; the implementation decodes it.
            .set_filesystem_configuration =
                [](void* plugin_context, const TF_Tensor_Handle* options_handle, TF_Status* status) noexcept
            {
                auto res = Filesystem::create(plugin_context)
                               ->set_filesystem_configuration(ice::TensorHandle{options_handle});
                if (!res) {
                    res.error().to_c(status);
                }
            },

            .get_filesystem_configuration_option =
                [](void* plugin_context,
                   const TF_TString* key,
                   TF_Filesystem_Option* out_option,
                   TF_Status* status) noexcept
            {
                auto res = Filesystem::create(plugin_context)
                               ->get_filesystem_configuration_option(ice::String::create(key));
                if (!res) {
                    res.error().to_c(status);
                    return;
                }
                *out_option = res.value().to_c();
            },

            .set_filesystem_configuration_option =
                [](void* plugin_context, const TF_Filesystem_Option* option, TF_Status* status) noexcept
            {
                auto res = Filesystem::create(plugin_context)
                               ->set_filesystem_configuration_option(
                                   ice::builder::FilesystemOption::create(*option)
                               );
                if (!res) {
                    res.error().to_c(status);
                }
            },

            // get_filesystem_configuration_keys — same string-tensor contract as get_children.
            .get_filesystem_configuration_keys = [](void* plugin_context,
                                                    TF_Status* status) noexcept -> TF_Tensor_Handle*
            {
                auto res = Filesystem::create(plugin_context)
                               ->get_filesystem_configuration_keys();
                if (!res) {
                    res.error().to_c(status);
                    return nullptr;
                }
                return res->get_handle();
            },
        };
        return &vtable;
    }

protected:
    ice::sonic::Tensor& m_tensor_runtime;
};

} // namespace ice::builder
