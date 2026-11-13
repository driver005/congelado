module;

#include "include/c/extern/filesystem/option_types.h"
#include "include/c/extern/filesystem/tree.h"
#include "include/c/intern/file_statistics.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tensor.h"
#include "include/c/intern/tstring.h"

#include <glob.h>

export module aten_xpu_extern_filesystem:tree;

import std;
import cc_ice_extern_filesystem_builder;
import cc_ice_extern_filesystem_sonic;
import cc_ice_intern_sonic;
import aten_xpu_intern;

export namespace aten_xpu {

// Directory and file management over std::filesystem. Listings come back as TextTensor (the
// caller frees them with the tensor's delete_tensor) and the configuration is an in-memory map.
class FilesystemTree : public ice::builder::FilesystemTree
{
public:
    FilesystemTree(
        const ::TF_FileStatisticsOps* file_statistics_ops,
        const ::TF_StatusOps* status_ops,
        const ::TF_StringOps* string_ops,
        const ::TF_TensorOps* tensor_ops
    ) noexcept :
        ice::builder::FilesystemTree{file_statistics_ops, status_ops, string_ops, tensor_ops},
        m_failure{string_ops},
        m_status_ops{status_ops},
        m_string_ops{string_ops},
        m_tensor_ops{tensor_ops}
    {
    }

    ~FilesystemTree() override = default;
    FilesystemTree(const FilesystemTree&) = delete;
    FilesystemTree& operator=(const FilesystemTree&) = delete;
    FilesystemTree(FilesystemTree&&) = delete;
    FilesystemTree& operator=(FilesystemTree&&) = delete;

    void destroy() noexcept override
    {
        delete this;
    }

    // Options handed out point into the configuration map, so there is nothing to free.
    void free_options(::TFFilesystemOption* options, int num_options) noexcept override
    {
        static_cast<void>(options);
        static_cast<void>(num_options);
    }

    void create_dir(
        const ice::sonic::String& path,
        const ice::sonic::Status& out_status
    ) noexcept override
    {
        std::error_code error;
        const std::filesystem::path target{String::view_of(path)};
        if (std::filesystem::exists(target, error)) {
            m_failure.fail(out_status, ice::sonic::Code::k_already_exists, target.string());
            return;
        }

        std::filesystem::create_directory(target, error);
        report(error, target.string(), out_status);
    }

    void recursively_create_dir(
        const ice::sonic::String& path,
        const ice::sonic::Status& out_status
    ) noexcept override
    {
        std::error_code error;
        const std::filesystem::path target{String::view_of(path)};
        std::filesystem::create_directories(target, error);
        report(error, target.string(), out_status);
    }

    void delete_file(
        const ice::sonic::String& path,
        const ice::sonic::Status& out_status
    ) noexcept override
    {
        std::error_code error;
        const std::filesystem::path target{String::view_of(path)};
        if (std::filesystem::is_directory(target, error)) {
            m_failure.fail(
                out_status,
                ice::sonic::Code::k_failed_precondition,
                target.string() + " is a directory"
            );
            return;
        }

        if (!std::filesystem::remove(target, error) && !error) {
            error = std::make_error_code(std::errc::no_such_file_or_directory);
        }
        report(error, target.string(), out_status);
    }

    void delete_dir(
        const ice::sonic::String& path,
        const ice::sonic::Status& out_status
    ) noexcept override
    {
        std::error_code error;
        const std::filesystem::path target{String::view_of(path)};
        if (!std::filesystem::is_directory(target, error)) {
            m_failure.fail(
                out_status,
                ice::sonic::Code::k_failed_precondition,
                target.string() + " is not a directory"
            );
            return;
        }

        std::filesystem::remove(target, error);
        report(error, target.string(), out_status);
    }

