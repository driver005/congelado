module;

#include "c/extern/filesystem/filesystem.h"

export module cc_abi_sonic_filesystem;

import std;
export import :leaves;
import cc_abi_primitives;
import cc_abi_sonic_intern;
import cc_abi_sonic_registration;

export namespace ice::sonic {

// Runtime — the mainframe-facing filesystem handle, one per URI scheme. Same
// in-process/cross-plugin duality as ice::sonic::Cache and
// ice::sonic::Generator.
class Filesystem : public ice::sonic::Runtime<Filesystem, TF_Filesystem>
{
public:
    explicit Filesystem(TF_Filesystem* ops, void* plugin_context) :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "filesystem";

    std::unique_ptr<ice::sonic::RandomAccessFile> new_random_access_file(const ice::String& path)
    {
        ice::Status status;
        void* handle =
            m_ops->new_random_access_file(get_handle(), path.get_handle(), status.get_handle());
        if (!status.ok()) {
            if (handle) {
                m_ops->random_access_file__destroy(handle);
            }
            return nullptr;
        }
        return std::make_unique<ice::sonic::RandomAccessFile>(m_ops, handle);
    }

    std::unique_ptr<ice::sonic::WritableFile> new_writable_file(const ice::String& path)
    {
        ice::Status status;
        void* handle =
            m_ops->new_writable_file(get_handle(), path.get_handle(), status.get_handle());
        if (!status.ok()) {
            if (handle) {
                m_ops->writable_file__destroy(handle);
            }
            return nullptr;
        }
        return std::make_unique<ice::sonic::WritableFile>(m_ops, handle);
    }

    std::unique_ptr<ice::sonic::WritableFile> new_appendable_file(const ice::String& path)
    {
        ice::Status status;
        void* handle =
            m_ops->new_appendable_file(get_handle(), path.get_handle(), status.get_handle());
        if (!status.ok()) {
            if (handle) {
                m_ops->writable_file__destroy(handle);
            }
            return nullptr;
        }
        return std::make_unique<ice::sonic::WritableFile>(m_ops, handle);
    }

    std::unique_ptr<ice::sonic::ReadOnlyMemoryRegion>
    new_read_only_memory_region_from_file(const ice::String& path)
    {
        ice::Status status;
        void* handle = m_ops->new_read_only_memory_region_from_file(
            get_handle(),
            path.get_handle(),
            status.get_handle()
        );
        if (!status.ok()) {
            if (handle) {
                m_ops->read_only_memory_region__destroy(handle);
            }
            return nullptr;
        }
        return std::make_unique<ice::sonic::ReadOnlyMemoryRegion>(m_ops, handle);
    }

    [[nodiscard]] std::expected<void, ice::Status> create_dir(const ice::String& path)
    {
        ice::Status status;
        m_ops->create_dir(get_handle(), path.get_handle(), status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> recursively_create_dir(const ice::String& path)
    {
        ice::Status status;
        m_ops->recursively_create_dir(get_handle(), path.get_handle(), status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> delete_file(const ice::String& path)
    {
        ice::Status status;
        m_ops->delete_file(get_handle(), path.get_handle(), status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> delete_dir(const ice::String& path)
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
        std::uint64_t* undeleted_files,
        std::uint64_t* undeleted_dirs
    )
    {
        ice::Status status;
        m_ops->delete_recursively(
            get_handle(),
            path.get_handle(),
            undeleted_files,
            undeleted_dirs,
            status.get_handle()
        );
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    rename_file(const ice::String& src, const ice::String& dst)
    {
        ice::Status status;
        m_ops->rename_file(get_handle(), src.get_handle(), dst.get_handle(), status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    copy_file(const ice::String& src, const ice::String& dst)
    {
        ice::Status status;
        m_ops->copy_file(get_handle(), src.get_handle(), dst.get_handle(), status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> path_exists(const ice::String& path)
    {
        ice::Status status;
        m_ops->path_exists(get_handle(), path.get_handle(), status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    paths_exist(const std::vector<ice::String>& paths)
    {
        ice::Status status;
        // The C slot takes a contiguous array of TF_TString values; the plugin only
        // reads it for the duration of the call, so copying the handles by value is
        // safe (TF_TString is a POD union — no destructor, no ownership transfer).
        std::vector<TF_TString> raw_paths;
        raw_paths.reserve(paths.size());
        for (const auto& path: paths) {
            raw_paths.push_back(*path.get_handle());
        }
        m_ops->paths_exist(
            get_handle(),
            raw_paths.data(),
            static_cast<int>(raw_paths.size()),
            status.get_handle()
        );
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    stat(const ice::String& path, TF_FileStatistics* out_stats)
    {
        ice::Status status;
        m_ops->stat(get_handle(), path.get_handle(), out_stats, status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<bool, ice::Status> is_directory(const ice::String& path)
    {
        ice::Status status;
        bool result = m_ops->is_directory(get_handle(), path.get_handle(), status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return result;
    }

    [[nodiscard]] std::expected<std::int64_t, ice::Status> get_file_size(const ice::String& path)
    {
        ice::Status status;
        std::int64_t result =
            m_ops->get_file_size(get_handle(), path.get_handle(), status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return result;
    }

    ice::String translate_name(const ice::String& uri)
    {
        ice::String tf_name;
        m_ops->translate_name(get_handle(), uri.get_handle(), tf_name.get_handle());
        return tf_name;
    }

    // Children/matching-path/config tensors come back as plugin-allocated
    // TF_Tensor_Handle values — passed through opaquely, same contract as
    // ice::sonic::Generator::get_definitions().

    [[nodiscard]] std::expected<ice::TensorHandle, ice::Status>
    get_children(const ice::String& path)
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
    get_matching_paths(const ice::String& glob)
    {
        ice::Status status;
        TF_Tensor_Handle* handle =
            m_ops->get_matching_paths(get_handle(), glob.get_handle(), status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return ice::TensorHandle{handle};
    }

    void flush_caches()
    {
        m_ops->flush_caches(get_handle());
    }

    [[nodiscard]] std::expected<ice::TensorHandle, ice::Status> get_filesystem_configuration()
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
    set_filesystem_configuration(ice::TensorHandle options)
    {
        ice::Status status;
        m_ops
            ->set_filesystem_configuration(get_handle(), options.get_handle(), status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<TF_Filesystem_Option, ice::Status>
    get_filesystem_configuration_option(const ice::String& key)
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
        return option;
    }

    [[nodiscard]] std::expected<void, ice::Status>
    set_filesystem_configuration_option(const TF_Filesystem_Option& option)
    {
        ice::Status status;
        m_ops->set_filesystem_configuration_option(get_handle(), &option, status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<ice::TensorHandle, ice::Status> get_filesystem_configuration_keys()
    {
        ice::Status status;
        TF_Tensor_Handle* handle =
            m_ops->get_filesystem_configuration_keys(get_handle(), status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return ice::TensorHandle{handle};
    }

    ice::String get_name() const
    {
        ice::String out;
        m_ops->get_name(get_handle(), out.get_handle());
        return out;
    }
};

} // namespace ice::sonic
