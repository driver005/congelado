module;

#include <cstdio>
#include <string_view>

export module cc_abi_gen:cli_runner;

import std;
import :cli_options;
import cc_abi_gen_parser;
import cc_abi_gen_generator;
import cc_abi_gen_writer;
import cc_utils_cli;
import cc_utils_cli_parser;

void replace_all(std::string& source, std::string_view from, std::string_view to)
{
    if (from.empty()) {
        return;
    }
    size_t start_pos = 0;
    while ((start_pos = source.find(from, start_pos)) != std::string::npos) {
        source.replace(start_pos, from.length(), to);
        start_pos += to.length(); // Advance past the replaced segment
    }
}

export namespace cc_abi_gen {

// Command-line entry point: `generate` (genrule's explicit form, or --pilot's manual form) and
// `check` (dry-run diff, --pilot only).
class CliRunner
{
public:
    CliRunner()
    {
        m_runtime.overwrite_path_callback(
            [](std::filesystem::path& out_path)
            {
                constexpr std::array<std::string_view, 2> target_sequence{"include", "c"};

                std::filesystem::path new_path;

                // Create a view over the remaining path components to process
                auto current_view = std::ranges::subrange(out_path.begin(), out_path.end());

                while (true) {
                    // Range-first: Search for {"include", "c"} inside the current path view
                    auto match = std::ranges::search(current_view, target_sequence);

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

                    // 2. Append our replacement
                    new_path /= "abi";

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

        m_runtime.set_repo_root(std::move(repo_root));
        m_runtime.set_output_dir(std::move(output_dir));

        auto current_cmd_opt = parser.get_invocation().current_command();
        if (!current_cmd_opt) {
            std::println(stderr, "{}", usage());
            return 1;
        }

        if (current_cmd_opt->get().get_name() == "generate") {
            return run_generate();
        }

        return run_check();
    }

private:
    // Every generated file lands in this C++ namespace (`ice::builder`/`ice::sonic` — see
    // helper::format_header) — matches every hand-written/checked-in file under include/cc/abi.
    static constexpr std::string_view NAMESPACE_NAME = "ice";

    std::string usage()
    {
        return "usage: cc_abi_gen generate --pilot [--out-dir <dir>] | "
               "cc_abi_gen generate --tier <builder|sonic|both> --domain <name> --header "
               "<path> "
               "[--out <path>] | "
               "cc_abi_gen check --pilot";
    }

    // Every command this binary accepts and the flags each one allows
    cc_utils::cli::parser::Schema build_command_schema()
    {
        using namespace cc_utils::cli::parser;

        Schema schema{"cc_abi_gen"};

        schema.add_option(
            std::move(
                Option{"generate", "Generate ABI"}
                    .add_flag(Flag{"pilot", "Pilot mode", FlagType::Boolean, {}})
                    .add_flag(Flag{"tier", "Tier", FlagType::String, {}})
                    .add_flag(Flag{"domain", "Domain name", FlagType::String, {}})
                    .add_flag(Flag{"header", "Header path", FlagType::String, {}})
                    .add_flag(Flag{"out", "Output path", FlagType::String, {}})
                    .add_flag(Flag{"out-dir", "Output directory", FlagType::String, {}})
                    .add_flag(Flag{"repo-root", "Repo root", FlagType::String, {}})
            )
        );

        schema.add_option(
            std::move(
                Option{"check", "Check ABI diff"}
                    .add_flag(Flag{"pilot", "Pilot mode", FlagType::Boolean, {}})
                    .add_flag(Flag{"repo-root", "Repo root", FlagType::String, {}})
            )
        );

        return schema;
    }

    CliOptions parse_options(const cc_utils::cli::Parser& parser)
    {
        CliOptions options;

        auto current_cmd_opt = parser.get_invocation().current_command();
        if (!current_cmd_opt) {
            return options;
        }

        const auto& current_cmd = current_cmd_opt->get();

        auto has_flag = [&current_cmd](std::string_view name) -> bool
        {
            for (const auto& flag: current_cmd.get_flags()) {
                if (flag.get_name() == name) {
                    return true;
                }
            }
            return false;
        };

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

        options.m_pilot = has_flag("pilot");

        if (auto value = get_value("tier")) {
            options.m_tier = std::move(*value);
        }
        if (auto value = get_value("domain")) {
            options.m_domain = std::move(*value);
        }
        if (auto value = get_value("header")) {
            options.m_header = std::move(*value);
        }
        if (auto value = get_value("out")) {
            options.m_out = std::move(*value);
        }
        if (auto value = get_value("out-dir")) {
            options.m_out_dir = std::move(*value);
        }
        if (auto value = get_value("repo-root")) {
            options.m_repo_root = std::move(*value);
        }

        return options;
    }

    std::filesystem::path resolve_repo_root(const CliOptions& options)
    {
        if (options.m_repo_root) {
            return *options.m_repo_root;
        }

        if (const char* workspace_directory = std::getenv("BUILD_WORKSPACE_DIRECTORY")) {
            return workspace_directory;
        }

        return std::filesystem::current_path();
    }

    std::filesystem::path resolve_output_dir(const CliOptions& options)
    {
        if (options.m_out_dir) {
            return *options.m_out_dir;
        }

        return "include/cc/";
    }

    // Explicit single-file mode: what the genrule wiring invokes.
    int run_generate_single(const CliOptions& options)
    {
        if (!options.m_tier || !options.m_domain || !options.m_header) {
            std::println(stderr, "{}", usage());
            return 1;
        }
        std::println(stderr, "[cc_abi_gen] parsing {}", *options.m_header);

        auto parsed = m_runtime.parse_directory("include/c");
        if (!parsed) {
            std::println(stderr, "{}", parsed.error());
            return 1;
        }

        // Parse tier using enum's from_string method
        auto mode = generator::emitter::from_string(*options.m_tier);
        if (!mode) {
            std::println(stderr, "invalid tier: {} (expected builder|sonic|both)", *options.m_tier);
            return 1;
        }

        const auto& registry = m_runtime.get_registry();
        auto model = registry.find(*options.m_header);
        if (!model.has_value()) {
            std::println(stderr, "[cc_abi_gen] no vtable struct found in: {}", *options.m_header);
            return 1;
        }

        auto gen_result = m_runtime.generate_single(*model, *mode);
        if (!gen_result) {
            std::println(stderr, "{}", gen_result.error());
            return 1;
        }

        if (*mode == generator::emitter::Mode::Both) {
            std::println(stderr, "[cc_abi_gen] wrote both layers");
        } else {
            std::println(stderr, "[cc_abi_gen] wrote {}", *options.m_out);
        }

        return 0;
    }

    int run_generate()
    {
        // Parse extern and intern directories separately (not the root include/c)
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

    int run_check_single(const CliOptions& options)
    {
        if (!options.m_tier || !options.m_domain || !options.m_header) {
            std::println(stderr, "{}", usage());
            return 1;
        }
        std::println(stderr, "[cc_abi_gen] parsing {}", *options.m_header);

        auto parsed = m_runtime.parse_directory("include/c");
        if (!parsed) {
            std::println(stderr, "{}", parsed.error());
            return 1;
        }

        // Parse tier using enum's from_string method
        auto mode = generator::emitter::from_string(*options.m_tier);
        if (!mode) {
            std::println(stderr, "invalid tier: {} (expected builder|sonic|both)", *options.m_tier);
            return 1;
        }

        const auto& registry = m_runtime.get_registry();
        auto model = registry.find(*options.m_header);
        if (!model.has_value()) {
            std::println(stderr, "[cc_abi_gen] no vtable struct found in: {}", *options.m_header);
            return 1;
        }

        auto check_result = m_runtime.check_single(*model, *mode);
        if (!check_result) {
            std::println(stderr, "{}", check_result.error());
            return 1;
        }

        if (*check_result) {
            std::println(stderr, "[cc_abi_gen] done: {} up to date", *mode);
        } else {
            std::println(stderr, "[cc_abi_gen] done: differences found in {}", *mode);
        }

        return 0;
    }

    int run_check()
    {
        auto parsed = m_runtime.parse_directory("include/c");
        if (!parsed) {
            std::println(stderr, "{}", parsed.error());
            return 1;
        }

        auto check_result = m_runtime.check();
        if (!check_result) {
            std::println(stderr, "{}", check_result.error());
            return 1;
        }

        if (*check_result) {
            std::println(stderr, "[cc_abi_gen] done: all up to date");
        } else {
            std::println(stderr, "[cc_abi_gen] done: differences found");
        }

        return *check_result ? 0 : 1;
    }

    writer::Writer m_writer;
    generator::runtime::Runtime m_runtime;
};

} // namespace cc_abi_gen