    void delete_recursively(
        const ice::sonic::String& path,
        std::uint64_t* undeleted_files,
        std::uint64_t* undeleted_dirs,
        const ice::sonic::Status& out_status
    ) noexcept override
    {
        *undeleted_files = 0;
        *undeleted_dirs = 0;

        std::error_code error;
        const std::filesystem::path target{String::view_of(path)};
        if (!std::filesystem::exists(target, error)) {
            m_failure.fail(out_status, ice::sonic::Code::k_not_found, target.string());
            return;
        }

        std::filesystem::remove_all(target, error);
        if (!error) {
            return;
        }

        std::error_code ignored;
        for (const auto& entry: std::filesystem::recursive_directory_iterator(target, ignored)) {
            if (entry.is_directory(ignored)) {
                ++*undeleted_dirs;
            } else {
                ++*undeleted_files;
            }
        }
        report(error, target.string(), out_status);
    }

    void rename_file(
        const ice::sonic::String& src,
        const ice::sonic::String& dst,
        const ice::sonic::Status& out_status
    ) noexcept override
    {
        std::error_code error;
        const std::filesystem::path from{String::view_of(src)};
        std::filesystem::rename(from, std::filesystem::path{String::view_of(dst)}, error);
        report(error, from.string(), out_status);
    }

    void copy_file(
        const ice::sonic::String& src,
        const ice::sonic::String& dst,
        const ice::sonic::Status& out_status
    ) noexcept override
    {
        std::error_code error;
        const std::filesystem::path from{String::view_of(src)};
        std::filesystem::copy_file(
            from,
            std::filesystem::path{String::view_of(dst)},
            std::filesystem::copy_options::overwrite_existing,
            error
        );
        report(error, from.string(), out_status);
    }

    void path_exists(
        const ice::sonic::String& path,
        const ice::sonic::Status& out_status
    ) noexcept override
    {
        std::error_code error;
        const std::filesystem::path target{String::view_of(path)};
        if (!std::filesystem::exists(target, error)) {
            m_failure.fail(out_status, ice::sonic::Code::k_not_found, target.string());
        }
    }

    // The ABI hands over one String for a whole array, so only the first path can be checked.
    void paths_exist(
        const ice::sonic::String& paths,
        int num_paths,
        const ice::sonic::Status& out_status
    ) noexcept override
    {
        if (num_paths > 0) {
            path_exists(paths, out_status);
        }
    }

    void stat(
        const ice::sonic::String& path,
        const ice::sonic::FileStatistics& out_stats,
        const ice::sonic::Status& out_status
    ) noexcept override
    {
        std::error_code error;
        const std::filesystem::path target{String::view_of(path)};
        const auto status = std::filesystem::status(target, error);
        if (error || !std::filesystem::exists(status)) {
            report(
                error ? error : std::make_error_code(std::errc::no_such_file_or_directory),
                target.string(),
                out_status
            );
            return;
        }

        const bool directory = std::filesystem::is_directory(status);
        const auto size = directory ? std::uintmax_t{0} : std::filesystem::file_size(target, error);
        const auto written = std::filesystem::last_write_time(target, error);
        const auto system_time = std::chrono::clock_cast<std::chrono::system_clock>(written);
        out_stats.set_is_directory(directory ? 1 : 0);
        out_stats.set_length(static_cast<std::int64_t>(size));
        out_stats.set_mtime_nsec(
            std::chrono::duration_cast<std::chrono::nanoseconds>(system_time.time_since_epoch())
                .count()
        );
    }

    void is_directory(
        const ice::sonic::String& path,
        int& out_is_directory,
        const ice::sonic::Status& out_status
    ) noexcept override
    {
        std::error_code error;
        const std::filesystem::path target{String::view_of(path)};
        const auto status = std::filesystem::status(target, error);
        if (error || !std::filesystem::exists(status)) {
            report(
                error ? error : std::make_error_code(std::errc::no_such_file_or_directory),
                target.string(),
                out_status
            );
            return;
        }

        out_is_directory = std::filesystem::is_directory(status) ? 1 : 0;
    }

