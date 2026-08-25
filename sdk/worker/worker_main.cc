#include "backward.hpp"

#include <congelado/abi.h>
#include <csignal>

import std;
import congelado_worker;
import congelado_heart;
import interfaces;
import io_layer_http2;
import core_config;
import core_logger;
import core_otel;
import core_events;
import core_client;
import core_plugin;
import io_base_socket;
import io_base_leverage;
import core_contract;
import shared;
import serde;
import connector;
import congelado_api_dto;
import congelado_api_routes;

namespace {

std::filesystem::path expand_tilde(const std::filesystem::path &path) {
    std::string path_str = path.string();
    // Only a leading '~' gets expanded — nothing to do for a path that doesn't start with one.
    if (!path_str.empty() && path_str.front() == '~') {
        // No HOME env var means no expansion happens — path_str is left with the literal '~'.
        // NOLINTNEXTLINE(concurrency-mt-unsafe)
        const char *home = std::getenv("HOME");
        if (home != nullptr) {
            path_str.replace(0, 1, home);
        }
    }
    return {path_str};
}

// Set only from inside request_shutdown() below — a plain atomic<bool>, async-signal-safe.
std::atomic<bool> g_shutdown_requested{false};

/// @brief SIGINT/SIGTERM handler — signals the wait loop so run_worker() returns and every local's
/// destructor runs (including plugin_store's, which calls congelado_on_unload()).
void request_shutdown(int /*signal*/) noexcept {
    g_shutdown_requested.store(true, std::memory_order_relaxed);
}

/// @brief Blocking-adapts a `congelado_api::*` typed call (which takes `(onResponse, onError)` and
/// returns immediately) into a synchronous `std::expected<T, std::string>`.
template <typename T, typename Fn>
[[nodiscard]] std::expected<T, std::string> call_typed_blocking(Fn &&issue_call) {
    std::promise<std::expected<T, std::string>> promise;
    auto future = promise.get_future();
    issue_call(
        [&promise](T value) { promise.set_value(std::move(value)); },
        [&promise](std::string error) { promise.set_value(std::unexpected{std::move(error)}); });
    return future.get();
}

/// @brief Builds the transport dispatch callback: logs error statuses, then hands the response to
/// both `manager` (WorkerContext::call_engine's own pending map) and `api_client` (the generated
/// typed API's pending map) — each is an independent `core::client::Register`, so a response only
/// ever matches whichever one actually issued that stream id; the other's dispatch() is a no-op.
[[nodiscard]] interfaces::io::ReceiveDispatchFn
make_engine_dispatch(interfaces::IWorkerManager &manager, congelado_api::Client &api_client) {
    return [&manager, &api_client](interfaces::io::IRequest &req, interfaces::io::IResponse &res,
                                   std::function<void()> /*send*/) {
        auto status = static_cast<int>(interfaces::io::types::status_code(res.get_status()));
        if (status >= 400) {
            auto &body_view = res.get_body();
            std::string body;
            body.reserve(body_view.size());
            for (auto byte : body_view) {
                body.push_back(static_cast<char>(byte));
            }
            core::logger::error("worker/host", "engine response error status={} stream={} body={}",
                                status, req.get_stream_id(), body);
        }
        manager.dispatch(req, res);
        api_client.dispatch(req, res);
    };
}

/// @brief Reads every `*.json` file directly under `dir` and returns their raw contents — used to
/// collect an app's serialized TaskDef/WorkflowDef files for on-load registration. A missing dir
/// yields an empty list (an app may ship only one def kind, or use the code-builder path instead).
[[nodiscard]] std::vector<std::string> collect_def_files(const std::filesystem::path &dir) {
    std::vector<std::string> defs;
    std::error_code error_code;
    if (!std::filesystem::is_directory(dir, error_code)) {
        return defs;
    }
    for (const auto &entry : std::filesystem::directory_iterator{dir, error_code}) {
        if (!entry.is_regular_file() || entry.path().extension() != ".json") {
            continue;
        }
        std::ifstream stream{entry.path(), std::ios::binary};
        if (!stream) {
            continue;
        }
        std::string content{std::istreambuf_iterator<char>{stream},
                            std::istreambuf_iterator<char>{}};
        defs.push_back(std::move(content));
    }
    return defs;
}

/// @brief Walks every `<apps_dir>/<app>/<subdir>/*.json` and concatenates the collected def files —
/// e.g. `subdir = "taskdefs"` or `"workflows"`. Each immediate child of `apps_dir` is one app.
[[nodiscard]] std::vector<std::string> collect_app_defs(const std::filesystem::path &apps_dir,
                                                        std::string_view subdir) {
    std::vector<std::string> defs;
    std::error_code error_code;
    if (!std::filesystem::is_directory(apps_dir, error_code)) {
        return defs;
    }
    for (const auto &app : std::filesystem::directory_iterator{apps_dir, error_code}) {
        if (!app.is_directory()) {
            continue;
        }
        auto app_defs = collect_def_files(app.path() / subdir);
        defs.insert(defs.end(), std::make_move_iterator(app_defs.begin()),
                    std::make_move_iterator(app_defs.end()));
    }
    return defs;
}

/// @brief Binds one manager->poll_slot() cycle into a schedulable contract via `shared::HandlerBase`
/// — the same `create(group, state)` idiom `connector::Connector`/`core::router::RouterExecutor`
/// already use, instead of handing `ContractGroup::create()` a raw lambda directly.
class PollSlotHandler final : public shared::HandlerBase {
  public:
    PollSlotHandler(interfaces::IWorkerManager &manager, std::uint32_t slot)
        : m_manager{manager}, m_slot{slot}, m_name{std::format("poll-{}", slot)} {}

