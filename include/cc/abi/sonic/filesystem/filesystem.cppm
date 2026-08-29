export module cc_abi_sonic_filesystem;

export import :leaves;

module;

#include "c/extern/filesystem/filesystem.h"



import std;
import cc_abi_sonic_intern;
import cc_abi_primitives;
import cc_abi_sonic_registration;
import :leaves;

export namespace ice::sonic {

// Runtime — the mainframe-facing filesystem handle, one per URI scheme. Same
// in-process/cross-plugin duality as ice::sonic::Cache and
// ice::sonic::Generator.
class Filesystem : public ice::sonic::Runtime<Filesystem, TF_Filesystem, /*PassNameToFactory=*/true>
{
public:
    static constexpr std::string_view domain_name = "filesystem";

    std::expected<std::unique_ptr<ice::builder::RandomAccessFile>, ice::Status>
    new_random_access_file(const ice::String& path)
    {


        ice::Status status;
        void* handle =
            this->m_ops->new_random_access_file(this->get_handle(), path.get_handle(), status.get_handle());
        if (!status.ok()) {
            if (handle) {
                this->m_ops->random_access_file__destroy(handle);
            }
            return std::unexpected{status};
        }
        return std::make_unique<RandomAccessFileRuntime>(this->m_ops, handle);
    }

    std::expected<std::unique_ptr<ice::builder::WritableFile>, ice::Status>
    new_writable_file(const ice::String& path)
    {


        ice::Status status;
        void* handle =
            this->m_ops->new_writable_file(this->get_handle(), path.get_handle(), status.get_handle());
        if (!status.ok()) {
            if (handle) {
                this->m_ops->writable_file__destroy(handle);
            }
            return std::unexpected{status};
        }
        return std::make_unique<WritableFileRuntime>(this->m_ops, handle);
    }

    std::expected<std::unique_ptr<ice::builder::WritableFile>, ice::Status>
    new_appendable_file(const ice::String& path)
    {


        ice::Status status;
        void* handle =
            this->m_ops->new_appendable_file(this->get_handle(), path.get_handle(), status.get_handle());
        if (!status.ok()) {
            if (handle) {
                this->m_ops->writable_file__destroy(handle);
            }
            return std::unexpected{status};
        }
        return std::make_unique<WritableFileRuntime>(this->m_ops, handle);
    }

    std::expected<std::unique_ptr<ice::builder::ReadOnlyMemoryRegion>, ice::Status>
    new_read_only_memory_region_from_file(const ice::String& path)
    {


        ice::Status status;
        void* handle = this->m_ops->new_read_only_memory_region_from_file(this->get_handle(), path.get_handle(), status.get_handle()
        );
        if (!status.ok()) {
            if (handle) {
                this->m_ops->read_only_memory_region__destroy(handle);
            }
            return std::unexpected{status};
        }
        return std::make_unique<ReadOnlyMemoryRegionRuntime>(this->m_ops, handle);
    }