    void get_file_size(
        const ice::sonic::String& path,
        std::int64_t& out_size,
        const ice::sonic::Status& out_status
    ) noexcept override
    {
        std::error_code error;
        const std::filesystem::path target{String::view_of(path)};
        const auto size = std::filesystem::file_size(target, error);
        if (error) {
            report(error, target.string(), out_status);
            return;
        }

        out_size = static_cast<std::int64_t>(size);
    }

    // Drops a leading "file://"; every other name is already a local path.
    void translate_name(
        const ice::sonic::String& uri,
        const ice::sonic::String& out_name
    ) noexcept override
    {
        std::string_view name = String::view_of(uri);
        if (name.starts_with(k_file_scheme)) {
            name.remove_prefix(k_file_scheme.size());
        }

        out_name.copy(name);
    }

    void get_children(
        const ice::sonic::String& path,
        ::TF_Tensor** out_children,
        const ice::sonic::Status& out_status
    ) noexcept override
    {
        *out_children = nullptr;

        std::error_code error;
        const std::filesystem::path target{String::view_of(path)};
        m_scratch_lines.clear();
        for (const auto& entry: std::filesystem::directory_iterator(target, error)) {
            m_scratch_lines.push_back(entry.path().filename().string());
        }
        if (error) {
            report(error, target.string(), out_status);
            return;
        }

        std::ranges::sort(m_scratch_lines);
        *out_children = make_tensor(m_scratch_lines);
    }

    void get_matching_paths(
        const ice::sonic::String& glob_pattern,
        ::TF_Tensor** out_matches,
        const ice::sonic::Status& out_status
    ) noexcept override
    {
        *out_matches = nullptr;

        const std::string pattern{String::view_of(glob_pattern)};
        ::glob_t found{};
        const int result = ::glob(pattern.c_str(), 0, nullptr, &found);
        m_scratch_lines.clear();
        if (result == 0) {
            for (std::size_t index = 0; index < found.gl_pathc; ++index) {
                m_scratch_lines.emplace_back(found.gl_pathv[index]);
            }
        }
        ::globfree(&found);

        if (result != 0 && result != GLOB_NOMATCH) {
            m_failure.fail(out_status, ice::sonic::Code::k_internal, "glob failed for " + pattern);
            return;
        }

        *out_matches = make_tensor(m_scratch_lines);
    }

    void flush_caches() noexcept override {}

    void get_filesystem_configuration(
        ::TF_Tensor** out_config,
        const ice::sonic::Status& out_status
    ) noexcept override
    {
        static_cast<void>(out_status);

        m_scratch_lines.clear();
        for (const auto& [name, option]: m_options) {
            m_scratch_lines.push_back(name + "=" + option.render());
        }
        *out_config = make_tensor(m_scratch_lines);
    }

    // The tensor holds "name=value" lines; every value is stored as a buffer option.
    void set_filesystem_configuration(
        const ice::sonic::Tensor& options,
        const ice::sonic::Status& out_status
    ) noexcept override
    {
        for (const std::string& line: TextTensor::lines_of(options)) {
            const std::size_t split = line.find('=');
            if (split == std::string::npos) {
                m_failure.fail(
                    out_status,
                    ice::sonic::Code::k_invalid_argument,
                    "option without '=': " + line
                );
                return;
            }

            m_options[line.substr(0, split)].setText(line.substr(split + 1));
        }
    }

    void get_filesystem_configuration_option(
        const ice::sonic::String& key,
        ice::sonic::FilesystemOption* out_option,
        const ice::sonic::Status& out_status
    ) noexcept override
    {
        const auto found = m_options.find(std::string{String::view_of(key)});
        if (found == m_options.end()) {
            m_failure.fail(out_status, ice::sonic::Code::k_not_found, "no such option");
            return;
        }

        found->second.fill(found->first, *out_option);
    }

