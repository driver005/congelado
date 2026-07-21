#include <CLI/CLI.hpp>
import std;
import congelado_client;
import congelado_heart;
import congelado_worker;

/**
 * @brief Entry point for the congelado dev CLI — dispatches to `generate` (OpenAPI client SDK
 * codegen), `serve` (run the server main loop via `ServerRunner`), or `task` (load task-plugin
 * workers from a directory and run one task via `TaskRunner`), based on whichever subcommand
 * CLI11 parsed off `argv`. Three commands, one bet.
 * @param argc argument count, forwarded straight into CLI11.
 * @param argv argument vector, forwarded straight into CLI11.
 * @return `0` on success; `1` if codegen fails, an `--input` pair is missing its `=`, no worker
 * is registered for the requested `--type`, or task execution errors out; otherwise whatever
 * `ServerRunner::run` returns for the `serve` subcommand.
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

        CLI11_PARSE(app, argc, argv);

        if (generate_cmd->parsed()) {
            std::filesystem::create_directories(output_dir);

            // Build the generator, optionally pointing it at a pre-existing DTO module instead
            // of having it generate its own.
            auto generator = congelado::client::Generator{}.namespace_name(ns);
            if (!shared_models.empty()) {
                generator = std::move(generator).shared_models(shared_models);
            }

            // Bad OpenAPI doc or write failure both land here — report and bail with a nonzero
            // exit instead of leaving a half-written output directory unexplained.
            auto result = generator.generate(openapi_path, output_dir);
            if (!result) {
                std::println(stderr, "generate failed: {}", result.error());
                return 1;
            }
            std::println("generated client SDK in '{}'", output_dir);
        }

        if (serve_cmd->parsed()) {
            return congelado::heart::ServerRunner{plugin_dir}.run(config_path);
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
            runner.load_workers(workers_dir);

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