    std::expected<void, ice::Status> create_dir(const ice::String& path
    )
    {


        ice::Status status;
        this->m_ops->create_dir(this->get_handle(), path.get_handle(), status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    std::expected<void, ice::Status>
    recursively_create_dir(const ice::String& path)
    {


        ice::Status status;
        this->m_ops->recursively_create_dir(this->get_handle(), path.get_handle(), status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    std::expected<void, ice::Status> delete_file(const ice::String& path
    )
    {


        ice::Status status;
        this->m_ops->delete_file(this->get_handle(), path.get_handle(), status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    std::expected<void, ice::Status> delete_dir(const ice::String& path
    )
    {


        ice::Status status;
        this->m_ops->delete_dir(this->get_handle(), path.get_handle(), status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    std::expected<void, ice::Status> delete_recursively(
        const ice::String& path, std::uint64_t* undeleted_files,
        std::uint64_t* undeleted_dirs
    )
    {


        ice::Status status;
        this->m_ops->delete_recursively(this->get_handle(), path.get_handle(), undeleted_files, undeleted_dirs,
            status.get_handle()
        );
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    std::expected<void, ice::Status>
    rename_file(const ice::String& src, const ice::String& dst)
    {


        ice::Status status;
        this->m_ops->rename_file(this->get_handle(), src.get_handle(), dst.get_handle(), status.get_handle()
        );
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    std::expected<void, ice::Status>
    copy_file(const ice::String& src, const ice::String& dst)
    {


        ice::Status status;
        this->m_ops->copy_file(this->get_handle(), src.get_handle(), dst.get_handle(), status.get_handle()
        );
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    std::expected<void, ice::Status> path_exists(const ice::String& path
    )
    {


        ice::Status status;
        this->m_ops->path_exists(this->get_handle(), path.get_handle(), status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    std::vector<std::expected<void, ice::Status>>
    paths_exist(const std::vector<ice::String>& paths)
    {


        std::vector<const TF_TString*> raw_paths;
        raw_paths.reserve(paths.size());
        for (const auto& path: paths) {
            raw_paths.push_back(path.get_handle());
        }
        std::vector<ice::Status> statuses(paths.size());
        std::vector<TF_Status*> raw_statuses;
        raw_statuses.reserve(paths.size());
        for (auto& status: statuses) {
            raw_statuses.push_back(status.get_handle());
        }
        this->m_ops->paths_exist(this->get_handle(), raw_paths.data(), static_cast<int>(paths.size()), raw_statuses.data()
        );
        std::vector<std::expected<void, ice::Status>> results;
        results.reserve(paths.size());
        for (auto& status: statuses) {
            if (status.ok()) {
                results.push_back({});
            } else {
                results.push_back(std::unexpected{status});
            }
        }
        return results;
    }

    std::expected<void, ice::Status>
    stat(const ice::String& path, TF_FileStatistics* out_stats)
    {


        ice::Status status;
        this->m_ops->stat(this->get_handle(), path.get_handle(), out_stats, status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    std::expected<bool, ice::Status> is_directory(const ice::String& path
    )
    {


        ice::Status status;
        bool result = this->m_ops->is_directory(this->get_handle(), path.get_handle(), status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return result;
    }

    std::expected<std::int64_t, ice::Status> get_file_size(const ice::String& path
    )
    {


        ice::Status status;
        std::int64_t result =
            this->m_ops->get_file_size(this->get_handle(), path.get_handle(), status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return result;
    }

    ice::String translate_name(const ice::String& uri)
    {


        ice::String tf_name;
        this->m_ops->translate_name(this->get_handle(), uri.get_handle(), tf_name.get_handle());
        return std::move(tf_name);
    }

    std::expected<std::vector<ice::String>, ice::Status>
    get_children(const ice::String& path)
    {


        ice::Status status;
        char** entries = nullptr;
        int count =
            this->m_ops->get_children(this->get_handle(), path.get_handle(), &entries, status.get_handle());
        if (count < 0 || !status.ok()) {
            return std::unexpected{status};
        }
        std::vector<ice::String> result;
        result.reserve(static_cast<std::size_t>(count));
        for (int i = 0; i < count; ++i) {
            result.emplace_back(std::string{entries[i]});
        }
        this->m_ops->free_string_array(entries, count);
        return result;
    }

    std::expected<std::vector<ice::String>, ice::Status>
    get_matching_paths(const ice::String& glob)
    {


        ice::Status status;
        char** entries = nullptr;
        int count = this->m_ops->get_matching_paths(this->get_handle(), glob.get_handle(), &entries, status.get_handle()
        );
        if (count < 0 || !status.ok()) {
            return std::unexpected{status};
        }
        std::vector<ice::String> result;
        result.reserve(static_cast<std::size_t>(count));
        for (int i = 0; i < count; ++i) {
            result.emplace_back(std::string{entries[i]});
        }
        this->m_ops->free_string_array(entries, count);
        return result;
    }

    void flush_caches()
    {


        this->m_ops->flush_caches(this->get_handle());
    }

    std::expected<std::vector<TF_Filesystem_Option>, ice::Status>
    get_filesystem_configuration()
    {


        ice::Status status;
        TF_Filesystem_Option* options = nullptr;
        int count =
            this->m_ops->get_filesystem_configuration(this->get_handle(), &options, status.get_handle());
        if (count < 0 || !status.ok()) {
            return std::unexpected{status};
        }
        // TF_Filesystem_Option has no copy/RAII semantics of its own — each element's
        // name/description/value are plugin-allocated raw pointers. Copying the structs into
        // this vector aliases those same pointers, so calling TF_Filesystem_FreeOptions on the
        // original array here would leave the vector's copies dangling. Left un-freed instead
        // (a leak, not a use-after-free) — acceptable since nothing implements the cross-plugin
        // path yet; a real deep-copy would need its own allocator story this struct doesn't have.
        std::vector<TF_Filesystem_Option> result(options, options + count);
        return result;
    }

    std::expected<void, ice::Status>
    set_filesystem_configuration(const std::vector<TF_Filesystem_Option>& options)
    {


        ice::Status status;
        this->m_ops->set_filesystem_configuration(this->get_handle(), options.data(), static_cast<int>(options.size()), status.get_handle()
        );
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    std::expected<TF_Filesystem_Option, ice::Status>
    get_filesystem_configuration_option(const ice::String& key)
    {


        ice::Status status;
        TF_Filesystem_Option option{};
        this->m_ops->get_filesystem_configuration_option(this->get_handle(), key.get_handle(), &option, status.get_handle()
        );
        if (!status.ok()) {
            return std::unexpected{status};
        }
        // Same un-freed-on-purpose tradeoff as get_filesystem_configuration() above — freeing
        // here would dangle the struct being returned.
        return option;
    }

    std::expected<void, ice::Status>
    set_filesystem_configuration_option(const TF_Filesystem_Option& option)
    {


        ice::Status status;
        this->m_ops->set_filesystem_configuration_option(this->get_handle(), &option, status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    std::expected<std::vector<ice::String>, ice::Status>
    get_filesystem_configuration_keys()
    {


        ice::Status status;
        char** keys = nullptr;
        int count =
            this->m_ops->get_filesystem_configuration_keys(this->get_handle(), &keys, status.get_handle());
        if (count < 0 || !status.ok()) {
            return std::unexpected{status};
        }
        std::vector<ice::String> result;
        result.reserve(static_cast<std::size_t>(count));
        for (int i = 0; i < count; ++i) {
            result.emplace_back(std::string{keys[i]});
        }
        this->m_ops->free_string_array(keys, count);
        return result;
    }

public:
    explicit Filesystem(TF_Filesystem* ops, void* plugin_context) : Runtime(ops, plugin_context) {}
};

} // namespace ice::sonic
