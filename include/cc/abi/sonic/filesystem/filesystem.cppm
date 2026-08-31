module;

#include "c/extern/filesystem/filesystem.h"

export module cc_abi_sonic_filesystem;

import std;
export import :leaves;
import cc_abi_primitives;
import cc_abi_sonic_intern;
import cc_abi_sonic_registration;

namespace {

// Tensor runtime the paths_exist adapter decodes its string-tensor input through —
// obtained lazily from the host's registered "tensor" factory
// (ice::sonic::Tensor::resolve), the same file-local pattern as stable_hlo's
// generator_tensor_runtime(). Null when the host hasn't registered a tensor factory —
// paths_exist then fails with a clear Status instead of dereferencing null.
std::unique_ptr<ice::sonic::Tensor>& tensor_runtime() noexcept
{
    static std::unique_ptr<ice::sonic::Tensor> tensor = []
    {
        auto t = ice::sonic::Tensor::resolve("tensor");
        if (!t) {
            return std::unique_ptr<ice::sonic::Tensor>{};
        }
        return std::move(*t);
    }();
    return tensor;
}

} // namespace

export namespace ice::sonic {

// Runtime — the mainframe-facing filesystem handle, one per URI scheme. Same
// in-process/cross-plugin duality as ice::sonic::Cache and
// ice::sonic::Generator. Every member is noexcept — the std::expected return is
// the only failure channel.
class Filesystem : public ice::sonic::Runtime<Filesystem, TF_Filesystem>
{
public:
    explicit Filesystem(TF_Filesystem* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "filesystem";

    [[nodiscard]] std::expected<std::unique_ptr<ice::sonic::RandomAccessFile>, ice::Status>
    create_random_access_file(const ice::String& path) noexcept
    {
        ice::Status status;
        TF_RandomAccessFile* handle = m_ops->create_random_access_file(
            get_handle(),
            path.get_handle(),
            status.get_handle()
        );
        if (!status.ok()) {
            if (handle) {
                m_ops->random_access_file_destroy(handle);
            }
            return std::unexpected{status};
        }
        return std::make_unique<ice::sonic::RandomAccessFile>(m_ops, handle);
    }

    [[nodiscard]] std::expected<std::unique_ptr<ice::sonic::WritableFile>, ice::Status>
    create_writable_file(const ice::String& path) noexcept
    {
        ice::Status status;
        TF_WritableFile* handle = m_ops->create_writable_file(
            get_handle(),
            path.get_handle(),
            status.get_handle()
        );
        if (!status.ok()) {
            if (handle) {
                m_ops->writable_file_destroy(handle);
            }
            return std::unexpected{status};
        }
        return std::make_unique<ice::sonic::WritableFile>(m_ops, handle);
    }

    [[nodiscard]] std::expected<std::unique_ptr<ice::sonic::WritableFile>, ice::Status>
    create_appendable_file(const ice::String& path) noexcept
    {
        ice::Status status;
        TF_WritableFile* handle = m_ops->create_appendable_file(
            get_handle(),
            path.get_handle(),
            status.get_handle()
        );
        if (!status.ok()) {
            if (handle) {
                m_ops->writable_file_destroy(handle);
            }
            return std::unexpected{status};
        }
        return std::make_unique<ice::sonic::WritableFile>(m_ops, handle);
    }

    [[nodiscard]] std::expected<std::unique_ptr<ice::sonic::ReadOnlyMemoryRegion>, ice::Status>
    create_read_only_memory_region_from_file(const ice::String& path) noexcept
    {
        ice::Status status;
        TF_ReadOnlyMemoryRegion* handle = m_ops->create_read_only_memory_region_from_file(
            get_handle(),
            path.get_handle(),
            status.get_handle()
        );
        if (!status.ok()) {
            if (handle) {
                m_ops->read_only_memory_region_destroy(handle);
            }
            return std::unexpected{status};
        }
        return std::make_unique<ice::sonic::ReadOnlyMemoryRegion>(m_ops, handle);
    }

    [[nodiscard]] std::expected<void, ice::Status> create_dir(const ice::String& path) noexcept
    {
        ice::Status status;
        m_ops->create_dir(get_handle(), path.get_handle(), status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    recursively_create_dir(const ice::String& path) noexcept
    {
        ice::Status status;
        m_ops->recursively_create_dir(get_handle(), path.get_handle(), status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> delete_file(const ice::String& path) noexcept
    {
        ice::Status status;
        m_ops->delete_file(get_handle(), path.get_handle(), status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> delete_dir(const ice::String& path) noexcept
    {
        ice::Status status;
        m_ops->delete_dir(get_handle(), path.get_handle(), status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> delete_recursively(
        const ice::String& path,
        ice::DeleteRecursivelyResult& out
    ) noexcept
    {
        ice::Status status;
        m_ops->delete_recursively(
            get_handle(),
            path.get_handle(),
            &out.undeleted_files,
            &out.undeleted_dirs,
            status.get_handle()
        );
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    rename_file(const ice::String& src, const ice::String& dst) noexcept
    {
        ice::Status status;
        m_ops->rename_file(get_handle(), src.get_handle(), dst.get_handle(), status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    copy_file(const ice::String& src, const ice::String& dst) noexcept
    {
        ice::Status status;
        m_ops->copy_file(get_handle(), src.get_handle(), dst.get_handle(), status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> path_exists(const ice::String& path) noexcept
    {
        ice::Status status;
        m_ops->path_exists(get_handle(), path.get_handle(), status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    // paths_exist — the caller passes a 1-D string tensor (built with the builder tier's
    // make_string_tensor); the C slot takes a contiguous (const TF_TString*, int) array,
    // so the tensor's element buffer is decoded back to that shape. The tensor must
    // outlive the call — the slot only reads from it synchronously.
    [[nodiscard]] std::expected<void, ice::Status>
    paths_exist(ice::TensorHandle paths) noexcept
    {
        auto* runtime = tensor_runtime().get();
        if (!runtime) {
            return std::unexpected{ice::Status{"no tensor runtime available"}};
        }
        auto raw = runtime->get_data_as<TF_TString>(paths.get_handle());
        ice::Status status;
        m_ops->paths_exist(
            get_handle(),
            raw.data(),
            static_cast<int>(raw.size()),
            status.get_handle()
        );
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    // Range-first on the C++ side too: the C ABI's raw TF_FileStatistics* out-param is
    // adapted into the typed stats value the caller passes in.
    [[nodiscard]] std::expected<void, ice::Status>
    stat(const ice::String& path, ice::FileStatistics& out_stats) noexcept
    {
        ice::Status status;
        m_ops->stat(get_handle(), path.get_handle(), out_stats.get_handle(), status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<bool, ice::Status> is_directory(const ice::String& path) noexcept
    {
        ice::Status status;
        bool result = m_ops->is_directory(get_handle(), path.get_handle(), status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return result;
    }

    [[nodiscard]] std::expected<std::int64_t, ice::Status>
    get_file_size(const ice::String& path) noexcept
    {
        ice::Status status;
        std::int64_t result =
            m_ops->get_file_size(get_handle(), path.get_handle(), status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return result;
    }

    ice::String translate_name(const ice::String& uri) noexcept
    {
        ice::String tf_name;
        m_ops->translate_name(get_handle(), uri.get_handle(), tf_name.get_handle());
        return tf_name;
    }

    // Children/matching-path/config tensors come back as plugin-allocated
    // TF_Tensor_Handle values — passed through opaquely, same contract as
    // ice::sonic::Generator::get_definitions().

    [[nodiscard]] std::expected<ice::TensorHandle, ice::Status>
    get_children(const ice::String& path) noexcept
    {
        ice::Status status;
        TF_Tensor_Handle* handle =
            m_ops->get_children(get_handle(), path.get_handle(), status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return ice::TensorHandle{handle};
    }

    [[nodiscard]] std::expected<ice::TensorHandle, ice::Status>
    get_matching_paths(const ice::String& glob) noexcept
    {
        ice::Status status;
        TF_Tensor_Handle* handle =
            m_ops->get_matching_paths(get_handle(), glob.get_handle(), status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return ice::TensorHandle{handle};
    }

    void flush_caches() noexcept
    {
        m_ops->flush_caches(get_handle());
    }

    [[nodiscard]] std::expected<ice::TensorHandle, ice::Status>
    get_filesystem_configuration() noexcept
    {
        ice::Status status;
        TF_Tensor_Handle* handle =
            m_ops->get_filesystem_configuration(get_handle(), status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return ice::TensorHandle{handle};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    set_filesystem_configuration(ice::TensorHandle options) noexcept
    {
        ice::Status status;
        m_ops
            ->set_filesystem_configuration(get_handle(), options.get_handle(), status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<ice::FilesystemOption, ice::Status>
    get_filesystem_configuration_option(const ice::String& key) noexcept
    {
        ice::Status status;
        TF_Filesystem_Option option{};
        m_ops->get_filesystem_configuration_option(
            get_handle(),
            key.get_handle(),
            &option,
            status.get_handle()
        );
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return ice::FilesystemOption{option};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    set_filesystem_configuration_option(const ice::FilesystemOption& option) noexcept
    {
        ice::Status status;
        m_ops->set_filesystem_configuration_option(get_handle(), option.get_handle(), status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<ice::TensorHandle, ice::Status>
    get_filesystem_configuration_keys() noexcept
    {
        ice::Status status;
        TF_Tensor_Handle* handle =
            m_ops->get_filesystem_configuration_keys(get_handle(), status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return ice::TensorHandle{handle};
    }

    ice::String get_name() const noexcept
    {
        ice::String out;
        m_ops->get_name(get_handle(), out.get_handle());
        return out;
    }
};

} // namespace ice::sonic
