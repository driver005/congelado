#include <CLI/CLI.hpp>
#include <congelado/abi.h>
import std;
import congelado_heart;
import congelado_worker;
import core_plugin;
import utils_openapi;
import utils_hash;
import connector;
import model;
import serde;

namespace {

/**
 * @brief Appends `connector::Sql::build_create_sql<T>()` for every T in Tuple, in tuple order.
 * @tparam Tuple the tuple of model types to generate DDL for.
 * @param out the DDL strings, one per model, appended in tuple order.
 */
template <typename Tuple, std::size_t... I>
void collect_create_ddl(std::vector<std::string> &out, std::index_sequence<I...>) {
    (out.push_back(connector::Sql::build_create_sql<std::tuple_element_t<I, Tuple>>()), ...);
}

/**
 * @brief Builds a deterministic DDL dump for every registered model, one `CREATE TABLE`
 * statement per model, in `model::AllModels`'s declared order.
 * @return the DDL statements, semicolon-terminated and newline-joined.
 */
[[nodiscard]] std::string build_schema_dump() {
    std::vector<std::string> statements;
    collect_create_ddl<model::AllModels>(
        statements, std::make_index_sequence<std::tuple_size_v<model::AllModels>>{});

    std::string dump;
    for (const auto &statement : statements) {
        dump += statement;
        dump += ";\n";
    }
    return dump;
}

/**
 * @brief Finds the lexically-last `*.sql` file in `dir` (fixed-width timestamp filenames sort
 * lexically in chronological order), if any.
 * @param dir the migrations directory to scan.
 * @return the newest migration file's path, or `std::nullopt` if `dir` has none.
 */
[[nodiscard]] std::optional<std::filesystem::path> find_last_migration(const std::filesystem::path &dir) {
    std::optional<std::filesystem::path> last;
    if (!std::filesystem::exists(dir)) {
        return last;
    }
    for (const auto &entry : std::filesystem::directory_iterator(dir)) {
        if (!entry.is_regular_file() || entry.path().extension() != ".sql") {
            continue;
        }
        if (!last || entry.path().filename().string() > last->filename().string()) {
            last = entry.path();
        }
    }
    return last;
}

} // namespace

/**
 * @brief Entry point for the congelado dev CLI — dispatches to `generate` (OpenAPI client SDK
 * codegen), `serve` (run the server main loop via `ServerRunner`), `task` (load task-plugin
 * workers from a directory and run one task via `TaskRunner`), or `migrate` (generate a
 * versioned `.sql` migration file from the current model schema), based on whichever subcommand
 * CLI11 parsed off `argv`.
 * @param argc argument count, forwarded straight into CLI11.
 * @param argv argument vector, forwarded straight into CLI11.
 * @return `0` on success; `1` if codegen fails, an `--input` pair is missing its `=`, no worker
 * is registered for the requested `--type`, task execution errors out, or `migrate` can't load
 * its SQL dialect plugin or write its output file; otherwise whatever `ServerRunner::run`
 * returns for the `serve` subcommand.
 */
