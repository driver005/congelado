module;

#include <cstdlib>

export module cc_abi_gen:cli_runner;

import std;
import :cli_options;
import cc_abi_gen_parser;
import cc_abi_gen_generator;
import cc_abi_gen_writer;

export namespace cc_abi_gen {

// Command-line entry point: `generate` (genrule's explicit form, or --pilot's manual form) and
// `check` (dry-run diff, --pilot only).
class CliRunner
{
public:
    int run(int argc, char **argv)
    {
        m_arguments.clear();
        for (int index = 0; index < argc; ++index) {

            m_arguments.push_back(argv[index]);
        }

        if (m_arguments.size() < 2) {

            std::cerr << usage() << "\n";
            return 1;
        }

        CliOptions options = parse_options();

        if (m_arguments[1] == "generate") {

            return run_generate(options);
        }

        if (m_arguments[1] == "check") {

            return run_check(options);
        }

        std::cerr << usage() << "\n";
        return 1;
    }

private:
    std::string usage()
    {
        return "usage: cc_abi_gen generate --pilot [--out-dir <dir>] | "
            "cc_abi_gen generate --tier <builder|sonic> --domain <name> --header <path> --out <path> | "
            "cc_abi_gen check --pilot";
    }

    CliOptions parse_options()
    {
        CliOptions options;
        for (std::size_t index = 2; index < m_arguments.size(); ++index) {

            const std::string &argument = m_arguments[index];
            auto next_value = [&]() -> std::string {
                return index + 1 < m_arguments.size() ? m_arguments[++index] : std::string{};
            };

            if (argument == "--pilot") {

                options.m_pilot = true;
            } else if (argument == "--tier") {

                options.m_tier = next_value();
            } else if (argument == "--domain") {

                options.m_domain = next_value();
            } else if (argument == "--header") {

                options.m_header = next_value();
            } else if (argument == "--out") {

                options.m_out = next_value();
            } else if (argument == "--out-dir") {

                options.m_out_dir = next_value();
            } else if (argument == "--repo-root") {

                options.m_repo_root = next_value();
            }
        }

        return options;
    }

    std::filesystem::path resolve_repo_root(const CliOptions &options)
    {
        if (options.m_repo_root) {

            return *options.m_repo_root;
        }

        if (const char *workspace_directory = std::getenv("BUILD_WORKSPACE_DIRECTORY")) {

            return workspace_directory;
        }

        return std::filesystem::current_path();
    }

    // Where --pilot's generated files land: the genrule's own output directory when --out-dir is
    // given (build-time path — GeneratedFileWriter creates this tree itself, since a genrule's
    // output directory starts out empty), otherwise the real checked-in include/cc/abi tree
    // (manual `bazel run` path).
    std::filesystem::path resolve_output_root(const CliOptions &options, const std::filesystem::path &repo_root)
    {
        if (options.m_out_dir) {

            return *options.m_out_dir;
        }

        return repo_root / "include/cc/abi";
    }

    std::string render(const VtableModel &model, bool sonic_tier)
    {
        return sonic_tier ? m_sonic_emitter.render(model) : m_builder_emitter.render(model);
    }

    // Registers a just-generated domain's own type in the shared TypeRegistry, so a later
    // domain in the same run that references it (e.g. as a parameter type) finds it known
    // instead of flagging it as a gap. The wrap/unwrap formats are left empty (no conversion
    // attempted) — unlike a plain value type (TF_TString, TF_Status), a generated vtable-domain
    // type's own sonic wrapper takes a (ops, plugin_context) pair to construct, not a single
    // pointer, so there's no one-argument wrap/unwrap convention to record here yet; this only
    // records that the type exists, not how to convert to/from it.
    void register_generated_type(const VtableModel &model)
    {
        KnownType known;
        known.m_pointee_name = model.m_struct_name;
        known.m_cpp_parameter_type = "ice::sonic::" + model.m_class_name + " &";
        m_type_registry.register_generated(known);
    }

    // Only meaningful within one process's run, so this check only runs at the end of the
    // --pilot loops below: the build-time genrule wiring now invokes `generate --pilot` once
    // (see include/cc/abi_gen/BUILD) covering every domain in a single process, so this fires
    // there too — it's only the older explicit single-file mode (run_generate_single) that
    // still can't be covered, since each such invocation is its own fresh process.
    int fail_if_types_pending()
    {
        if (!m_type_registry.has_pending()) {

            return 0;
        }

        std::cerr << "[cc_abi_gen] unmodeled types referenced but never generated:\n";
        for (const std::string &pending : m_type_registry.pending()) {

            std::cerr << std::format("  {}\n", pending);
        }

        return 1;
    }

