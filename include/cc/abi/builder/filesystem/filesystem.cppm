module;

#include "c/extern/filesystem/option_types.h"
#include "c/intern/tf_file_statistics.h"

#include <cstring>

export module cc_abi_builder_filesystem;

export import :leaves;
import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;
import :leaves;

export namespace ice::builder {

class Filesystem
{
public:
    // Tensor runtime injected at construction — all tensor allocation goes through it,
    // no raw TF_AllocateTensor calls anywhere in this class.
    explicit Filesystem(ice::sonic::Tensor& tensor_runtime) :
        m_tensor_runtime{tensor_runtime}
    {
    }

    virtual ~Filesystem() = default;

    virtual std::expected<std::unique_ptr<RandomAccessFile>, ice::Status>
    new_random_access_file(const ice::String& path) = 0;
    virtual std::expected<std::unique_ptr<WritableFile>, ice::Status>
    new_writable_file(const ice::String& path) = 0;
    virtual std::expected<std::unique_ptr<WritableFile>, ice::Status>
    new_appendable_file(const ice::String& path) = 0;
    virtual std::expected<std::unique_ptr<ReadOnlyMemoryRegion>, ice::Status>
    new_read_only_memory_region_from_file(const ice::String& path) = 0;

    virtual std::expected<void, ice::Status> create_dir(const ice::String& path) = 0;
    virtual std::expected<void, ice::Status> recursively_create_dir(const ice::String& path) = 0;
    virtual std::expected<void, ice::Status> delete_file(const ice::String& path) = 0;
    virtual std::expected<void, ice::Status> delete_dir(const ice::String& path) = 0;
    virtual std::expected<void, ice::Status> delete_recursively(
        const ice::String& path, std::uint64_t* undeleted_files, std::uint64_t* undeleted_dirs
    ) = 0;
    virtual std::expected<void, ice::Status>
    rename_file(const ice::String& src, const ice::String& dst) = 0;
    virtual std::expected<void, ice::Status>
    copy_file(const ice::String& src, const ice::String& dst) = 0;

    virtual std::expected<void, ice::Status> path_exists(const ice::String& path) = 0;
    virtual std::vector<std::expected<void, ice::Status>>
    paths_exist(const std::vector<ice::String>& paths) = 0;

    virtual std::expected<void, ice::Status>
    stat(const ice::String& path, TF_FileStatistics* out_stats) = 0;
    virtual std::expected<bool, ice::Status> is_directory(const ice::String& path) = 0;
    virtual std::expected<std::int64_t, ice::Status> get_file_size(const ice::String& path) = 0;
    virtual ice::String translate_name(const ice::String& uri) = 0;

    virtual std::expected<std::vector<ice::String>, ice::Status>
    get_children(const ice::String& path) = 0;
    virtual std::expected<std::vector<ice::String>, ice::Status>
    get_matching_paths(const ice::String& glob) = 0;

    virtual void flush_caches() = 0;

    virtual std::expected<std::vector<TF_Filesystem_Option>, ice::Status>
    get_filesystem_configuration() = 0;
    virtual std::expected<void, ice::Status>
    set_filesystem_configuration(const std::vector<TF_Filesystem_Option>& options) = 0;
    virtual std::expected<TF_Filesystem_Option, ice::Status>
    get_filesystem_configuration_option(const ice::String& key) = 0;
    virtual std::expected<void, ice::Status>
    set_filesystem_configuration_option(const TF_Filesystem_Option& option) = 0;
    virtual std::expected<std::vector<ice::String>, ice::Status>
    get_filesystem_configuration_keys() = 0;

    // Allocate a 1-D string tensor via the injected runtime and fill it from a
    // vector of ice::String values.  Returns an ice::TensorHandle — consistent
    // with the rest of the builder layer.
    std::expected<ice::TensorHandle, ice::Status>
    make_string_tensor(const std::vector<ice::String>& strings)
    {
        int64_t count = static_cast<int64_t>(strings.size());
        auto* raw = m_tensor_runtime.allocate_tensor(
            ice::builder::DataTypeEnum::String, &count, 1,
            static_cast<size_t>(count) * sizeof(TF_TString)
        );
        if (!raw) {
            return std::unexpected{ice::Status{"OOM: string tensor"}};
        }
        ice::TensorHandle handle{raw};
        auto* dst = m_tensor_runtime.get_data_as<TF_TString>(raw);
        for (int64_t i = 0; i < count; ++i) {
            strings[static_cast<size_t>(i)].to_c(&dst[i]);
        }
        return handle;
    }

    // Allocate a raw-bytes tensor and memcpy opts into it.
    std::expected<ice::TensorHandle, ice::Status>
    make_options_tensor(const std::vector<TF_Filesystem_Option>& opts)
    {
        int64_t count = static_cast<int64_t>(opts.size());
        size_t bytes = static_cast<size_t>(count) * sizeof(TF_Filesystem_Option);
        auto* raw =
            m_tensor_runtime.allocate_tensor(ice::builder::DataTypeEnum::Uint8, &count, 1, bytes);
        if (!raw) {
            return std::unexpected{ice::Status{"OOM: options tensor"}};
        }
        std::memcpy(m_tensor_runtime.get_data(raw), opts.data(), bytes);
        return ice::TensorHandle{raw};
    }

