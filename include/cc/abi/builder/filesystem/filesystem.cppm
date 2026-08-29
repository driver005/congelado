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
    new_random_access_file(const ice::String& path) noexcept = 0;
    [[nodiscard]] virtual std::expected<std::unique_ptr<WritableFile>, ice::Status>
    new_writable_file(const ice::String& path) noexcept = 0;
    [[nodiscard]] virtual std::expected<std::unique_ptr<WritableFile>, ice::Status>
    new_appendable_file(const ice::String& path) noexcept = 0;
    [[nodiscard]] virtual std::expected<std::unique_ptr<ReadOnlyMemoryRegion>, ice::Status>
    new_read_only_memory_region_from_file(const ice::String& path) noexcept = 0;

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
        std::uint64_t* undeleted_files,
        std::uint64_t* undeleted_dirs
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    rename_file(const ice::String& src, const ice::String& dst) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    copy_file(const ice::String& src, const ice::String& dst) noexcept = 0;

    [[nodiscard]] virtual std::expected<void, ice::Status>
    path_exists(const ice::String& path) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    paths_exist(const std::vector<ice::String>& paths) noexcept = 0;

    // Range-first on the C++ side too: the C ABI's raw TF_FileStatistics* out-param is
    // adapted into a typed value the implementation fills.
    [[nodiscard]] virtual std::expected<void, ice::Status>
    stat(const ice::String& path, ice::builder::FileStatistics& out_stats) noexcept = 0;
    [[nodiscard]] virtual std::expected<bool, ice::Status>
    is_directory(const ice::String& path) noexcept = 0;
    [[nodiscard]] virtual std::expected<std::int64_t, ice::Status>
    get_file_size(const ice::String& path) noexcept = 0;
    virtual ice::String translate_name(const ice::String& uri) noexcept = 0;

    [[nodiscard]] virtual std::expected<std::vector<ice::String>, ice::Status>
    get_children(const ice::String& path) noexcept = 0;
    [[nodiscard]] virtual std::expected<std::vector<ice::String>, ice::Status>
    get_matching_paths(const ice::String& glob) noexcept = 0;

    virtual void flush_caches() noexcept = 0;

    [[nodiscard]] virtual std::expected<std::vector<TF_Filesystem_Option>, ice::Status>
    get_filesystem_configuration() noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    set_filesystem_configuration(const std::vector<TF_Filesystem_Option>& options) noexcept = 0;
    [[nodiscard]] virtual std::expected<TF_Filesystem_Option, ice::Status>
    get_filesystem_configuration_option(const ice::String& key) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    set_filesystem_configuration_option(const TF_Filesystem_Option& option) noexcept = 0;
    [[nodiscard]] virtual std::expected<std::vector<ice::String>, ice::Status>
    get_filesystem_configuration_keys() noexcept = 0;

    // Allocate a 1-D string tensor via the injected runtime and fill it from a
    // vector of ice::String values.  Returns an ice::TensorHandle — consistent
    // with the rest of the builder layer. Each row is a deep copy (String::to_c),
    // so the tensor owns its contents even after the source vector dies.
    [[nodiscard]] std::expected<ice::TensorHandle, ice::Status>
    make_string_tensor(const std::vector<ice::String>& strings) noexcept
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
        for (int64_t i = 0; i < count; ++i) {
            strings[static_cast<size_t>(i)].to_c(&dst[static_cast<size_t>(i)]);
        }
        return handle;
    }

    // Allocate a raw-bytes tensor and memcpy opts into it.
    [[nodiscard]] std::expected<ice::TensorHandle, ice::Status>
    make_options_tensor(const std::vector<TF_Filesystem_Option>& opts) noexcept
    {
        int64_t count = static_cast<int64_t>(opts.size());
        size_t bytes = static_cast<size_t>(count) * sizeof(TF_Filesystem_Option);
        auto res = m_tensor_runtime.allocate_tensor(ice::DataTypeEnum::Uint8, std::span{&count, 1}, bytes);
        if (!res) {
            return std::unexpected{res.error()};
        }
        auto* raw = res.value();
        std::memcpy(m_tensor_runtime.get_data(raw), opts.data(), bytes);
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
            .new_random_access_file =
                [](void* plugin_context, const TF_TString* path, TF_Status* status) noexcept
                -> void*
            {
                auto res = Filesystem::create(plugin_context)
                               ->new_random_access_file(ice::String::create(path));
                if (!res) {
                    res.error().to_c(status);
                    return nullptr;
                }
                return res.value().release();
            },
            .random_access_file__destroy =
                [](void* file_context) noexcept
            {
                delete RandomAccessFile::create(file_context);
            },
            .random_access_file__read =
                [](void* file_context, uint64_t offset, size_t n, char* buffer, TF_Status* status) noexcept
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
            .new_writable_file =
                [](void* plugin_context, const TF_TString* path, TF_Status* status) noexcept
                -> void*
            {
                auto res = Filesystem::create(plugin_context)
                               ->new_writable_file(ice::String::create(path));
                if (!res) {
                    res.error().to_c(status);
                    return nullptr;
                }
                return res.value().release();
            },
            .new_appendable_file =
                [](void* plugin_context, const TF_TString* path, TF_Status* status) noexcept
                -> void*
            {
                auto res = Filesystem::create(plugin_context)
                               ->new_appendable_file(ice::String::create(path));
                if (!res) {
                    res.error().to_c(status);
                    return nullptr;
                }
                return res.value().release();
            },
            .writable_file__destroy =
                [](void* file_context) noexcept
            {
                delete WritableFile::create(file_context);
            },
            .writable_file__append =
                [](void* file_context, const TF_TString* buffer, size_t n, TF_Status* status) noexcept
            {
                // Honor the C contract: `buffer` points to `n` bytes (n may differ from
                // the TF_TString's stored size).
                const std::string_view text = buffer
                                                  ? std::string_view{string_get_data_pointer(buffer), n}
                                                  : std::string_view{};
                auto res = WritableFile::create(file_context)->append(ice::String{text});
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .writable_file__tell = [](void* file_context, TF_Status* status) noexcept -> int64_t
            {
                auto res = WritableFile::create(file_context)->tell();
                if (!res) {
                    res.error().to_c(status);
                    return -1;
                }
                return res.value();
            },
            .writable_file__flush =
                [](void* file_context, TF_Status* status) noexcept
            {
                auto res = WritableFile::create(file_context)->flush();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .writable_file__sync =
                [](void* file_context, TF_Status* status) noexcept
            {
                auto res = WritableFile::create(file_context)->sync();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .writable_file__close =
                [](void* file_context, TF_Status* status) noexcept
            {
                auto res = WritableFile::create(file_context)->close();
                if (!res) {
                    res.error().to_c(status);
                }
            },

            // ReadOnlyMemoryRegion — the C ABI's __data/__length pair derives from the
            // C++ interface's single span.
            .new_read_only_memory_region_from_file =
                [](void* plugin_context, const TF_TString* path, TF_Status* status) noexcept
                -> void*
            {
                auto res = Filesystem::create(plugin_context)
                               ->new_read_only_memory_region_from_file(ice::String::create(path));
                if (!res) {
                    res.error().to_c(status);
                    return nullptr;
                }
                return res.value().release();
            },
            .read_only_memory_region__destroy =
                [](void* region_context) noexcept
            {
                delete ReadOnlyMemoryRegion::create(region_context);
            },
            .read_only_memory_region__data = [](void* region_context) noexcept -> const void*
            {
                return ReadOnlyMemoryRegion::create(region_context)->data().data();
            },
            .read_only_memory_region__length = [](void* region_context) noexcept -> uint64_t
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
                auto res = Filesystem::create(plugin_context)
                               ->delete_recursively(
                                   ice::String::create(path),
                                   undeleted_files,
                                   undeleted_dirs
                               );
                if (!res) {
                    res.error().to_c(status);
                }
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
                    std::vector<ice::String> str_paths;
                    str_paths.reserve(static_cast<size_t>(num_paths));
                    for (int i = 0; i < num_paths; ++i) {
                        str_paths.push_back(ice::String::create(&paths[i]));
                    }
                    auto res = Filesystem::create(plugin_context)->paths_exist(str_paths);
                    if (!res) {
                        res.error().to_c(status);
                    }
                } catch (const std::exception& e) {
                    // vector allocation inside a noexcept lambda must not terminate.
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

            // get_children — returns ice::TensorHandle unwrapped to TF_Tensor_Handle*.
            .get_children = [](void* plugin_context,
                               const TF_TString* path,
                               TF_Status* status) noexcept -> TF_Tensor_Handle*
            {
                auto* self = Filesystem::create(plugin_context);
                auto res = self->get_children(ice::String::create(path));
                if (!res) {
                    res.error().to_c(status);
                    return nullptr;
                }
                auto t = self->make_string_tensor(res.value());
                if (!t) {
                    t.error().to_c(status);
                    return nullptr;
                }
                return t->get_handle();
            },

            // get_matching_paths — same contract as get_children.
            .get_matching_paths = [](void* plugin_context,
                                     const TF_TString* glob,
                                     TF_Status* status) noexcept -> TF_Tensor_Handle*
            {
                auto* self = Filesystem::create(plugin_context);
                auto res = self->get_matching_paths(ice::String::create(glob));
                if (!res) {
                    res.error().to_c(status);
                    return nullptr;
                }
                auto t = self->make_string_tensor(res.value());
                if (!t) {
                    t.error().to_c(status);
                    return nullptr;
                }
                return t->get_handle();
            },

            .flush_caches =
                [](void* plugin_context) noexcept
            {
                Filesystem::create(plugin_context)->flush_caches();
            },

            // get_filesystem_configuration — tensor of raw TF_Filesystem_Option structs.
            .get_filesystem_configuration = [](void* plugin_context,
                                               TF_Status* status) noexcept -> TF_Tensor_Handle*
            {
                auto* self = Filesystem::create(plugin_context);
                auto res = self->get_filesystem_configuration();
                if (!res) {
                    res.error().to_c(status);
                    return nullptr;
                }
                auto t = self->make_options_tensor(res.value());
                if (!t) {
                    t.error().to_c(status);
                    return nullptr;
                }
                return t->get_handle();
            },

            // set_filesystem_configuration — tensor carries raw TF_Filesystem_Option array.
            .set_filesystem_configuration =
                [](void* plugin_context, const TF_Tensor_Handle* options_handle, TF_Status* status) noexcept
            {
                auto* self = Filesystem::create(plugin_context);
                ice::TensorHandle h{options_handle};
                int64_t count = self->m_tensor_runtime.get_num_dims(h.get_handle()) > 0
                                    ? self->m_tensor_runtime.get_dim(h.get_handle(), 0)
                                    : 0;
                const auto* raw = static_cast<const TF_Filesystem_Option*>(
                    self->m_tensor_runtime.get_data(h.get_handle())
                );
                auto res = self->set_filesystem_configuration({raw, raw + count});
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
                *out_option = res.value();
            },

            .set_filesystem_configuration_option =
                [](void* plugin_context, const TF_Filesystem_Option* option, TF_Status* status) noexcept
            {
                auto res = Filesystem::create(plugin_context)
                               ->set_filesystem_configuration_option(*option);
                if (!res) {
                    res.error().to_c(status);
                }
            },

            // get_filesystem_configuration_keys — same string-tensor contract as get_children.
            .get_filesystem_configuration_keys = [](void* plugin_context,
                                                    TF_Status* status) noexcept -> TF_Tensor_Handle*
            {
                auto* self = Filesystem::create(plugin_context);
                auto res = self->get_filesystem_configuration_keys();
                if (!res) {
                    res.error().to_c(status);
                    return nullptr;
                }
                auto t = self->make_string_tensor(res.value());
                if (!t) {
                    t.error().to_c(status);
                    return nullptr;
                }
                return t->get_handle();
            },
        };
        return &vtable;
    }

protected:
    ice::sonic::Tensor& m_tensor_runtime;
};

} // namespace ice::builder
