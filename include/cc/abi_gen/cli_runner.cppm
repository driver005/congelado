module;

#include <cstdio>

export module cc_abi_gen:cli_runner;

import std;
import :cli_options;
import cc_abi_gen_parser;
import cc_abi_gen_generator;
import cc_abi_gen_writer;
import cc_utils_cli;
import cc_utils_cli_parser;

export namespace cc_abi_gen {

// Command-line entry point: `generate` (genrule's explicit form, or --pilot's manual form) and `check` (dry-run diff, --pilot only).
class CliRunner
{
public:
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

        auto current_cmd_opt = parser.get_invocation().current_command();
        if (!current_cmd_opt) {
            std::println(stderr, "{}", usage());
            return 1;
        }

        if (current_cmd_opt->get().get_name() == "generate") {
            return run_generate(options);
        }

        return run_check(options);
    }

private:
    // Every generated file lands in this C++ namespace (`ice::builder`/`ice::sonic` — see helper::format_header) — matches every hand-written/checked-in file under include/cc/abi.
    static constexpr std::string_view NAMESPACE_NAME = "ice";

    std::string usage()
    {
        return "usage: cc_abi_gen generate --pilot [--out-dir <dir>] | "
               "cc_abi_gen generate --tier <builder|sonic> --domain <name> --header <path> --out "
               "<path> | "
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

    std::filesystem::path
    resolve_output_root(const CliOptions& options, const std::filesystem::path& repo_root)
    {
        if (options.m_out_dir) {
            return *options.m_out_dir;
        }

        return repo_root / "include/cc/abi";
    }

    // Pilot domains are discovered by convention, not hardcoded: any subdirectory of include/c/extern/ whose name matches its own header (include/c/extern/<name>/<name>.h) is one of parser::helper::DomainPaths's inputs. Keeps the pilot set in sync with the tree instead of a list that silently drifts (see git history: an earlier hardcoded list named a domain whose header had since moved).
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

    // Intern headers are parsed into the shared registry for cross-reference resolution only — they never drive their own generation (see parse_pilot_domains); the wrapper classes for these types are hand-written under include/cc/abi/primitives.
    std::vector<std::filesystem::path> discover_intern_headers(const std::filesystem::path& repo_root)
    {
        std::vector<std::filesystem::path> headers;

        std::error_code error;
        std::filesystem::path intern_root = repo_root / "include/c/intern";

        for (const auto& entry: std::filesystem::directory_iterator{intern_root, error}) {
            if (!entry.is_regular_file()) {
                continue;
            }

            if (entry.path().extension() == ".h") {
                headers.push_back(entry.path());
            }
        }

        std::ranges::sort(headers);

        return headers;
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

    // Parses every domain header discovered by discover_pilot_domains() into one shared parser::Parser (and therefore one shared parser::Registry) — every domain's types stay visible to every other domain's slot parameters, regardless of parse order.
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
                std::filesystem::path{repo_root / "include/cc/abi"},
                true
            };

            std::println(stderr, "[cc_abi_gen] parsing {}", input_paths.get_header().string());

            auto parse_result =
                parser_instance.parse_file(input_paths.get_header(), repo_root / "include");
            if (!parse_result) {
                return std::unexpected{std::move(parse_result.error())};
            }
        }

        // Intern headers register their types (e.g. TF_String, TF_Status, TF_Array) into the same registry so extern domains can cross-reference them, but they are not part of `domains` returned below — intern types get no generated wrapper of their own.
        for (const std::filesystem::path& header: discover_intern_headers(repo_root)) {
            std::println(stderr, "[cc_abi_gen] parsing {}", header.string());

            auto parse_result = parser_instance.parse_file(header, repo_root / "include");
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
            std::println(stderr, "{}", write_result.error());
            return 1;
        }

