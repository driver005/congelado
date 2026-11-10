module;

#include <cstdio>

export module cc_abi_gen:cli_runner;

import std;
import :cli_options;
import :runtime;
import cc_utils_cli;
import cc_utils_cli_parser;

export namespace cc_abi_gen {

// Command-line entry point: `generate` (genrule's explicit form, or --pilot's manual form).
class CliRunner
{
public:
    CliRunner()
    {
        m_runtime.overwrite_path_callback(
            [](std::filesystem::path& out_path)
            {
                constexpr std::array<std::string_view, 2> TARGET_SEQUENCE{"include", "c"};

                std::filesystem::path new_path;

                // Create a view over the remaining path components to process
                auto current_view = std::ranges::subrange(out_path.begin(), out_path.end());

                while (true) {
                    // Range-first: Search for {"include", "c"} inside the current path view
                    auto match = std::ranges::search(current_view, TARGET_SEQUENCE);

                    if (match.empty()) {
                        // No more matches. Append the remaining components and exit.
                        for (const auto& comp: current_view) {
                            new_path /= comp;
                        }
                        break;
                    }

                    // 1. Append everything before the match
                    for (const auto& comp:
                         std::ranges::subrange(current_view.begin(), match.begin())) {
                        new_path /= comp;
                    }

                    new_path /= FOLDER_NAME;
                    new_path /= NAMESPACE_NAME;

                    // 3. Advance the view to start immediately after the match
                    current_view = std::ranges::subrange(match.end(), current_view.end());
                }

                // Mutate the caller's path in-place
                out_path = new_path;
            }
        );
    };

    int run(int argc, char** argv)
    {
        cc_utils::cli::Parser parser{build_command_schema()};

        auto parse_result = parser.parse(argc, argv);
        if (!parse_result) {
            std::println(stderr, "{}\n{}", parse_result.error(), usage());
            return 1;
        }

        auto check_result = parser.check();
        if (!check_result) {
            std::println(stderr, "{}\n{}", check_result.error(), usage());
            return 1;
        }

        CliOptions options = parse_options(parser);

        auto repo_root = resolve_repo_root(options);
        auto output_dir = resolve_output_dir(options);
        auto folder = resolve_folder_name(options);
        auto namespace_name = resolve_namespace_name(options);

        m_runtime.set_repo_root(std::move(repo_root));
        m_runtime.set_output_dir(std::move(output_dir));
        m_runtime.set_base_folder(std::move(folder));
        m_runtime.set_namespace_name(std::move(namespace_name));

        auto current_cmd_opt = parser.get_invocation().current_command();
        if (!current_cmd_opt) {
            std::println(stderr, "{}", usage());
            return 1;
        }

        if (current_cmd_opt->get().get_name() != "generate") {
            std::println(stderr, "{}", usage());
            return 1;
        }

        return run_generate();
    }

private:
    // Every generated file lands in this C++ namespace and under include/<folder>.
    static constexpr std::string_view NAMESPACE_NAME = "ice";
    static constexpr std::string_view FOLDER_NAME = "cc";

    static std::string_view usage()
    {
        return "usage: cc_abi_gen generate --pilot [--out-dir <dir>] [--repo-root <dir>] "
               "[--folder <name>] [--namespace <name>]";
    }

    // Every command this binary accepts and the flags each one allows
    static cc_utils::cli::parser::Schema build_command_schema()
    {
        using namespace cc_utils::cli::parser;

        Schema schema{"cc_abi_gen"};

        schema.add_option(
            std::move(
                Option{"generate", "Generate ABI"}
                    .add_flag(Flag{"pilot", "Pilot mode", FlagType::Boolean, {}})
                    .add_flag(Flag{"folder", "Folder name", FlagType::String, {}})
                    .add_flag(Flag{"namespace", "Namespace name", FlagType::String, {}})
                    .add_flag(Flag{"out-dir", "Output directory", FlagType::String, {}})
                    .add_flag(Flag{"repo-root", "Repo root", FlagType::String, {}})
            )
        );

        return schema;
    }

    static CliOptions parse_options(const cc_utils::cli::Parser& parser)
    {
        CliOptions options;

        auto current_cmd_opt = parser.get_invocation().current_command();
        if (!current_cmd_opt) {
            return options;
        }

        const auto& current_cmd = current_cmd_opt->get();

        auto get_value = [&current_cmd](std::string_view name) -> std::optional<std::string>
        {
            for (const auto& flag: current_cmd.get_flags()) {
                if (flag.get_name() == name) {
                    if (flag.get_has_value()) {
                        return flag.get_value();
                    }
                }
            }
            return std::nullopt;
        };

        if (auto value = get_value("folder")) {
            options.m_folder = std::move(*value);
        }
        if (auto value = get_value("namespace")) {
            options.m_namespace_name = std::move(*value);
        }
        if (auto value = get_value("out-dir")) {
            options.m_out_dir = std::move(*value);
        }
        if (auto value = get_value("repo-root")) {
            options.m_repo_root = std::move(*value);
        }

        return options;
    }

    static std::filesystem::path resolve_repo_root(const CliOptions& options)
    {
        if (options.m_repo_root) {
            return *options.m_repo_root;
        }

        // NOLINTNEXTLINE(concurrency-mt-unsafe)
        if (const char* workspace_directory = std::getenv("BUILD_WORKSPACE_DIRECTORY")) {
            return workspace_directory;
        }

        return std::filesystem::current_path();
    }

    static std::string resolve_folder_name(const CliOptions& options)
    {
        if (options.m_folder) {
            return *options.m_folder;
        }

        return std::string{FOLDER_NAME};
    }

    static std::string resolve_namespace_name(const CliOptions& options)
    {
        if (options.m_namespace_name) {
            return *options.m_namespace_name;
        }

        return std::string{NAMESPACE_NAME};
    }

    static std::filesystem::path resolve_output_dir(const CliOptions& options)
    {
        if (options.m_out_dir) {
            return *options.m_out_dir;
        }

        return "include/";
    }

    int run_generate()
    {
        auto parsed = m_runtime.parse_directory("include/c");
        if (!parsed) {
            std::println(stderr, "{}", parsed.error());
            return 1;
        }

        auto gen_result = m_runtime.generate();
        if (!gen_result) {
            std::println(stderr, "{}", gen_result.error());
            return 1;
        }

        std::println(stderr, "[cc_abi_gen] done");

        return 0;
    }

    Runtime m_runtime{
        std::filesystem::path{},
        std::string{FOLDER_NAME},
        std::string{NAMESPACE_NAME}
    };
};

} // namespace cc_abi_gen
