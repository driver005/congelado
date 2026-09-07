module;

#include <cstdlib>

export module cc_abi_gen:cli_runner;

import std;
import :cli_options;
import cc_abi_gen_parser;
import cc_abi_gen_generator;
import cc_abi_gen_writer;
import cc_utils_cli;

export namespace cc_abi_gen {

// Command-line entry point: `generate` (genrule's explicit form, or --pilot's manual form) and
// `check` (dry-run diff, --pilot only).
class CliRunner
{
public:
    int run(int argc, char** argv)
    {
        cc_utils::cli::Arguments arguments;
        for (int index = 0; index < argc; ++index) {
            arguments.add(std::string{argv[index]});
        }

        cc_utils::cli::Parser parser{build_command_schema()};

        auto parse_result = parser.parse(arguments);
        if (!parse_result) {
            std::cerr << parse_result.error() << "\n" << usage() << "\n";
            return 1;
        }

        CliOptions options = parse_options(parser);

        if (parser.get_command() == "generate") {
            return run_generate(options);
        }

        return run_check(options);
    }

private:
    // Every generated file lands in this C++ namespace (`ice::builder`/`ice::sonic` — see
    // helper::format_header) — matches every hand-written/checked-in file under include/cc/abi.
    static constexpr std::string_view NAMESPACE_NAME = "ice";

    std::string usage()
    {
        return "usage: cc_abi_gen generate --pilot [--out-dir <dir>] | "
               "cc_abi_gen generate --tier <builder|sonic> --domain <name> --header <path> --out "
               "<path> | "
               "cc_abi_gen check --pilot";
    }

    // Every command this binary accepts and the flags each one allows — cc_utils::cli::Parser
    // rejects an unknown command or an unrecognized `--flag` against this schema before
    // CliRunner ever sees it.
    std::vector<cc_utils::cli::CommandSchema> build_command_schema()
    {
        std::vector<cc_utils::cli::CommandSchema> commands;

        commands.emplace_back(
            std::string{"generate"},
            std::vector<
                std::string>{"pilot", "tier", "domain", "header", "out", "out-dir", "repo-root"}
        );

        commands.emplace_back(std::string{"check"}, std::vector<std::string>{"pilot", "repo-root"});

        return commands;
    }