int main(int argc, char *argv[]) {
    // Everything in this function can throw (CLI11's App/Option construction, parsing,
    // filesystem ops, codegen, the server/task runners) and CLI11_PARSE only catches its
    // own CLI::ParseError internally — other exception types would escape uncaught. An
    // uncaught exception escaping main() terminates the process the same way a noexcept
    // violation would, just with a worse diagnostic than a reported message.
    try {
        CLI::App app{"congelado developer CLI"};
        app.require_subcommand(1);

        // ── generate: OpenAPI client SDK codegen ────────────────────────────
        auto *generate_cmd =
            app.add_subcommand("generate", "Generate a typed C++ client SDK from an OpenAPI document");
        std::string openapi_path;
        std::string output_dir;
        std::string ns = "client";
        std::string shared_models;
        generate_cmd->add_option("-o,--openapi", openapi_path, "Path to openapi.json")
            ->required()
            ->check(CLI::ExistingFile);
        generate_cmd->add_option("-d,--output", output_dir, "Output directory for generated .cppm files")
            ->required();
        generate_cmd->add_option("-n,--namespace", ns,
                                 "Namespace/module prefix for generated code (default: client)");
        generate_cmd->add_option("-m,--shared-models", shared_models,
                                 "Reuse an existing module for DTOs instead of generating one (e.g. \"model\")");

        // ── serve: run the server main loop ─────────────────────────────────
        auto *serve_cmd = app.add_subcommand("serve", "Run the server main loop (ServerRunner)");
        std::string plugin_dir;
        std::string config_path = "congelado.toml";
        serve_cmd->add_option("-p,--plugin-dir", plugin_dir, "Directory to scan for plugins")
            ->required()
            ->check(CLI::ExistingDirectory);
        serve_cmd->add_option("-c,--config", config_path, "Path to the server config TOML");

        // ── task: load task-plugin workers and run one task ─────────────────
        auto *task_cmd =
            app.add_subcommand("task", "Load task-plugin workers from a directory and run one task (TaskRunner)");
        std::string workers_dir;
        std::string task_type;
        std::string worker_id;
        std::vector<std::string> input_pairs;
        task_cmd->add_option("-w,--workers-dir", workers_dir, "Directory to scan for task-plugin .so files")
            ->required()
            ->check(CLI::ExistingDirectory);
        task_cmd->add_option("-t,--type", task_type, "Task type to execute")->required();
        task_cmd->add_option("-i,--input", input_pairs, "Input key=value pair (repeatable)");
        task_cmd->add_option("--worker-id", worker_id, "Worker identity to report to the engine");

        // ── migrate: generate a versioned migration file from the current model schema ──
        auto *migrate_cmd = app.add_subcommand(
            "migrate", "Generate a versioned migration file from the current model schema");
        std::string migrations_out_dir = "migrations";
        migrate_cmd->add_option("-d,--migrations-dir", migrations_out_dir,
                                "Directory to write the generated migration into (default: migrations)");

        CLI11_PARSE(app, argc, argv);

        if (generate_cmd->parsed()) {
            std::filesystem::create_directories(output_dir);

            // congelado::client::Generator moved out of this CLI's direct reach and into
            // plugins/openapi_generator/ (see interfaces::IOpenApiGenerator) — client-SDK
            // codegen is this subcommand's entire purpose, so a missing/failed plugin load is
            // just as hard a failure as any other "generate failed" bail below. Plugin
            // directory is derived from this binary's own path, same convention
            // congelado_worker's main() already uses for its own plugin scan.
            auto plugin_base =
                argc > 0 ? std::filesystem::path(argv[0]).parent_path() : std::filesystem::path{};
            auto plugins_dir =
                std::filesystem::path{std::format("{}/../../../plugins", plugin_base.string())};

            core::plugin::SharedLibrary plugin_store{"plugin"};
            plugin_store.scan(plugins_dir);
            auto open_res = plugin_store.open(plugins_dir / "libopenapi_generator.so");
            if (!open_res) {
                std::println(stderr, "generate failed: plugin load failed: {}",
                             open_res.error().get_message());
                return 1;
            }
            CongeladoHostCallbacks empty_host_cb{};
            auto build_res = plugin_store.build(empty_host_cb, {});
            if (!build_res) {
                std::println(stderr, "generate failed: plugin build failed: {}",
                             build_res.error().get_message());
                return 1;
            }

            utils::openapi::OpenApiGeneratorRegistry generator_registry;
            plugin_store.for_each(
                [&generator_registry](const std::shared_ptr<core::plugin::FfiRuntime> &runtime) {
                    auto plugin = runtime->get_plugin();
                    if (!plugin) {
                        return;
                    }
                    if (auto generator = congelado::heart::resolve_openapi_generator(*plugin)) {
                        generator_registry.add_generator(std::move(generator));
                    }
                });
            if (!generator_registry.has_generator()) {
                std::println(stderr, "generate failed: no OpenAPI generator plugin loaded — "
                                     "was openapi_generator built?");
                return 1;
            }
            auto *doc_generator = generator_registry.get_generators().front().get();

            // Bad OpenAPI doc or write failure both land here — report and bail with a nonzero
            // exit instead of leaving a half-written output directory unexplained.
            std::optional<std::string_view> shared_models_opt;
            if (!shared_models.empty()) {
                shared_models_opt = shared_models;
            }
            auto result = doc_generator->generate_client_sdk(openapi_path, output_dir, ns, shared_models_opt);
            if (!result) {
                std::println(stderr, "generate failed: {}", result.error());
                return 1;
            }
            std::println("generated client SDK in '{}'", output_dir);
        }

        if (serve_cmd->parsed()) {
            // --plugin-dir is the one directory this subcommand ever scans — pass it as the
            // external dir and blank out the internal default so no unrelated "./plugins" gets
            // picked up.
            return congelado::heart::ServerRunner{plugin_dir, std::filesystem::path{}}
                .run(config_path);
        }

        if (task_cmd->parsed()) {
            // Parse every repeatable --input key=value pair into a map — a pair missing its '='
            // is a hard error, no partial-parse motion here.
            std::unordered_map<std::string, std::string> data;
            for (const auto &pair : input_pairs) {
                auto eq = pair.find('=');
                if (eq == std::string::npos) {
                    std::println(stderr, "invalid --input '{}': expected key=value", pair);
                    return 1;
                }
                data.emplace(pair.substr(0, eq), pair.substr(eq + 1));
            }

            // Scan the workers directory for task-plugin .so files before checking whether the
            // requested type actually showed up among them.
            congelado::worker::TaskRunner runner{worker_id};
            // --workers-dir is the one directory this subcommand ever scans — pass it as the
            // external dir and blank out the internal default so no unrelated "./workers" gets
            // picked up.
            runner.load_workers(workers_dir, std::filesystem::path{});

            if (!runner.has_task_type(task_type)) {
                std::println(stderr, "task failed: no worker registered for type '{}'", task_type);
                return 1;
            }

            // Worker's there — run it and print whatever output fields it hands back.
            congelado::worker::TaskInput input{data};
            auto result = runner.execute(task_type, input);
            if (!result) {
                std::println(stderr, "task failed: '{}' raised an error", task_type);
                return 1;
            }

            for (const auto &[key, value] : result->get_data()) {
                std::println("{}={}", key, value);
            }
        }

        if (migrate_cmd->parsed()) {
            // Only the SQL dialect serde plugin is needed to generate DDL text — no live DB,
            // no Connector, no AppContext. Same single-.so open() pattern as `generate` above,
            // so no protocol plugin's on_ready() listener is ever touched.
            auto plugin_base =
                argc > 0 ? std::filesystem::path(argv[0]).parent_path() : std::filesystem::path{};
            auto plugins_dir =
                std::filesystem::path{std::format("{}/../../../plugins", plugin_base.string())};

            core::plugin::SharedLibrary plugin_store{"plugin"};
            plugin_store.scan(plugins_dir);
            auto open_res = plugin_store.open(plugins_dir / "libsql_postgres_plugin.so");
            if (!open_res) {
                std::println(stderr, "migrate failed: plugin load failed: {}",
                             open_res.error().get_message());
                return 1;
            }
            CongeladoHostCallbacks empty_host_cb{};
            auto build_res = plugin_store.build(empty_host_cb, {});
            if (!build_res) {
                std::println(stderr, "migrate failed: plugin build failed: {}",
                             build_res.error().get_message());
                return 1;
            }

            serde::SerdeFormatRegistry format_registry;
            plugin_store.for_each(
                [&format_registry](const std::shared_ptr<core::plugin::FfiRuntime> &runtime) {
                    auto plugin = runtime->get_plugin();
                    if (!plugin) {
                        return;
                    }
                    if (auto format = congelado::heart::resolve_serde_format(*plugin)) {
                        format_registry.add_format(std::move(format));
                    }
                });
            // Mirrors connector::Sql's own internal SQL_DIALECT_CONTENT_TYPE constant (not
            // exported outside the connector module) — kept in sync with
            // sql_postgres_plugin.cc's content_type() override, "application/sql+postgres".
            if (format_registry.find("application/sql+postgres") == nullptr) {
                std::println(stderr, "migrate failed: no SQL dialect plugin loaded — "
                                     "was sql_postgres built?");
                return 1;
            }
            serde::SerdeFormatRegistry::set_active(&format_registry);

            auto dump = build_schema_dump();
            auto checksum = utils::Sha256::hash_hex(dump);

            std::filesystem::create_directories(migrations_out_dir);
            auto last_migration = find_last_migration(migrations_out_dir);
            if (last_migration) {
                std::ifstream last_stream(*last_migration);
                std::string last_contents((std::istreambuf_iterator<char>(last_stream)),
                                          std::istreambuf_iterator<char>());
                if (utils::Sha256::hash_hex(last_contents) == checksum) {
                    std::println("no schema changes, nothing to generate");
                    return 0;
                }
            }

            auto timestamp = std::format("{:%Y%m%d%H%M%S}",
                                         std::chrono::floor<std::chrono::seconds>(
                                             std::chrono::system_clock::now()));
            auto out_path =
                std::filesystem::path{migrations_out_dir} / std::format("{}_schema.sql", timestamp);
            std::ofstream out_stream(out_path);
            if (!out_stream) {
                std::println(stderr, "migrate failed: cannot write '{}'", out_path.string());
                return 1;
            }
            out_stream << dump;
            std::println("generated migration '{}'", out_path.string());
        }

        return 0;
    } catch (const std::exception &exception) {
        try {
            std::println(stderr, "fatal: {}", exception.what());
        } catch (...) { // NOLINT(bugprone-empty-catch) — best-effort diagnostic only
        }
        return 1;
    } catch (...) {
        try {
            std::println(stderr, "fatal: unknown exception");
        } catch (...) { // NOLINT(bugprone-empty-catch) — best-effort diagnostic only
        }
        return 1;
    }
}