        std::println(stderr, "[cc_abi_gen] wrote {}", out_path.string());

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
            std::println(stderr, "{}", diff_result.error());
            return false;
        }

        if (diff_result->get_identical()) {
            std::println(stderr, "[cc_abi_gen] up to date: {}", real_path.string());
        } else {
            std::println("--- {} differs ---", real_path.string());
            std::print("{}", diff_result->get_unified_diff());
        }

        return diff_result->get_identical();
    }

    // Explicit single-file mode: what the genrule wiring invokes.
    int run_generate_single(const CliOptions& options)
    {
        if (!options.m_tier || !options.m_domain || !options.m_header || !options.m_out) {
            std::println(stderr, "{}", usage());
            return 1;
        }

        std::filesystem::path repo_root = resolve_repo_root(options);

        std::println(stderr, "[cc_abi_gen] parsing {}", *options.m_header);

        std::optional<parser::Parser> parser_instance;

        try {
            parser_instance.emplace("clang++", *options.m_domain);
        } catch (const std::exception& e) {
            std::println(stderr, "[cc_abi_gen] failed to initialize parser: {}", e.what());
            return 1;
        }

        // Use the -> operator to access the parser methods
        auto parse_result = parser_instance->parse_file(*options.m_header, repo_root / "include");
        if (!parse_result) {
            std::println(stderr, "{}", parse_result.error());
            return 1;
        }

        const parser::Registry& registry = parser_instance->get_registry();
        if (registry.begin() == registry.end()) {
            std::println(stderr, "[cc_abi_gen] no vtable struct found in: {}", *options.m_header);
            return 1;
        }

        const parser::vtable::Model& model = registry.begin()->second;

        generator::emitter::Builder builder_emitter{
            registry,
            std::string{NAMESPACE_NAME},
            repo_root
        };
        generator::emitter::Sonic sonic_emitter{registry, std::string{NAMESPACE_NAME}, repo_root};

        auto rendered = render(builder_emitter, sonic_emitter, model, *options.m_tier == "sonic");
        if (!rendered) {
            std::println(stderr, "{}", rendered.error());
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
            std::println(stderr, "{}", domains.error());
            return 1;
        }

        const parser::Registry& registry = parser_instance.get_registry();
        generator::emitter::Builder builder_emitter{
            registry,
            std::string{NAMESPACE_NAME},
            repo_root
        };
        generator::emitter::Sonic sonic_emitter{registry, std::string{NAMESPACE_NAME}, repo_root};

        std::vector<std::string> succeeded;

        for (const auto& [struct_name, model]: registry) {
            // Intern types are parsed for cross-reference resolution only — see discover_intern_headers's doc comment. They never get a generated wrapper of their own; only the pilot-discovered extern domains do.
            if (!std::ranges::binary_search(*domains, model.get_domain_name())) {
                continue;
            }

            parser::helper::DomainPaths output_paths{
                std::string{model.get_domain_name()},
                std::filesystem::path{repo_root},
                std::filesystem::path{output_root},
                true
            };

            bool domain_ok = true;
            std::string domain_error;

            for (bool sonic_tier: {false, true}) {
                auto rendered = render(builder_emitter, sonic_emitter, model, sonic_tier);
                if (!rendered) {
                    domain_ok = false;
                    domain_error = std::move(rendered.error());
                    break;
                }

                const std::filesystem::path& out_path =
                    sonic_tier ? output_paths.get_sonic_cppm() : output_paths.get_builder_cppm();

                if (int status = write_tier(m_writer, *rendered, out_path, repo_root);
                    status != 0) {
                    domain_ok = false;
                    domain_error = "write failed: " + out_path.string();
                    break;
                }
            }

            if (domain_ok) {
                succeeded.emplace_back(model.get_domain_name());
            } else {
                std::error_code error;
                std::filesystem::remove_all(output_root / model.get_domain_name(), error);

                std::println(
                    stderr,
                    "[cc_abi_gen] FAILED {}: {}",
                    model.get_domain_name(),
                    domain_error
                );
                std::abort();
            }
        }

        std::println(stderr, "[cc_abi_gen] done: {} succeeded", succeeded.size());
        if (!succeeded.empty()) {
            std::println(stderr, "[cc_abi_gen]   succeeded: {}", join(succeeded));
        }

        return 0;
    }

    std::string join(const std::vector<std::string>& names)
    {
        std::string result;

        for (std::size_t index = 0; index < names.size(); ++index) {
            if (index != 0) {
                result += ", ";
            }
            result += names[index];
        }

        return result;
    }

    int run_check(const CliOptions& options)
    {
        if (!options.m_pilot) {
            std::println(stderr, "{}", usage());
            return 1;
        }

        std::filesystem::path repo_root = resolve_repo_root(options);

        parser::Parser parser_instance{"clang++", "pilot"};

        auto domains = parse_pilot_domains(parser_instance, repo_root);
        if (!domains) {
            std::println(stderr, "{}", domains.error());
            return 1;
        }

        const parser::Registry& registry = parser_instance.get_registry();
        generator::emitter::Builder builder_emitter{
            registry,
            std::string{NAMESPACE_NAME},
            repo_root
        };
        generator::emitter::Sonic sonic_emitter{registry, std::string{NAMESPACE_NAME}, repo_root};

        bool all_identical = true;

        for (const auto& [struct_name, model]: registry) {
            // Intern types are parsed for cross-reference resolution only — see discover_intern_headers's doc comment. They never get a generated wrapper of their own; only the pilot-discovered extern domains do.
            if (!std::ranges::binary_search(*domains, model.get_domain_name())) {
                continue;
            }

            parser::helper::DomainPaths real_paths{
                std::string{model.get_domain_name()},
                std::filesystem::path{repo_root},
                std::filesystem::path{repo_root / "include/cc/abi"},
                true
            };

            for (bool sonic_tier: {false, true}) {
                auto rendered = render(builder_emitter, sonic_emitter, model, sonic_tier);
                if (!rendered) {
                    std::println(stderr, "{}", rendered.error());
                    return 1;
                }

                const std::filesystem::path& real_path =
                    sonic_tier ? real_paths.get_sonic_cppm() : real_paths.get_builder_cppm();

                all_identical =
                    diff_tier(m_writer, *rendered, real_path, repo_root) && all_identical;
            }
        }

        std::println(
            stderr,
            "[cc_abi_gen] done: {}",
            all_identical ? "all up to date" : "differences found"
        );

        return all_identical ? 0 : 1;
    }

    writer::Writer m_writer;
};

} // namespace cc_abi_gen
