module;

#include <cstdio>

export module cc_abi_gen_parser:system_include_finder;

import std;

export namespace cc_abi_gen {

// Asks the real system clang++ for its default `#include <...>` search path (C and C++ system
// headers, resource dir), by parsing `clang++ -E -xc++ - -v`'s stderr output. Needed because
// buildASTFromCodeWithArgs runs the frontend directly (bypassing clang::driver::Driver), so none
// of the system/resource-dir include paths a real `clang++` invocation computes implicitly are
// added automatically.
class SystemIncludeFinder
{
public:
    const std::vector<std::string> &discover()
    {
        m_directories.clear();

        std::FILE *pipe = popen("clang++ -E -xc++ - -v < /dev/null 2>&1", "r");
        if (pipe == nullptr) {

            return m_directories;
        }

        m_scratch_output.clear();
        std::array<char, 4096> buffer{};
        std::size_t read_count = 0;
        while ((read_count = std::fread(buffer.data(), 1, buffer.size(), pipe)) > 0) {

            m_scratch_output.append(buffer.data(), read_count);
        }
        pclose(pipe);

        std::istringstream stream(m_scratch_output);
        std::string line;
        bool in_search_list = false;
        while (std::getline(stream, line)) {

            if (line.find("#include <...> search starts here") != std::string::npos) {

                in_search_list = true;
                continue;
            }

            if (line.find("End of search list.") != std::string::npos) {

                break;
            }

            if (in_search_list) {

                std::size_t start = line.find_first_not_of(' ');
                if (start != std::string::npos) {

                    m_directories.push_back(line.substr(start));
                }
            }
        }

        return m_directories;
    }

private:
    std::vector<std::string> m_directories;
    std::string m_scratch_output;
};

} // namespace cc_abi_gen
