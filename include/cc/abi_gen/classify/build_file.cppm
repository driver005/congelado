export module cc_abi_gen_classify:build_file;

import std;

export namespace cc_abi_gen::classify {

// One Bazel BUILD file holding a single module library rule.
class BuildFile
{
public:
    BuildFile() = default;

    explicit BuildFile(std::string name) :
        m_name(std::move(name))
    {
    }

    ~BuildFile() = default;

    BuildFile(const BuildFile& other) = default;
    BuildFile& operator=(const BuildFile& other) = default;
    BuildFile(BuildFile&& other) noexcept = default;
    BuildFile& operator=(BuildFile&& other) noexcept = default;

    [[nodiscard]] std::string generate_code() const
    {
        std::string code = m_banner;
        if (!code.empty()) {
            code += '\n';
        }

        code += std::format("load(\"{}\", \"{}\")\n\n", m_load_file, m_rule);
        code += std::format("{}(\n", m_rule);
        code += std::format("    name = \"{}\",\n", m_name);

        if (!m_partitions.empty()) {
            code += "    partitions = [\n";
            for (const std::string& partition: m_partitions) {
                code += std::format("        \"{}\",\n", partition);
            }
            code += "    ],\n";
        }

        code += std::format("    primary_interface = \"{}\",\n", m_primary_interface);
        code += std::format("    visibility = [\"{}\"],\n", m_visibility);

        if (!m_dependencies.empty()) {
            code += "    deps = [\n";
            for (const std::string& dependency: m_dependencies) {
                code += std::format("        \"{}\",\n", dependency);
            }
            code += "    ],\n";
        }

        code += ")\n";

        return code;
    }

    // Comment block written before the load line, each line already starting with `#`.
    BuildFile& set_banner(std::string banner)
    {
        m_banner = std::move(banner);
        return *this;
    }

    // Example: ("//bazel:build_defs.bzl", "congelado_module_library")
    BuildFile& set_load(std::string load_file, std::string rule)
    {
        m_load_file = std::move(load_file);
        m_rule = std::move(rule);
        return *this;
    }

    BuildFile& set_name(std::string name)
    {
        m_name = std::move(name);
        return *this;
    }

    // Example: status.cppm
    BuildFile& set_partition(std::string partition)
    {
        m_partitions.push_back(std::move(partition));
        return *this;
    }

    BuildFile& set_primary_interface(std::string primary_interface)
    {
        m_primary_interface = std::move(primary_interface);
        return *this;
    }

    BuildFile& set_visibility(std::string visibility)
    {
        m_visibility = std::move(visibility);
        return *this;
    }

    // Example: //include/c:tf_c_headers
    BuildFile& set_dependency(std::string dependency)
    {
        m_dependencies.push_back(std::move(dependency));
        return *this;
    }

private:
    std::string m_banner;
    std::string m_load_file{"//bazel:build_defs.bzl"};
    std::string m_rule{"congelado_module_library"};
    std::string m_name;
    std::vector<std::string> m_partitions;
    std::string m_primary_interface{"base.cppm"};
    std::string m_visibility{"//visibility:public"};
    std::vector<std::string> m_dependencies;
};

} // namespace cc_abi_gen::classify