    CliOptions parse_options(const cc_utils::cli::Parser& parser)
    {
        CliOptions options;

        options.m_pilot = parser.has_flag("pilot");

        if (auto value = parser.get_value("tier")) {
            options.m_tier = std::move(*value);
        }
        if (auto value = parser.get_value("domain")) {
            options.m_domain = std::move(*value);
        }
        if (auto value = parser.get_value("header")) {
            options.m_header = std::move(*value);
        }
        if (auto value = parser.get_value("out")) {
            options.m_out = std::move(*value);
        }
        if (auto value = parser.get_value("out-dir")) {
            options.m_out_dir = std::move(*value);
        }
        if (auto value = parser.get_value("repo-root")) {
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

    std::filesystem::path
    resolve_output_root(const CliOptions& options, const std::filesystem::path& repo_root)
    {
        if (options.m_out_dir) {
            return *options.m_out_dir;
        }

        return repo_root / "include/cc/abi";
    }

    // Pilot domains are discovered by convention, not hardcoded: any subdirectory of
    // include/c/extern/ whose name matches its own header (include/c/extern/<name>/<name>.h) is
    // one of parser::helper::DomainPaths's inputs. Keeps the pilot set in sync with the tree
    // instead of a list that silently drifts (see git history: an earlier hardcoded list named a
    // domain whose header had since moved).
    std::vector<std::string> discover_pilot_domains(const std::filesystem::path& repo_root)
    {
        std::vector<std::string> domains;

        std::error_code error;
        std::filesystem::path extern_root = repo_root / "include/c/extern";

        for (const auto& entry: std::filesystem::directory_iterator{extern_root, error}) {
            if (!entry.is_directory()) {
                continue;
            }

            std::string name = entry.path().filename().string();
            if (std::filesystem::exists(entry.path() / (name + ".h"))) {
                domains.push_back(std::move(name));
            }
        }

        std::ranges::sort(domains);

        return domains;
    }

    std::expected<std::string, std::string> render(
        generator::emitter::Builder& builder_emitter,
        generator::emitter::Sonic& sonic_emitter,
        const parser::vtable::Model& model,
        bool sonic_tier
    )
    {
        if (sonic_tier) {
            return sonic_emitter.render(model);
        }

        return builder_emitter.render(model);
    }

    // Parses every domain header discovered by discover_pilot_domains() into one shared
    // parser::Parser (and therefore one shared parser::Registry) — every domain's types stay
    // visible to every other domain's slot parameters, regardless of parse order.
    std::expected<std::vector<std::string>, std::string>
    parse_pilot_domains(parser::Parser& parser_instance, const std::filesystem::path& repo_root)
    {
        std::vector<std::string> domains = discover_pilot_domains(repo_root);
        if (domains.empty()) {
            return std::unexpected{"no pilot domains found under include/c/extern"};
        }

        for (const std::string& domain: domains) {
            parser::helper::DomainPaths input_paths{
                std::string{domain},
                std::filesystem::path{repo_root},
                std::filesystem::path{repo_root / "include/cc/abi"}
            };

            std::cerr
                << std::format("[cc_abi_gen] parsing {}\n", input_paths.get_header().string());

            auto parse_result =
                parser_instance.parse_file(input_paths.get_header(), repo_root / "include");
            if (!parse_result) {
                return std::unexpected{std::move(parse_result.error())};
            }
        }

        return domains;
    }

    int write_tier(
        writer::Writer& writer,
        const std::string& rendered,
        const std::filesystem::path& out_path,
        const std::filesystem::path& repo_root
    )
    {
        auto write_result = writer.write(rendered, out_path, repo_root);
        if (!write_result) {
            std::cerr << write_result.error() << "\n";
            return 1;
        }

        std::cerr << std::format("[cc_abi_gen] wrote {}\n", out_path.string());

        return 0;
    }

    bool diff_tier(
        writer::Writer& writer,
        const std::string& rendered,
        const std::filesystem::path& real_path,
        const std::filesystem::path& repo_root
    )
    {
        auto diff_result = writer.diff(rendered, real_path, repo_root);
        if (!diff_result) {
            std::cerr << diff_result.error() << "\n";
            return false;
        }

        if (diff_result->get_identical()) {
            std::cerr << std::format("[cc_abi_gen] up to date: {}\n", real_path.string());
        } else {
            std::cout << "--- " << real_path.string() << " differs ---\n";
            std::cout << diff_result->get_unified_diff();
        }

        return diff_result->get_identical();
    }

    // Explicit single-file mode: what the genrule wiring invokes.
    int run_generate_single(const CliOptions& options)
    {
        if (!options.m_tier || !options.m_domain || !options.m_header || !options.m_out) {
            std::cerr << usage() << "\n";
            return 1;
        }

        std::filesystem::path repo_root = resolve_repo_root(options);

        std::cerr << std::format("[cc_abi_gen] parsing {}\n", *options.m_header);

        std::optional<parser::Parser> parser_instance;

        try {
            parser_instance.emplace("clang++", *options.m_domain);
        } catch (const std::exception& e) {
            std::println("[cc_abi_gen] failed to initialize parser: {}", e.what());
            return 1;
        }

        // Use the -> operator to access the parser methods
        auto parse_result = parser_instance->parse_file(*options.m_header, repo_root / "include");
        if (!parse_result) {
            std::println("{}", parse_result.error());
            return 1;
        }

        parser::Registry& registry = parser_instance->get_registry();
        if (registry.begin() == registry.end()) {
            std::println("[cc_abi_gen] no vtable struct found in: {}", *options.m_header);
            return 1;
        }

        const parser::vtable::Model& model = registry.begin()->second;

        generator::emitter::Builder builder_emitter{registry, NAMESPACE_NAME};
        generator::emitter::Sonic sonic_emitter{registry, NAMESPACE_NAME};

        auto rendered = render(builder_emitter, sonic_emitter, model, *options.m_tier == "sonic");
        if (!rendered) {
            std::cerr << rendered.error() << "\n";
            return 1;
        }

        return write_tier(m_writer, *rendered, *options.m_out, repo_root);
    }

    int run_generate(const CliOptions& options)
    {
        if (!options.m_pilot) {
            return run_generate_single(options);
        }

        std::filesystem::path repo_root = resolve_repo_root(options);
        std::filesystem::path output_root = resolve_output_root(options, repo_root);

        parser::Parser parser_instance{"clang++", "pilot"};

        auto domains = parse_pilot_domains(parser_instance, repo_root);
        if (!domains) {
            std::cerr << domains.error() << "\n";
            return 1;
        }

        parser::Registry& registry = parser_instance.get_registry();
        generator::emitter::Builder builder_emitter{registry, NAMESPACE_NAME};
        generator::emitter::Sonic sonic_emitter{registry, NAMESPACE_NAME};

        for (const auto& [struct_name, model]: registry) {
            parser::helper::DomainPaths output_paths{
                std::string{model.get_domain_name()},
                std::filesystem::path{repo_root},
                std::filesystem::path{output_root}
            };

            for (bool sonic_tier: {false, true}) {
                auto rendered = render(builder_emitter, sonic_emitter, model, sonic_tier);
                if (!rendered) {
                    std::cerr << rendered.error() << "\n";
                    return 1;
                }

                const std::filesystem::path& out_path =
                    sonic_tier ? output_paths.get_sonic_cppm() : output_paths.get_builder_cppm();

                if (int status = write_tier(m_writer, *rendered, out_path, repo_root);
                    status != 0) {
                    return status;
                }
            }
        }

        std::cerr << std::format("[cc_abi_gen] done: {} domain(s) regenerated\n", domains->size());

        return 0;
    }

    int run_check(const CliOptions& options)
    {
        if (!options.m_pilot) {
            std::cerr << usage() << "\n";
            return 1;
        }

        std::filesystem::path repo_root = resolve_repo_root(options);

        parser::Parser parser_instance{"clang++", "pilot"};

        auto domains = parse_pilot_domains(parser_instance, repo_root);
        if (!domains) {
            std::cerr << domains.error() << "\n";
            return 1;
        }

        parser::Registry& registry = parser_instance.get_registry();
        generator::emitter::Builder builder_emitter{registry, NAMESPACE_NAME};
        generator::emitter::Sonic sonic_emitter{registry, NAMESPACE_NAME};

        bool all_identical = true;

        for (const auto& [struct_name, model]: registry) {
            parser::helper::DomainPaths real_paths{
                std::string{model.get_domain_name()},
                std::filesystem::path{repo_root},
                std::filesystem::path{repo_root / "include/cc/abi"}
            };

            for (bool sonic_tier: {false, true}) {
                auto rendered = render(builder_emitter, sonic_emitter, model, sonic_tier);
                if (!rendered) {
                    std::cerr << rendered.error() << "\n";
                    return 1;
                }

                const std::filesystem::path& real_path =
                    sonic_tier ? real_paths.get_sonic_cppm() : real_paths.get_builder_cppm();

                all_identical =
                    diff_tier(m_writer, *rendered, real_path, repo_root) && all_identical;
            }
        }

        std::cerr << std::format(
            "[cc_abi_gen] done: {}\n",
            all_identical ? "all up to date" : "differences found"
        );

        return all_identical ? 0 : 1;
    }

    writer::Writer m_writer;
};

} // namespace cc_abi_gen
