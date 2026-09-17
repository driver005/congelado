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

export namespace cc_abi_gen {

// Command-line entry point: `generate` (genrule's explicit form, or --pilot's manual form) and
// `check` (dry-run diff, --pilot only).
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

    // Discovers every C header under include/c/extern/ and include/c/intern/. Each header's file
    // stem becomes its domain name (tf_ prefix stripped for intern files).
    std::vector<std::filesystem::path> discover_all_headers(const std::filesystem::path& repo_root)
    {
        std::vector<std::filesystem::path> headers;
        std::error_code error;

        // Extern: recursively find all .h files
        std::filesystem::path extern_root = repo_root / "include/c/extern";
        if (std::filesystem::exists(extern_root)) {
            for (const auto& entry:
                 std::filesystem::recursive_directory_iterator{extern_root, error}) {
                if (entry.is_regular_file() && entry.path().extension() == ".h") {
                    headers.push_back(entry.path());
                }
            }
        }

        // Intern: find all tf_*.h files
        std::filesystem::path intern_root = repo_root / "include/c/intern";
        if (std::filesystem::exists(intern_root)) {
            for (const auto& entry: std::filesystem::directory_iterator{intern_root, error}) {
                if (entry.is_regular_file() && entry.path().extension() == ".h") {
                    headers.push_back(entry.path());
                }
            }
        }

        std::ranges::sort(headers);
        return headers;
    }

    // Derives a domain name from a header file stem: "tf_string" → "string", "span" → "span",
    // "pub_sub" → "pub_sub"
    std::string domain_from_stem(const std::filesystem::path& header)
    {
        auto stem = header.stem().string();
        if (stem.starts_with("tf_")) {
            return stem.substr(3);
        }
        return stem;
    }

    // Parses every C header discovered by discover_all_headers() into one shared parser::Parser.
    // Each file's stem becomes the domain name for structs defined in that file.
    std::expected<std::vector<std::string>, std::string>
    parse_all_headers(parser::Parser& parser_instance, const std::filesystem::path& repo_root)
    {
        auto headers = discover_all_headers(repo_root);
        if (headers.empty()) {
            return std::unexpected{"no headers found under include/c/"};
        }

        for (const auto& header: headers) {
            auto parse_result = parser_instance.parse_file(header, repo_root);
            if (!parse_result) {
                // Skip files without vtable structs (e.g. option_types.h)
                std::println(stderr, "[cc_abi_gen]   skipped: {}", parse_result.error());
                continue;
            }
        }

        // Collect domain names from registry (last parse wins for each struct)
        std::vector<std::string> domains;
        const auto& registry = parser_instance.get_registry();
        for (const auto& [struct_name, model]: registry) {
            domains.push_back(model.get_domain_name());
        }
        std::ranges::sort(domains);

        return domains;
    }

    // Derives all output paths from the header path stored in the model.
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
        auto parse_result = parser_instance->parse_file(*options.m_header, repo_root);
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

        auto header_relative = std::filesystem::path{*options.m_header};
        auto include_pos = header_relative.string().find("include/");
        if (include_pos != std::string::npos) {
            header_relative = header_relative.string().substr(include_pos + 8);
        }

        auto rendered = render(
            builder_emitter,
            sonic_emitter,
            model,
            header_relative.string(),
            *options.m_tier == "sonic"
        );
        if (!rendered) {
            std::println(stderr, "{}", rendered.error());
            return 1;
        }

        return write_tier(m_writer, *rendered, *options.m_out, repo_root);
    }

    std::expected<std::string, std::string> render(
        generator::emitter::Builder& builder_emitter,
        generator::emitter::Sonic& sonic_emitter,
        const parser::vtable::Model& model,
        const std::string& header_path,
        bool sonic_tier
    )
    {
        if (sonic_tier) {
            return sonic_emitter.render(model);
        }

        return builder_emitter.render(model);
    }

    int run_generate(const CliOptions& options)
    {
        if (!options.m_pilot) {
            return run_generate_single(options);
        }

        std::filesystem::path repo_root = resolve_repo_root(options);
        std::filesystem::path output_root = resolve_output_root(options, repo_root);

        parser::Parser parser_instance{"clang++", "pilot"};

        auto domains = parser_instance.parse_directory(
            repo_root,
            "include/c",
            std::array<std::string_view, 2>{".h", ".hpp"}
        );
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
            auto tier = model.to_tier();
            if (!tier.has_value()) {
                std::println(stderr, "{}", tier.error());
                return 1;
            }
            auto file_name = model.to_file_name();
            if (!file_name.has_value()) {
                std::println(stderr, "{}", file_name.error());
                return 1;
            }

            auto domain_root = output_root / *tier / model.get_domain_name();
            auto builder = domain_root / "builder" / (std::string{*file_name} + ".cppm");
            auto sonic = domain_root / "sonic" / (std::string{*file_name} + ".cppm");

            bool domain_ok = true;
            std::string domain_error;

            for (bool sonic_tier: {false, true}) {
                auto rendered = render(
                    builder_emitter,
                    sonic_emitter,
                    model,
                    model.get_header_path(),
                    sonic_tier
                );
                if (!rendered) {
                    domain_ok = false;
                    domain_error = std::move(rendered.error());
                    break;
                }

                std::filesystem::path out_path;
                if (sonic_tier) {
                    out_path = sonic;
                } else {
                    out_path = builder;
                }

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
                std::filesystem::remove(builder, error);
                std::filesystem::remove(sonic, error);

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

        auto domains = parse_all_headers(parser_instance, repo_root);
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
            auto& hp = model.get_header_path();
            auto include_pos = hp.find("include/");
            std::string header_rel;
            if (include_pos != std::string::npos) {
                header_rel = hp.substr(include_pos + 8);
            } else {
                header_rel = hp;
            }

            constexpr auto ext = "/include/c/extern/";
            bool is_extern = hp.find(ext) != std::string::npos;
            std::string out_domain;
            if (is_extern) {
                out_domain = hp.substr(hp.find(ext) + std::string(ext).size());
            } else {
                out_domain = model.get_domain_name();
            }
            out_domain = out_domain.substr(0, out_domain.find('/'));

            std::string tier;
            if (is_extern) {
                tier = "extern";
            } else {
                tier = "intern";
            }
            auto domain_root = repo_root / "include/cc/abi" / tier / out_domain;
            auto builder =
                domain_root / "builder" / (std::string{model.get_domain_name()} + ".cppm");
            auto sonic = domain_root / "sonic" / (std::string{model.get_domain_name()} + ".cppm");

            for (bool sonic_tier: {false, true}) {
                auto rendered =
                    render(builder_emitter, sonic_emitter, model, header_rel, sonic_tier);
                if (!rendered) {
                    std::println(stderr, "{}", rendered.error());
                    return 1;
                }

                std::filesystem::path real_path;
                if (sonic_tier) {
                    real_path = sonic;
                } else {
                    real_path = builder;
                }

                all_identical =
                    diff_tier(m_writer, *rendered, real_path, repo_root) && all_identical;
            }
        }

        if (all_identical) {
            std::println(stderr, "[cc_abi_gen] done: all up to date");
        } else {
            std::println(stderr, "[cc_abi_gen] done: differences found");
        }

        if (all_identical) {
            return 0;
        } else {
            return 1;
        }
    }

    writer::Writer m_writer;
};

} // namespace cc_abi_gen