    static const void* get_generic_vtable()
    {
        static TF_Filesystem vtable = {
            sizeof(TF_Filesystem),

            // free_options
            [](TF_Filesystem_Option* options, int /*num_options*/) {
                free(options);
            },

            // destroy
            [](void* plugin_context) {
                delete static_cast<Filesystem*>(plugin_context);
            },

            // RandomAccessFile
            [](void* plugin_context, const TF_TString* path, TF_Status* status) -> void* {
                auto res = static_cast<Filesystem*>(plugin_context)
                               ->new_random_access_file(ice::String::create(path));
                if (!res) {
                    res.error().to_c(status);
                    return nullptr;
                }
                return res.value().release();
            },
            [](void* file_context) {
                delete static_cast<RandomAccessFile*>(file_context);
            },
            [](void* file_context, uint64_t offset, size_t n, char* buffer,
               TF_Status* status) -> int64_t {
                auto res = static_cast<RandomAccessFile*>(file_context)->read(offset, n, buffer);
                if (!res) {
                    res.error().to_c(status);
                    return -1;
                }
                return res.value();
            },

            // WritableFile
            [](void* plugin_context, const TF_TString* path, TF_Status* status) -> void* {
                auto res = static_cast<Filesystem*>(plugin_context)
                               ->new_writable_file(ice::String::create(path));
                if (!res) {
                    res.error().to_c(status);
                    return nullptr;
                }
                return res.value().release();
            },
            [](void* plugin_context, const TF_TString* path, TF_Status* status) -> void* {
                auto res = static_cast<Filesystem*>(plugin_context)
                               ->new_appendable_file(ice::String::create(path));
                if (!res) {
                    res.error().to_c(status);
                    return nullptr;
                }
                return res.value().release();
            },
            [](void* file_context) {
                delete static_cast<WritableFile*>(file_context);
            },
            [](void* file_context, const TF_String* buffer, size_t n, TF_Status* status) {
                auto sr = ice::String::create(buffer);
                auto res = static_cast<WritableFile*>(file_context)->append(sr.c_str(), n);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            [](void* file_context, TF_Status* status) -> int64_t {
                auto res = static_cast<WritableFile*>(file_context)->tell();
                if (!res) {
                    res.error().to_c(status);
                    return -1;
                }
                return res.value();
            },
            [](void* file_context, TF_Status* status) {
                auto res = static_cast<WritableFile*>(file_context)->flush();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            [](void* file_context, TF_Status* status) {
                auto res = static_cast<WritableFile*>(file_context)->sync();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            [](void* file_context, TF_Status* status) {
                auto res = static_cast<WritableFile*>(file_context)->close();
                if (!res) {
                    res.error().to_c(status);
                }
            },

            // ReadOnlyMemoryRegion
            [](void* plugin_context, const TF_TString* path, TF_Status* status) -> void* {
                auto res = static_cast<Filesystem*>(plugin_context)
                               ->new_read_only_memory_region_from_file(ice::String::create(path));
                if (!res) {
                    res.error().to_c(status);
                    return nullptr;
                }
                return res.value().release();
            },
            [](void* region_context) {
                delete static_cast<ReadOnlyMemoryRegion*>(region_context);
            },
            [](void* region_context) -> const void* {
                return static_cast<ReadOnlyMemoryRegion*>(region_context)->data();
            },
            [](void* region_context) -> uint64_t {
                return static_cast<ReadOnlyMemoryRegion*>(region_context)->length();
            },

            // Filesystem ops
            [](void* plugin_context, const TF_TString* path, TF_Status* status) {
                auto res =
                    static_cast<Filesystem*>(plugin_context)->create_dir(ice::String::create(path));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            [](void* plugin_context, const TF_TString* path, TF_Status* status) {
                auto res = static_cast<Filesystem*>(plugin_context)
                               ->recursively_create_dir(ice::String::create(path));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            [](void* plugin_context, const TF_TString* path, TF_Status* status) {
                auto res = static_cast<Filesystem*>(plugin_context)
                               ->delete_file(ice::String::create(path));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            [](void* plugin_context, const TF_TString* path, TF_Status* status) {
                auto res =
                    static_cast<Filesystem*>(plugin_context)->delete_dir(ice::String::create(path));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            [](void* plugin_context, const TF_TString* path, uint64_t* undeleted_files,
               uint64_t* undeleted_dirs, TF_Status* status) {
                auto res = static_cast<Filesystem*>(plugin_context)
                               ->delete_recursively(
                                   ice::String::create(path), undeleted_files, undeleted_dirs
                               );
                if (!res) {
                    res.error().to_c(status);
                }
            },
            [](void* plugin_context, const TF_TString* src, const TF_TString* dst,
               TF_Status* status) {
                auto res = static_cast<Filesystem*>(plugin_context)
                               ->rename_file(ice::String::create(src), ice::String::create(dst));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            [](void* plugin_context, const TF_TString* src, const TF_TString* dst,
               TF_Status* status) {
                auto res = static_cast<Filesystem*>(plugin_context)
                               ->copy_file(ice::String::create(src), ice::String::create(dst));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            [](void* plugin_context, const TF_TString* path, TF_Status* status) {
                auto res = static_cast<Filesystem*>(plugin_context)
                               ->path_exists(ice::String::create(path));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            // paths_exist — both tensors passed as ice::TensorHandle via the C ABI boundary.
            [](void* plugin_context, const TF_Tensor_Handle* paths, TF_Tensor_Handle* out_statuses,
               TF_Status* /*status*/) {
                auto* self = static_cast<Filesystem*>(plugin_context);
                ice::TensorHandle paths_h{paths};
                ice::TensorHandle stats_h{out_statuses};
                int n =
                    self->m_tensor_runtime.get_num_dims(paths_h.get_handle()) > 0
                        ? static_cast<int>(self->m_tensor_runtime.get_dim(paths_h.get_handle(), 0))
                        : 0;
                auto* path_strs = static_cast<const TF_TString*>(
                    self->m_tensor_runtime.get_data(paths_h.get_handle())
                );
                auto* stat_ptrs =
                    static_cast<TF_Status**>(self->m_tensor_runtime.get_data(stats_h.get_handle()));
                std::vector<ice::String> str_paths;
                str_paths.reserve(n);
                for (int i = 0; i < n; ++i) {
                    str_paths.push_back(ice::String::create(&path_strs[i]));
                }
                auto results = self->paths_exist(str_paths);
                for (int i = 0; i < n && i < static_cast<int>(results.size()); ++i) {
                    if (!results[i] && stat_ptrs && stat_ptrs[i]) {
                        results[i].error().to_c(stat_ptrs[i]);
                    }
                }
            },
            [](void* plugin_context, const TF_TString* path, TF_FileStatistics* out_stats,
               TF_Status* status) {
                auto res = static_cast<Filesystem*>(plugin_context)
                               ->stat(ice::String::create(path), out_stats);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            [](void* plugin_context, const TF_TString* path, TF_Status* status) -> bool {
                auto res = static_cast<Filesystem*>(plugin_context)
                               ->is_directory(ice::String::create(path));
                if (!res) {
                    res.error().to_c(status);
                    return false;
                }
                return res.value();
            },
            [](void* plugin_context, const TF_TString* path, TF_Status* status) -> int64_t {
                auto res = static_cast<Filesystem*>(plugin_context)
                               ->get_file_size(ice::String::create(path));
                if (!res) {
                    res.error().to_c(status);
                    return -1;
                }
                return res.value();
            },
            [](void* plugin_context, const TF_TString* uri, TF_String* out) {
                static_cast<Filesystem*>(plugin_context)
                    ->translate_name(ice::String::create(uri))
                    .to_c(out);
            },

            // get_children — returns ice::TensorHandle unwrapped to TF_Tensor_Handle*.
            [](void* plugin_context, const TF_TString* path,
               TF_Status* status) -> TF_Tensor_Handle* {
                auto* self = static_cast<Filesystem*>(plugin_context);
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
            [](void* plugin_context, const TF_TString* glob,
               TF_Status* status) -> TF_Tensor_Handle* {
                auto* self = static_cast<Filesystem*>(plugin_context);
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

            [](void* plugin_context) {
                static_cast<Filesystem*>(plugin_context)->flush_caches();
            },

            // get_filesystem_configuration — tensor of raw TF_Filesystem_Option structs.
            [](void* plugin_context, TF_Status* status) -> TF_Tensor_Handle* {
                auto* self = static_cast<Filesystem*>(plugin_context);
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
            [](void* plugin_context, const TF_Tensor_Handle* options_handle, TF_Status* status) {
                auto* self = static_cast<Filesystem*>(plugin_context);
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

            [](void* plugin_context, const TF_TString* key, TF_Filesystem_Option* out_option,
               TF_Status* status) {
                auto res = static_cast<Filesystem*>(plugin_context)
                               ->get_filesystem_configuration_option(ice::String::create(key));
                if (!res) {
                    res.error().to_c(status);
                    return;
                }
                *out_option = res.value();
            },

            [](void* plugin_context, const TF_Filesystem_Option* option, TF_Status* status) {
                auto res = static_cast<Filesystem*>(plugin_context)
                               ->set_filesystem_configuration_option(*option);
                if (!res) {
                    res.error().to_c(status);
                }
            },

            // get_filesystem_configuration_keys — same string-tensor contract as get_children.
            [](void* plugin_context, TF_Status* status) -> TF_Tensor_Handle* {
                auto* self = static_cast<Filesystem*>(plugin_context);
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