    void set_filesystem_configuration_option(
        const ice::sonic::FilesystemOption* option,
        const ice::sonic::Status& out_status
    ) noexcept override
    {
        if (option->getName() == nullptr) {
            m_failure
                .fail(out_status, ice::sonic::Code::k_invalid_argument, "option without a name");
            return;
        }

        m_options[option->getName()].assign(*option);
    }

    void get_filesystem_configuration_keys(
        ::TF_Tensor** out_keys,
        const ice::sonic::Status& out_status
    ) noexcept override
    {
        static_cast<void>(out_status);

        m_scratch_lines.clear();
        for (const auto& entry: m_options) {
            m_scratch_lines.push_back(entry.first);
        }
        *out_keys = make_tensor(m_scratch_lines);
    }

private:
    static constexpr std::string_view k_file_scheme = "file://";

    // One configuration entry: an integer, a real or a text buffer.
    class Option
    {
    public:
        void assign(const ice::sonic::FilesystemOption& option)
        {
            const auto& value = option.getValue();
            m_type = option.getTypeTag();
            m_description = option.getDescription() == nullptr ? "" : option.getDescription();
            // The value is a union: only the member the type names is valid.
            switch (m_type) {
                case TF_FILESYSTEM_OPTION_TYPE_INT:
                    m_integer = value.int_value;
                    break;
                case TF_FILESYSTEM_OPTION_TYPE_REAL:
                    m_real = value.real_value;
                    break;
                default:
                    m_text.assign(
                        value.buffer_value.data == nullptr ? "" : value.buffer_value.data,
                        value.buffer_value.data == nullptr ? 0 : value.buffer_value.size
                    );
                    break;
            }
        }

        void setText(std::string text)
        {
            m_type = TF_FILESYSTEM_OPTION_TYPE_BUFFER;
            m_text = std::move(text);
        }

        std::string render() const
        {
            switch (m_type) {
                case TF_FILESYSTEM_OPTION_TYPE_INT:
                    return std::to_string(m_integer);
                case TF_FILESYSTEM_OPTION_TYPE_REAL:
                    return std::format("{}", m_real);
                default:
                    return m_text;
            }
        }

        // The pointers refer to this entry, which lives as long as the configuration.
        void fill(const std::string& name, ice::sonic::FilesystemOption& option) const
        {
            option.setTypeTag(m_type);
            option.setName(name.c_str());
            option.setDescription(m_description.c_str());
            auto& value = option.getValue();
            value.type = m_type;
            switch (m_type) {
                case TF_FILESYSTEM_OPTION_TYPE_INT:
                    value.int_value = m_integer;
                    break;
                case TF_FILESYSTEM_OPTION_TYPE_REAL:
                    value.real_value = m_real;
                    break;
                default:
                    value.buffer_value.data = m_text.data();
                    value.buffer_value.size = m_text.size();
                    break;
            }
        }

    private:
        ::TFFilesystemOptionType m_type = TF_FILESYSTEM_OPTION_TYPE_BUFFER;
        std::string m_description;
        std::int64_t m_integer = 0;
        double m_real = 0.0;
        std::string m_text;
    };

    void report(
        const std::error_code& error,
        const std::string& context,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        if (error) {
            m_failure.fail_errno(out_status, error.value(), context);
        }
    }

    ::TF_Tensor* make_tensor(const std::vector<std::string>& lines) const
    {
        auto* tensor = new TextTensor{m_status_ops, m_string_ops, m_tensor_ops};
        tensor->assign(lines);

        return const_cast<::TF_Tensor*>(&tensor->get_handle());
    }

    Failure m_failure;
    const ::TF_StatusOps* m_status_ops;
    const ::TF_StringOps* m_string_ops;
    const ::TF_TensorOps* m_tensor_ops;
    std::vector<std::string> m_scratch_lines;
    std::map<std::string, Option> m_options;
};

} // namespace aten_xpu
