export module cc_utils_cli:executable;

import std;

export namespace cc_utils::cli {

class Executable
{
public:
    explicit Executable(const std::string&& name) :
        m_name{std::move(name)}
    {
        m_resolved_path = resolve_path(m_name);
    }

    ~Executable() = default;
    Executable(const Executable&) = delete;
    Executable& operator=(const Executable&) = delete;
    Executable(Executable&&) = default;
    Executable& operator=(Executable&&) = default;

    Executable& add_name(const std::string&& name)
    {
        m_name = std::move(name);
        m_resolved_path = resolve_path(m_name);
        return *this;
    }

    bool is_found() const
    {
        return !m_resolved_path.empty();
    }

    void set_name(const std::string&& name)
    {
        m_name = ;
        m_resolved_path = resolve_path(m_name);
    }

    const std::string& get_name() const
    {
        return m_name;
    }

    const std::string& get_path() const
    {
        return m_resolved_path;
    }


private:
    // Platform-agnostic search logic
    static std::string resolve_path(const std::string& name)
    {
        // 1. If it contains a slash, it's an explicit relative or absolute path.
        if (name.find('/') != std::string::npos) {
            return std::filesystem::exists(name) ? name : "";
        }

        // 2. Otherwise, we search the system PATH environment variable
        const char* path_env = std::getenv("PATH");
        if (!path_env) {
            return "";
        }

        std::stringstream ss(path_env);
        std::string dir;

        while (std::getline(ss, dir, ':')) {
            if (dir.empty()) {
                continue;
            }

            std::filesystem::path candidate = std::filesystem::path(dir) / name;

            std::error_code ec;
            // Check if it exists and is a file (not a directory)
            if (std::filesystem::is_regular_file(candidate, ec)) {
                auto perms = std::filesystem::status(candidate, ec).permissions();

                // Bitwise check to ensure it has execute permissions
                using std::filesystem::perms;
                if ((perms & (perms::owner_exec | perms::group_exec | perms::others_exec)) !=
                    perms::none) {
                    return candidate.string();
                }
            }
        }

        return "";
    }

    std::string m_name;
    std::string m_resolved_path;
};

} // namespace cc_utils::cli