    // Explicit single-file mode: what the genrule wiring invokes.
    int run_generate_single(const CliOptions &options)
    {
        if (!options.m_tier || !options.m_domain || !options.m_header || !options.m_out) {

            std::cerr << usage() << "\n";
            return 1;
        }

        std::filesystem::path repo_root = resolve_repo_root(options);

        std::cerr << std::format("[cc_abi_gen] parsing {}\n", *options.m_header);

        auto model = m_header_parser.parse(*options.m_header, repo_root / "include");
        if (!model) {

            std::cerr << model.error() << "\n";
            return 1;
        }

        auto write_result = m_writer.write(
            render(*model, *options.m_tier == "sonic"), *options.m_out, repo_root);
        if (!write_result) {

            std::cerr << write_result.error() << "\n";
            return 1;
        }

        std::cerr << std::format("[cc_abi_gen] wrote {}\n", *options.m_out);

        return 0;
    }

    int write_generated(
        const VtableModel &model,
        const std::filesystem::path &out_path,
        const std::filesystem::path &repo_root,
        bool sonic_tier)
    {
        auto write_result = m_writer.write(render(model, sonic_tier), out_path, repo_root);
        if (!write_result) {

            std::cerr << write_result.error() << "\n";
            return 1;
        }

        std::cerr << std::format("[cc_abi_gen] wrote {}\n", out_path.string());

        return 0;
    }

    int run_generate(const CliOptions &options)
    {
        if (!options.m_pilot) {

            return run_generate_single(options);
        }

        std::filesystem::path repo_root = resolve_repo_root(options);
        std::filesystem::path output_root = resolve_output_root(options, repo_root);
        for (const std::string &domain : m_pilot_domains) {

            DomainPaths paths(domain, repo_root, output_root);

            std::cerr << std::format("[cc_abi_gen] parsing {}\n", paths.m_header.string());

            auto model = m_header_parser.parse(paths.m_header, repo_root / "include");
            if (!model) {

                std::cerr << model.error() << "\n";
                return 1;
            }

            if (int status = write_generated(*model, paths.m_builder_cppm, repo_root, false);
                status != 0) {

                return status;
            }

            if (int status = write_generated(*model, paths.m_sonic_cppm, repo_root, true);
                status != 0) {

                return status;
            }

            register_generated_type(*model);
        }

        if (int status = fail_if_types_pending(); status != 0) {

            return status;
        }

        std::cerr << std::format(
            "[cc_abi_gen] done: {} domain(s) regenerated\n", m_pilot_domains.size());

        return 0;
    }

    bool report_diff(
        const VtableModel &model,
        const std::filesystem::path &real_path,
        const std::filesystem::path &repo_root,
        bool sonic_tier)
    {
        auto diff_result = m_writer.diff(render(model, sonic_tier), real_path, repo_root);
        if (!diff_result) {

            std::cerr << diff_result.error() << "\n";
            return false;
        }

        if (diff_result->m_identical) {

            std::cerr << std::format("[cc_abi_gen] up to date: {}\n", real_path.string());
        } else {

            std::cout << "--- " << real_path.string() << " differs ---\n";
            std::cout << diff_result->m_unified_diff;
        }

        return diff_result->m_identical;
    }

    int run_check(const CliOptions &options)
    {
        if (!options.m_pilot) {

            std::cerr << usage() << "\n";
            return 1;
        }

        std::filesystem::path repo_root = resolve_repo_root(options);
        bool all_identical = true;

        for (const std::string &domain : m_pilot_domains) {

            DomainPaths paths(domain, repo_root, repo_root / "include/cc/abi");

            std::cerr << std::format("[cc_abi_gen] parsing {}\n", paths.m_header.string());

            auto model = m_header_parser.parse(paths.m_header, repo_root / "include");
            if (!model) {

                std::cerr << model.error() << "\n";
                return 1;
            }

            all_identical = report_diff(*model, paths.m_builder_cppm, repo_root, false) && all_identical;
            all_identical = report_diff(*model, paths.m_sonic_cppm, repo_root, true) && all_identical;

            register_generated_type(*model);
        }

        if (int status = fail_if_types_pending(); status != 0) {

            return status;
        }

        std::cerr << std::format(
            "[cc_abi_gen] done: {}\n", all_identical ? "all up to date" : "differences found");

        return all_identical ? 0 : 1;
    }

    std::vector<std::string> m_arguments;
    std::vector<std::string> m_pilot_domains{"cache", "logger"};
    TypeRegistry m_type_registry;
    HeaderParser m_header_parser;
    BuilderEmitter m_builder_emitter{m_type_registry};
    SonicEmitter m_sonic_emitter{m_type_registry};
    GeneratedFileWriter m_writer;
};

} // namespace cc_abi_gen