    [[nodiscard]] std::string_view get_name() const noexcept override { return m_name; }

    shared::WorkerFunction on_execute() override {
        return [this] { m_manager.poll_slot(m_slot); };
    }

    shared::ErrorHandler on_error() override {
        return [this](std::exception_ptr eptr) {
            try {
                std::rethrow_exception(eptr);
            } catch (const std::exception &error) {
                core::logger::error("worker/poll", "poll-{} error: {}", m_slot, error.what());
            }
            // Errors don't auto-reschedule — re-arm so one bad cycle doesn't stop this slot.
            shared::this_handler::shedule();
        };
    }

  private:
    interfaces::IWorkerManager &m_manager;
    std::uint32_t m_slot;
    std::string m_name;
};

/**
 * @brief Worker host process: loads the process plugins + the worker-manager plugin + the IWorker
 * worker plugins, connects to the engine over HTTP/2, and runs the manager's poll loop on a
 * contract thread pool until killed. The http2 polling + IWorker dispatch itself lives in the
 * manager/worker_external plugin; this host just wires everything up and drives the pool.
 * @param argc argument count.
 * @param argv `argv[1]` (optional) worker config path (default `~/cc/congelado/config/worker.toml`);
 * `argv[2]` (optional) the worker-plugin directory (default `./workers`).
 * @return `0` on clean shutdown, `1` on a fatal setup failure.
 */
int run_worker(int argc, char *argv[]) {
    backward::SignalHandling sh;

    // The engine can close its end mid-write, raising SIGPIPE — ignore it; the send's try/catch
    // handles a failed write via the thrown exception path.
    std::signal(SIGPIPE, SIG_IGN);

    auto config_path = argc > 1 ? std::string{argv[1]} : "~/cc/congelado/config/worker.toml";
    // argv[2] overrides the worker-plugin dir; empty means "derive from argv[0]" below (next to the
    // build output, like plugins_dir/apps_dir), so a no-arg local `xmake run` finds build/workers.
    auto workers_dir = argc > 2 ? std::string{argv[2]} : std::string{};

    // ── 0. Process environment: force-load the plugins this host needs ──────────
    // A serde format to decode worker.toml + submit JSON, optionally an OTel provider + a file
    // logger, and the worker-manager plugin (http2 poll runtime + IWorkerManager).
    serde::SerdeFormatRegistry serde_format_registry;
    serde::SerdeFormatRegistry::set_active(&serde_format_registry);

    std::unordered_map<std::string, core::plugin::types::GenerationConfig> plugin_configs;
    if (auto raw_cfg = core::config::load(expand_tilde(config_path))) {
        for (const auto &[name, plugin_cfg] : raw_cfg->get_plugins()) {
            core::plugin::types::GenerationConfig generation_config;
            generation_config.add_runtime(plugin_cfg.get_type());
            for (const auto &[key, value] : plugin_cfg.get_fields()) {
                auto extra = generation_config.get_extra();
                extra[key] = value;
                generation_config.set_extra(std::move(extra));
            }
            plugin_configs[name] = std::move(generation_config);
        }
    }

    core::logger::LoggerRegistry logger_registry;
    core::logger::LoggerRegistry::set_active(&logger_registry);

    core::otel::TracerRegistry tracer_registry;
    core::otel::MeterRegistry meter_registry;
    core::otel::LogRecordRegistry log_record_registry;
    core::otel::TracerRegistry::set_active(&tracer_registry);
    core::otel::MeterRegistry::set_active(&meter_registry);
    core::otel::LogRecordRegistry::set_active(&log_record_registry);

    // Event bus: the worker host resolves any IEventSink plugins it loaded and fans them out through
    // core::events::publish — that's how the kafka_publish worker reaches a broker (injected sink),
    // rather than each worker linking a broker library itself.
    core::events::EventBusRegistry event_bus_registry;
    core::events::EventBusRegistry::set_active(&event_bus_registry);

    auto plugin_base =
        argc > 0 ? std::filesystem::path(argv[0]).parent_path() : std::filesystem::path{};
    auto plugins_dir =
        std::filesystem::path{std::format("{}/../../../plugins", plugin_base.string())};
    // App def files live next to the build output (build/apps/<app>/{taskdefs,workflows}/*.json),
    // derived from argv[0] like plugins_dir so it resolves the same under local `xmake run` and the
    // docker image.
    auto apps_dir = std::filesystem::path{std::format("{}/../../../apps", plugin_base.string())};
    // Default the worker-plugin dir to build/workers relative to argv[0] (same derivation) when no
    // argv[2] was given, so it resolves without passing a path locally or in docker.
    if (workers_dir.empty()) {
        workers_dir = std::format("{}/../../../workers", plugin_base.string());
    }
    core::plugin::SharedLibrary plugin_store{"plugin"};
    plugin_store.scan(plugins_dir);
    for (const char *plugin_name : {"libtoml_plugin.so", "libotel_otlp_plugin.so",
                                    "libfile_logger.so", "libjson_plugin.so",
                                    "libworker_manager_external_plugin.so"}) {
        if (auto open_res = plugin_store.open(plugins_dir / plugin_name); !open_res) {
            std::println(stderr, "[worker] plugin '{}' load failed: {}", plugin_name,
                         open_res.error().get_message());
        }
    }

    // Contract infrastructure the external worker-manager plugin needs for its inbound HTTP server
    // — created before plugin build so the host callbacks can carry it into the plugin's on_load.
    core::contract::ContractGroup<> contract_group;
    io::base::leverage::Leverager<io::base::leverage::Context> leverager;
    core::contract::ContractRegistry contract_registry;
    core::contract::ContractThreadPool<> thread_pool(contract_group,
                                                     std::thread::hardware_concurrency());

    // Point the external worker-manager plugin at the worker config so start_server() can read its
    // bind address + worker id (parsed there, once the serde TOML format is registered).
    {
        core::plugin::types::GenerationConfig external_cfg;
        external_cfg.add_runtime("worker_manager");
        auto extra = external_cfg.get_extra();
        extra["config_path"] = expand_tilde(config_path).string();
        external_cfg.set_extra(std::move(extra));
        plugin_configs["worker_manager_external"] = std::move(external_cfg);
    }

    CongeladoHostCallbacks empty_host_cb{};
    CongeladoHostCallbacks host_cb{};
    host_cb.controller_ctx = &contract_group;
    host_cb.leverager_ctx = &leverager;
    host_cb.registry_ctx = &contract_registry;
    interfaces::IWorkerManager *manager = nullptr;
    std::shared_ptr<interfaces::IWorkerManager> manager_holder;
    if (auto build_res = plugin_store.build(host_cb, plugin_configs); build_res) {
        plugin_store.for_each([&](const std::shared_ptr<core::plugin::FfiRuntime> &runtime) {
            auto plugin = runtime->get_plugin();
            if (!plugin) {
                return;
            }
            if (auto format = congelado::heart::resolve_serde_format(*plugin)) {
                serde_format_registry.add_format(std::move(format));
            }
            if (auto logger_adapter = congelado::heart::LoggerAdapter::register_from(*plugin)) {
                logger_adapter->register_logger(logger_registry);
            }
            if (auto sink = congelado::heart::resolve_event_sink(*plugin)) {
                event_bus_registry.add_sink(std::move(sink));
            }
            if (auto otel_provider = congelado::heart::resolve_otel_provider(*plugin)) {
                if (auto *tracer = otel_provider->get_tracer_provider()) {
                    tracer_registry.add_provider(std::shared_ptr<interfaces::ITracerProvider>(
                        tracer, [](interfaces::ITracerProvider *) {}));
                }
                if (auto *meter = otel_provider->get_meter_provider()) {
                    meter_registry.add_provider(std::shared_ptr<interfaces::IMeterProvider>(
                        meter, [](interfaces::IMeterProvider *) {}));
                }
                if (auto *log_provider = otel_provider->get_log_provider()) {
                    log_record_registry.add_provider(std::shared_ptr<interfaces::ILogRecordProvider>(
                        log_provider, [](interfaces::ILogRecordProvider *) {}));
                }
            }
            if (auto worker_manager = congelado::heart::resolve_worker_manager(*plugin)) {
                manager_holder = worker_manager;
                manager = worker_manager.get();
            }
        });
    } else {
        std::println(stderr, "[worker] plugin build failed: {}", build_res.error().get_message());
    }
    if (manager == nullptr) {
        std::println(stderr, "[worker] no worker_manager plugin found — aborting");
        return 1;
    }

    congelado::heart::OtelLogBridge::install(logger_registry);

    // ── 1. Load worker config ──────────────────────────────────────────
    auto cfg_result = congelado::worker::WorkerConfig::from_file(expand_tilde(config_path));
    if (!cfg_result) {
        throw std::runtime_error(std::format("config load failed: {}", cfg_result.error()));
    }
    auto &cfg = *cfg_result;

    // NOLINTNEXTLINE(concurrency-mt-unsafe) — boot-time only
    if (const char *env_worker_id = std::getenv("CONGELADO_WORKER_ID"); env_worker_id != nullptr) {
        cfg.setWorkerId(env_worker_id);
    }
    // NOLINTNEXTLINE(concurrency-mt-unsafe) — boot-time only
    if (const char *env_concurrency = std::getenv("CONGELADO_WORKER_CONCURRENCY");
        env_concurrency != nullptr) {
        std::uint32_t concurrency = cfg.getConcurrency();
        auto parsed = std::from_chars(env_concurrency,
                                      env_concurrency + std::strlen(env_concurrency), concurrency);
        if (parsed.ec == std::errc{}) {
            cfg.setConcurrency(concurrency);
        }
    }

    std::println("[worker] loaded config: id='{}' engine={}:{} concurrency={}", cfg.getWorkerId(),
                 cfg.getEngineHost(), cfg.getEnginePort(), cfg.getConcurrency());

    // ── Shared connector: resolve DB/cache from the process plugins and inject it into the app
    // workers, the same capability DI the engine host uses. With no storage plugin loaded the
    // connector runs in-process (LocalStore) and executes its ops synchronously.
    connector::Connector shared_connector;
    interfaces::IDatabase *worker_database = nullptr;
    interfaces::ICache *worker_cache = nullptr;
    plugin_store.for_each([&](const std::shared_ptr<core::plugin::FfiRuntime> &runtime) {
        auto plugin = runtime->get_plugin();
        if (!plugin) {
            return;
        }
        if (worker_database == nullptr) {
            if (auto database = congelado::heart::resolve_storage(*plugin)) {
                worker_database = database.get();
            }
        }
        if (worker_cache == nullptr) {
            if (auto cache = congelado::heart::resolve_cache(*plugin)) {
                worker_cache = cache.get();
            }
        }
    });
    shared_connector.set_database(worker_database);
    shared_connector.set_cache(worker_cache);
    auto connector_contract =
        shared_connector.create(contract_group, core::contract::ContractState::IDLE);
    shared_connector.set_wake([contract = connector_contract]() mutable { contract.schedule(); });
    contract_registry.add(std::move(connector_contract));

    CongeladoHostCallbacks worker_host_cb{};
    worker_host_cb.controller_ctx = &contract_group;
    worker_host_cb.leverager_ctx = &leverager;
    worker_host_cb.connector_ctx = &shared_connector;
    worker_host_cb.database_ctx = worker_database;
    worker_host_cb.cache_ctx = worker_cache;
    // The `client`/`client_pool` workers own their downstream client: each calls
    // downstream_protocol.get_client(...) itself in on_load and connects it (see
    // connect_downstream in their impl). This protocol just needs a stable address that outlives
    // every worker, hence function scope. No `client_host` configured ⇒ no protocol handed out,
    // and those workers degrade to an error at run time.
    core::config::PluginConfig downstream_cfg;
    std::optional<io::layer::http2::Http2Protocol> downstream_protocol;
    std::unordered_map<std::string, core::plugin::types::GenerationConfig> worker_plugin_configs;
    if (cfg.getClientHost().has_value()) {
        downstream_cfg.add_field("host", *cfg.getClientHost());
        downstream_cfg.add_field("port", std::to_string(cfg.getClientPort().value_or(443)));
        if (cfg.getClientCert().has_value()) {
            downstream_cfg.add_field("cert", *cfg.getClientCert());
        }
        if (cfg.getClientKey().has_value()) {
            downstream_cfg.add_field("key", *cfg.getClientKey());
        }
        downstream_protocol.emplace(&downstream_cfg);

        // The client workers read their own "host"/"port"/"cert"/"key" config fields (not
        // client_host/client_port) — forward the same downstream endpoint under each plugin's own
        // registered name so their on_load can connect_downstream().
        core::plugin::types::GenerationConfig client_cfg;
        auto extra = client_cfg.get_extra();
        extra["host"] = *cfg.getClientHost();
        extra["port"] = std::to_string(cfg.getClientPort().value_or(443));
        if (cfg.getClientCert().has_value()) {
            extra["cert"] = *cfg.getClientCert();
        }
        if (cfg.getClientKey().has_value()) {
            extra["key"] = *cfg.getClientKey();
        }
        client_cfg.set_extra(extra);
        worker_plugin_configs["client_worker"] = client_cfg;
        worker_plugin_configs["client_pool_worker"] = std::move(client_cfg);
    }
    worker_host_cb.client_protocol_ctx = downstream_protocol.has_value()
                                             ? static_cast<void *>(&*downstream_protocol)
                                             : nullptr;

    // ── 2. Load the IWorker worker plugins and register them into the manager ──
    std::vector<std::shared_ptr<interfaces::IWorker>> worker_holders;
    core::plugin::SharedLibrary worker_store{"plugin"};
    worker_store.scan(workers_dir);
    if (auto open_res = worker_store.open_all(); !open_res) {
        std::println(stderr, "[worker] failed to open worker plugins from '{}': {}", workers_dir,
                     open_res.error().get_message());
    }
    if (auto build_res = worker_store.build(worker_host_cb, worker_plugin_configs); !build_res) {
        std::println(stderr, "[worker] worker plugin build failed: {}",
                     build_res.error().get_message());
    }
    std::vector<std::string> builder_task_defs;
    std::vector<std::string> builder_workflow_defs;
    worker_store.for_each([&](const std::shared_ptr<core::plugin::FfiRuntime> &runtime) {
        auto plugin = runtime->get_plugin();
        if (!plugin) {
            return;
        }
        if (auto worker = congelado::heart::resolve_worker(*plugin)) {
            manager->add_worker(*worker);
            worker_holders.push_back(std::move(worker));
        }
        // Code-built defs (IAppDefs) — the C++-builder counterpart to the def files; collect them
        // here and merge into the on-load registration below.
        if (auto app_defs = congelado::heart::resolve_app_defs(*plugin)) {
            auto task_defs = app_defs->get_task_defs();
            builder_task_defs.insert(builder_task_defs.end(),
                                     std::make_move_iterator(task_defs.begin()),
                                     std::make_move_iterator(task_defs.end()));
            auto workflow_defs = app_defs->get_workflow_defs();
            builder_workflow_defs.insert(builder_workflow_defs.end(),
                                         std::make_move_iterator(workflow_defs.begin()),
                                         std::make_move_iterator(workflow_defs.end()));
        }
    });
    std::print("[worker] registered {} workers:", worker_holders.size());
    for (const auto &worker : worker_holders) {
        std::print(" {}", worker->get_task_type());
    }
    std::println();

    // ── 3. Set up engine connection via HTTP/2 ─────────────────────────
    // contract_group / leverager / thread_pool were created above (before plugin build) so the
    // external plugin could receive the group + leverager via host callbacks. >1 pool thread is
    // load-bearing: it pumps the outbound engine connection AND every poll contract.
    core::config::PluginConfig http2_cfg;
    http2_cfg.add_field("host", cfg.getEngineHost());
    http2_cfg.add_field("port", std::to_string(cfg.getEnginePort()));
    http2_cfg.add_field("cert", cfg.getEngineCert().empty() ? "server.crt" : cfg.getEngineCert());
    http2_cfg.add_field("key", cfg.getEngineKey().empty() ? "server.key" : cfg.getEngineKey());

    // Owns the correlator for every generated congelado_api::* typed call — bound to the live
    // connection below, kept alive for the rest of main() (it's referenced by the dispatch
    // callback wired into the connection for as long as that connection exists).
    congelado_api::Client engine_api_client;

    auto protocol = io::layer::http2::Http2Protocol{&http2_cfg};
    auto dispatch = make_engine_dispatch(*manager, engine_api_client);
    auto engine_client = protocol.get_client(std::move(dispatch));

    bool verify_peer = !cfg.getEngineCert().empty() || !cfg.getEngineKey().empty();
    io::base::socket::Endpoint engine_endpoint{cfg.getEngineHost(),
                                               static_cast<std::uint16_t>(cfg.getEnginePort())};
    auto *http2_client = dynamic_cast<io::layer::http2::Client *>(engine_client.get());
    if (http2_client == nullptr) {
        std::println(stderr, "[worker] engine client is not an HTTP/2 client");
        return 1;
    }
    const auto retry_delay = std::chrono::milliseconds{
        cfg.getConnectRetryDelayMs().value_or(congelado::worker::consts::connect_retry_delay_ms)};
    const auto connect_timeout_ms =
        cfg.getConnectTimeoutMs().value_or(congelado::worker::consts::connect_timeout_ms);
    const bool retry_forever = connect_timeout_ms == 0;
    const auto connect_deadline =
        std::chrono::steady_clock::now() + std::chrono::milliseconds{connect_timeout_ms};

    auto connected_promise = std::make_shared<std::promise<void>>();
    auto connected_future = connected_promise->get_future();

    auto connect_result = http2_client->connect(
        engine_endpoint, leverager, contract_group, verify_peer,
        [connected_promise, http2_client, manager, &engine_api_client] {
            // Point both correlators — the worker-manager's own WorkerContext::call_engine() and
            // the generated congelado_api::* typed calls — at this connection. Each owns its own
            // core::client::Register; make_engine_dispatch() above routes every incoming response
            // to both, correlated by stream id.
            manager->set_runtime(*http2_client);
            engine_api_client.setRuntime(*http2_client);
            connected_promise->set_value();
        });
    if (!connect_result) {
        std::println(stderr, "[worker] {}", connect_result.error());
        return 1;
    }

    bool connected = false;
    while (!connected) {
        if (connected_future.wait_for(retry_delay) == std::future_status::ready) {
            connected = true;
            break;
        }
        if (!retry_forever && std::chrono::steady_clock::now() >= connect_deadline) {
            std::println(stderr, "[worker] gave up connecting to engine at {}:{} after {}ms",
                         cfg.getEngineHost(), cfg.getEnginePort(), connect_timeout_ms);
            return 1;
        }
        std::println("[worker] engine connect attempt failed, retrying in {}ms",
                     retry_delay.count());
        auto retry_result = http2_client->retry();
        if (!retry_result) {
            std::println(stderr, "[worker] {}", retry_result.error());
            return 1;
        }
    }
    std::println("[worker] connected to engine at {}:{}", cfg.getEngineHost(), cfg.getEnginePort());

    // Stand up the worker's own inbound API server now the engine link is up, workers are
    // registered, and the serde TOML format is loaded (start_server parses the worker config).
    manager->start_server();

    // ── 3b. Register app-supplied defs (task + workflow) via the manager, over the live engine
    // connection. Task defs before workflows so a workflow's referenced task defs already exist.
    auto app_task_defs = collect_app_defs(apps_dir, "taskdefs");
    auto app_workflow_defs = collect_app_defs(apps_dir, "workflows");
    // Merge the code-built (IAppDefs) defs collected above with the def-file defs — both authoring
    // paths register the same way.
    app_task_defs.insert(app_task_defs.end(), std::make_move_iterator(builder_task_defs.begin()),
                         std::make_move_iterator(builder_task_defs.end()));
    app_workflow_defs.insert(app_workflow_defs.end(),
                             std::make_move_iterator(builder_workflow_defs.begin()),
                             std::make_move_iterator(builder_workflow_defs.end()));
    std::println("[worker] registering {} task defs, {} workflow defs from apps",
                 app_task_defs.size(), app_workflow_defs.size());
    manager->register_task_defs(app_task_defs);
    manager->register_workflow_defs(app_workflow_defs);

    // ── 4. Self-register configured tasks so the engine queue-claim JOIN has something to match ──
    for (const auto &task_cfg : cfg.getTasks()) {
        congelado_api_dto::TaskDef task_def;
        task_def.setName(task_cfg.getName());
        task_def.setWorkerType(task_cfg.getWorkerType());
        task_def.setType("SIMPLE");
        auto reg_result = call_typed_blocking<congelado_api_dto::TaskDef>(
            [&task_def, &engine_api_client](auto onResponse, auto onError) {
                engine_api_client.tasks_post(task_def, std::move(onResponse), std::move(onError));
            });
        if (!reg_result) {
            std::println(stderr, "[worker] task self-registration failed for '{}': {}",
                         task_cfg.getName(), reg_result.error());
        } else {
            std::println("[worker] registered task '{}'", task_cfg.getName());
        }
    }

    // ── 5. Register poll contracts — each PollSlotHandler drives one manager->poll_slot() slot.
    // poll_slot() itself decides whether to re-arm immediately (idle/resumed) or park (dispatched an
    // async task, resumed later via the wake callback registered below) — unlike the old
    // poll_once(), this loop must NOT unconditionally reschedule.
    std::println("[worker] registering {} poll contracts...", cfg.getConcurrency());
    std::vector<std::unique_ptr<PollSlotHandler>> poll_handlers;
    poll_handlers.reserve(cfg.getConcurrency());
    for (std::uint32_t slot = 0; slot < cfg.getConcurrency(); ++slot) {
        auto handler = std::make_unique<PollSlotHandler>(*manager, slot);
        auto contract = handler->create(contract_group, core::contract::ContractState::SCHEDULED);
        manager->register_poll_slot(slot, [contract]() mutable { contract.schedule(); });
        poll_handlers.push_back(std::move(handler));
    }

    // ── 6. Wait for shutdown ──────────────────────────────────────────
    std::signal(SIGINT, request_shutdown);
    std::signal(SIGTERM, request_shutdown);
    std::println("[worker] running. Press Ctrl+C to stop.");
    while (!g_shutdown_requested.load(std::memory_order_relaxed)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    std::println("[worker] shutdown requested — tearing down");
    manager->shutdown_all();
    return 0;
}

} // namespace

int main(int argc, char *argv[]) {
    try {
        return run_worker(argc, argv);
    } catch (const std::exception &e) {
        try {
            std::println(stderr, "[worker] fatal: {}", e.what());
        } catch (...) { // NOLINT(bugprone-empty-catch) — best-effort diagnostic only
        }
        return 1;
    } catch (...) {
        try {
            std::println(stderr, "[worker] fatal: unknown exception");
        } catch (...) { // NOLINT(bugprone-empty-catch) — best-effort diagnostic only
        }
        return 1;
    }
}
