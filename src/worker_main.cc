#include "backward.hpp"

#include <congelado/abi.h>
#include <csignal>

import std;
import worker;
import congelado_worker;
import congelado_heart;
import interfaces;
import io_layer_http2;
import core_config;
import core_logger;
import core_otel;
import core_plugin;
import core_router;
import core_events;
import utils_openapi;
import io_base_socket;
import io_base_leverage;
import io_flow_socket;
import core_contract;
import shared;
import serde;
import model;
import congelado_client;
import congelado_api_dto;
import congelado_api_routes;

namespace {

std::filesystem::path expand_tilde(const std::filesystem::path &path) {
    std::string path_str = path.string();
    // Only a leading '~' gets expanded — nothing to do for a path that doesn't start with one.
    if (!path_str.empty() && path_str.front() == '~') {
        // No HOME env var means no expansion happens — path_str is left with the literal '~'.
        // getenv has no portable thread-safe alternative in std C++; only called here at boot.
        // NOLINTNEXTLINE(concurrency-mt-unsafe)
        const char *home = std::getenv("HOME");
        if (home != nullptr) {
            path_str.replace(0, 1, home);
        }
    }
    return {path_str};
}

// Set only from inside request_shutdown() below — a plain atomic<bool>, not a std::promise,
// since std::signal handlers may only touch lock-free atomics/sig_atomic_t per the standard's
// async-signal-safety rules.
std::atomic<bool> g_shutdown_requested{false};

/// @brief SIGINT/SIGTERM handler — signals the wait loop at the bottom of `run_worker()` to fall
/// through so the (otherwise infinite) function returns and every local's destructor actually
/// runs, including plugin_store's — that's what calls congelado_on_unload() on
/// libotel_otlp_plugin.so, flushing its batched exporters — instead of the process just getting
/// killed mid-flight.
void request_shutdown(int /*signal*/) noexcept {
    g_shutdown_requested.store(true, std::memory_order_relaxed);
}

/**
 * @brief Shape-matches `engine::TaskSubmitBody` (`plugins/engine/core/handler/task.cppm`) field
 * for field — no shared header needed, `serde::Ser::serialize` reduces this via rfl's native
 * reflection off these exact public member names, same struct-shape-duplication pattern used
 * for `SqlRequest`/`Document` elsewhere in this codebase. Kept local rather than routed through
 * the generated `congelado_api_dto::TaskSubmitBody`: that generated DTO's `output_data` is
 * `std::string`, not a map — the client codegen collapses any raw `map<string,string>` schema
 * property down to `std::string` (`dto_writer.cppm`'s `resolve_cpp_type`, `SchemaKind::OBJECT`
 * case), so it can't actually carry this field's real shape.
 */
struct SubmitResultBody {
    model::TaskResult result{model::TaskResult::SUCCESS};
    std::unordered_map<std::string, std::string> output_data;
};

/**
 * @brief Builds the JSON body submitted back to the engine after a task runs.
 * @param result the task outcome.
 * @param data the output key/value pairs to submit as `output_data`.
 * @return the serialized JSON body, ready to POST as-is.
 */
std::string build_submit_json(model::TaskResult result,
                              std::unordered_map<std::string, std::string> data) {
    SubmitResultBody body{.result = result, .output_data = std::move(data)};
    auto encoded = serde::Ser::serialize("application/json", body);
    return {reinterpret_cast<const char *>(encoded.data()), encoded.size()};
}

/**
 * @brief One poll-execute-submit cycle for every task type this worker has registered — the
 * body of each poll thread's loop, factored out so `main()` just spins threads over it.
 * @param ctx the worker context to poll/execute/submit through.
 */
void poll_cycle(worker::WorkerContext &ctx) {
    for (auto type : ctx.get_task_types()) {
        // Fresh root span per poll attempt — CONSUMER kind, matching OTel's own convention for
        // "picked work off a queue" spans. Deliberately NOT continued from any prior context:
        // the contract that re-invokes poll_cycle() every ~100ms has no ambient carry-over
        // between reschedules (each is its own fresh contract-worker invocation), so there's no
        // meaningful parent to inherit here anyway. Nested call_engine() calls below pick this
        // up automatically as their parent via the ambient stack.
        auto cycle_span = core::otel::start_span(std::format("poll_cycle {}", type),
                                                 interfaces::SpanKind::CONSUMER);

        worker::WorkerContext::EngineResponse engine_res;
        try {
            engine_res = ctx.call_engine("GET", std::format("/api/v1/tasks/queue/{}", type));
        } catch (const std::exception &e) {
            core::logger::error("worker/poll-thread", "poll exception type={}: {}", type, e.what());
            core::events::publish("worker.poll_thread.exception",
                                  {{"type", std::string{type}}, {"error", e.what()}});
            cycle_span.set_status(interfaces::SpanStatus::ERROR, e.what());
            continue;
        }

        // Empty queue for this type — nothing to do, move on to the next registered type.
        if (engine_res.m_status == 204) {
            cycle_span.set_status(interfaces::SpanStatus::OK, "");
            continue;
        }
        if (engine_res.m_status != 200) {
            core::logger::error("worker/poll-thread", "poll failed type={} status={}", type,
                                engine_res.m_status);
            core::events::publish(
                "worker.poll_thread.poll_failed",
                {{"type", std::string{type}}, {"status", std::to_string(engine_res.m_status)}});
            cycle_span.set_status(interfaces::SpanStatus::ERROR, "");
            continue;
        }

        auto parsed =
            serde::Ser::deserialize<model::TaskInstance>("application/json", engine_res.m_body);
        if (!parsed.has_value()) {
            core::logger::error("worker/poll-thread", "parse failed: {}", parsed.error());
            core::events::publish("worker.poll_thread.parse_failed", {{"error", parsed.error()}});
            cycle_span.set_status(interfaces::SpanStatus::ERROR, parsed.error());
            continue;
        }

        auto &instance = *parsed;
        worker::TaskInput input{instance.get_input_data()};
        auto exec_start = std::chrono::steady_clock::now();
        auto output_opt = ctx.run_task(instance.get_def_name(), input);
        auto exec_ms =
            std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - exec_start)
                .count();

        auto result =
            output_opt.has_value() ? model::TaskResult::SUCCESS : model::TaskResult::FAILURE;
        const auto &output_data =
            output_opt ? output_opt->get_data() : std::unordered_map<std::string, std::string>{};

        std::array<interfaces::Attribute, 2> metric_attrs{
            interfaces::Attribute{"task.type", type},
            interfaces::Attribute{"task.result", result == model::TaskResult::SUCCESS
                                                     ? std::string_view{"success"}
                                                     : std::string_view{"failure"}},
        };
        core::otel::counter_add("task.completed", 1.0, metric_attrs);
        core::otel::histogram_record("task.duration_ms", exec_ms, metric_attrs);

        auto task_id = std::format("{}", instance.get_task_id());
        auto submit_body = build_submit_json(result, output_data);

        try {
            auto submit_res =
                ctx.call_engine("POST", "/api/v1/tasks/" + task_id + "/result", submit_body);
            if (submit_res.m_status != 200) {
                core::logger::error("worker/poll-thread", "submit {} failed status={}", task_id,
                                    submit_res.m_status);
                core::events::publish(
                    "worker.poll_thread.submit_failed",
                    {{"task_id", task_id}, {"status", std::to_string(submit_res.m_status)}});
                cycle_span.set_status(interfaces::SpanStatus::ERROR, "");
            } else {
                cycle_span.set_status(interfaces::SpanStatus::OK, "");
            }
        } catch (const std::exception &e) {
            core::logger::error("worker/poll-thread", "submit exception: {}", e.what());
            core::events::publish("worker.poll_thread.submit_exception", {{"error", e.what()}});
            cycle_span.set_status(interfaces::SpanStatus::ERROR, e.what());
        }
    }
}

/**
 * @brief Worker process body — loads config, loads task-worker plugins, connects to the engine
 * over HTTP/2, binds the worker's HTTP handlers, and runs one poll thread per configured
 * concurrency slot until killed.
 * @param argc argument count.
 * @param argv `argv[1]` (optional) is the worker config path, defaults to `"worker.toml"`;
 * `argv[2]` (optional) is the internal (built-in) task-worker plugin directory, defaults to
 * `"./workers"` — should not normally be overridden; `argv[3]` (optional) is the external
 * (user-provided, custom) task-worker plugin directory, the intended user-facing knob.
 * @return `0` on a clean shutdown, `1` on config load or task-worker load failure.
 */
int run_worker(int argc, char *argv[]) {
    backward::SignalHandling sh;

    // The engine (or a dropped connection) can close its end mid-write, raising SIGPIPE on the
    // next send — default disposition kills the whole process. Ignore it; call_engine()'s own
    // try/catch around the send already handles a failed write via the thrown exception path.
    std::signal(SIGPIPE, SIG_IGN);

    // Default config for a no-arg `xmake run engine_worker`. The docker worker passes its own
    // config path explicitly (Dockerfile.worker copies config/worker.toml -> ./worker.toml and
    // runs it), so it never relies on this default.
    auto config_path = argc > 1 ? std::string{argv[1]} : "~/cc/congelado/config/worker.toml";
    auto internal_workers_dir = argc > 2 ? std::string{argv[2]} : "./workers";
    auto external_workers_dir = argc > 3 ? std::optional<std::string>{argv[3]} : std::nullopt;

    // ── 0. Force-load plugins before config parsing ────────────────────
    // WorkerConfig::from_file (step 1 below) decodes worker.toml via serde::Ser, which
    // dispatches to whatever TOML format plugin is registered in
    // serde::SerdeFormatRegistry — nothing is compiled in by default. This has to run first,
    // unconditionally, so that registration exists before config parsing needs it; it can't
    // depend on config itself (that's exactly the chicken-and-egg this step avoids). Same
    // standard plugins directory ServerRunner (the engine binary) already scans, derived the
    // same way main.cc derives it: relative to this binary's own path. plugin_store is
    // function-scope (not a nested block) so the dlopen handles it owns — and the format
    // pointers registered from them — stay alive for the rest of the process; SharedLibrary's
    // destructor dlcloses everything, so it must outlive every use of those pointers.
    // Function-scope, not a nested block — this is the process's one SerdeFormatRegistry
    // instance, and set_active() below points the ambient serde::Ser facade at it for the
    // rest of the process's lifetime.
    serde::SerdeFormatRegistry serde_format_registry;
    serde::SerdeFormatRegistry::set_active(&serde_format_registry);

    // Per-plugin config (e.g. otel_otlp_plugin's endpoint/service_name) for the plugin.build()
    // call below — sidesteps the same chicken-and-egg differently than the comment above:
    // core::config::load() reads worker.toml's [plugins.*] tables via a built-in, non-plugin
    // tomlplusplus parser (the exact same one sdk/heart/app.cppm uses for congelado.toml), so
    // it needs no serde plugin loaded first and can run before plugin_store.build() below.
    // Previously this call site always passed an empty config, meaning every plugin loaded here
    // (including otel_otlp_plugin) silently fell back to its hardcoded defaults
    // (localhost:4318/service_name="congelado") regardless of what worker.toml said — a real gap,
    // not a deliberate limitation. A missing/malformed worker.toml here just means every plugin
    // gets empty config, same graceful fallback as before this fix — step 1 below is what
    // actually hard-fails on a bad config file, this read is best-effort.
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

    // Pre-existing gap, fixed alongside adding OTel: the worker never activated a
    // LoggerRegistry at all — every core::logger::* call in this process silently fell back to
    // bare stderr, never reaching a real logger backend even when one (e.g. file_logger) was
    // available in the same plugins directory the engine scans. Also required for the OTel log
    // bridge below to have any effect here, since it registers as a core::logger backend.
    core::logger::LoggerRegistry logger_registry;
    core::logger::LoggerRegistry::set_active(&logger_registry);

    // OTel is optional, same as in the engine — no provider plugin found just means every
    // core::otel::* facade call degrades to a silent no-op.
    core::otel::TracerRegistry tracer_registry;
    core::otel::MeterRegistry meter_registry;
    core::otel::LogRecordRegistry log_record_registry;
    core::otel::TracerRegistry::set_active(&tracer_registry);
    core::otel::MeterRegistry::set_active(&meter_registry);
    core::otel::LogRecordRegistry::set_active(&log_record_registry);

    // utils::openapi::Generator moved out of this process's direct reach and into
    // plugins/openapi_generator/ (see interfaces::IOpenApiGenerator) — resolved the same
    // best-effort way as the serde/logger/otel capabilities just below: this worker treats
    // every plugin dependency here as optional-with-a-warning (see the loop's own comment), not
    // mandatory-or-abort like congelado_heart::ServerRunner, so a missing generator plugin just
    // means the worker's own /openapi.json route/file don't materialize, not a startup failure.
    utils::openapi::OpenApiGeneratorRegistry openapi_generator_registry;

    auto plugin_base =
        argc > 0 ? std::filesystem::path(argv[0]).parent_path() : std::filesystem::path{};
    auto plugins_dir =
        std::filesystem::path{std::format("{}/../../../plugins", plugin_base.string())};
    core::plugin::SharedLibrary plugin_store{"plugin"};
    plugin_store.scan(plugins_dir);
    // Only the plugins this process actually needs — a serde format to decode worker.toml
    // (application/toml), optionally an OTel provider for tracing, and the OpenAPI generator
    // for this worker's own /openapi.json — not open_all(), which previously dlopen'd every
    // plugin under plugins_dir indiscriminately: http2, postgres, sql_postgres, python_bridge,
    // lua_bridge, ... none of which a worker process has any business loading. Each carries its
    // own global init state (io_uring rings, embedded interpreter state, etc.) that this process
    // never tears down before exit, which is what LeakSanitizer was actually reporting — nothing
    // to do with this process's own logic.
    for (const char *plugin_name : {"libtoml_plugin.so", "libotel_otlp_plugin.so",
                                    "libfile_logger.so", "libopenapi_generator.so",
                                    "libjson_plugin.so"}) {
        if (auto open_res = plugin_store.open(plugins_dir / plugin_name); !open_res) {
            std::println(stderr, "[worker] plugin '{}' load failed: {}", plugin_name,
                         open_res.error().get_message());
        }
    }
    CongeladoHostCallbacks empty_host_cb{};
    if (auto build_res = plugin_store.build(empty_host_cb, plugin_configs); build_res) {
        plugin_store.for_each([&serde_format_registry, &logger_registry, &tracer_registry,
                               &meter_registry, &log_record_registry, &openapi_generator_registry](
                                  const std::shared_ptr<core::plugin::FfiRuntime> &runtime) {
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
                    log_record_registry.add_provider(
                        std::shared_ptr<interfaces::ILogRecordProvider>(
                            log_provider, [](interfaces::ILogRecordProvider *) {}));
                }
            }
            if (auto generator = congelado::heart::resolve_openapi_generator(*plugin)) {
                openapi_generator_registry.add_generator(std::move(generator));
            }
        });
    } else {
        std::println(stderr, "[worker] plugin build failed: {}", build_res.error().get_message());
    }
    if (!openapi_generator_registry.has_generator()) {
        std::println(stderr, "[worker] no OpenAPI generator plugin found — "
                             "/openapi.json route/file will not be available");
    }

    // Construct+register the OTel log bridge unconditionally — same graceful-no-op contract as
    // the engine's copy of this: OtelLogBridge::emit() silently no-ops if log_record_registry
    // ends up empty.
    congelado::heart::OtelLogBridge::install(logger_registry);

    // ── 1. Load worker config ──────────────────────────────────────────
    auto cfg_result = congelado::worker::WorkerConfig::from_file(expand_tilde(config_path));
    if (!cfg_result) {
        throw std::runtime_error(std::format("config load failed: {}", cfg_result.error()));
    }
    auto &cfg = *cfg_result;

    std::println("[worker] loaded config: id='{}' engine={}:{} concurrency={}", cfg.getWorkerId(),
                 cfg.getEngineHost(), cfg.getEnginePort(), cfg.getConcurrency());

    // ── 2. Create worker context ───────────────────────────────────────
    worker::WorkerContext ctx(cfg.getWorkerId());

    // ── 3. Load FFI task-worker plugins from directory ─────────────────
    try {
        ctx.load_workers(external_workers_dir, internal_workers_dir);
    } catch (const std::exception &e) {
        throw std::runtime_error(
            std::format("failed to load workers from '{}' (internal) / '{}' (external): {}",
                        internal_workers_dir, external_workers_dir.value_or("<none>"), e.what()));
    }

    auto task_types = ctx.get_task_types();
    std::print("[worker] loaded {} task types:", task_types.size());
    for (auto type : task_types) {
        std::print(" {}", type);
    }
    std::println();

    // ── 4. Set up engine connection via HTTP/2 ─────────────────────────
    core::contract::ContractGroup<> contract_group;
    io::base::leverage::Leverager<io::base::leverage::Context> leverager;
    // More than 1 thread is load-bearing here, not just extra throughput: this same
    // contract_group now pumps both the outbound engine connection AND the worker's own
    // inbound server (added below). An inbound route handler that itself calls
    // call_engine() (e.g. ExecutionHandler::list_executions) blocks on a future that only
    // resolves once the outbound response gets pumped through — with a single thread,
    // that pumping never happens because the one thread is the one sitting in the block,
    // exactly the deadlock call_engine()'s own doc comment warns about. A second thread is
    // what lets the block actually get serviced.
    // On top of that baseline, every poll-cycle contract (registered in step 7 below) also
    // blocks its pool thread for the duration of its call_engine() round-trips plus its
    // 100ms pacing sleep — so the pool needs headroom beyond the connection-pumping baseline,
    // or poll contracts and inbound dispatch pumping end up starving each other. That total is
    // the top-level `threads` config key now (see WorkerConfig::getThreads), sized to cover the
    // connection-pumping baseline plus one thread per concurrent poll-cycle — must stay > 1.
    core::contract::ContractThreadPool<> thread_pool(
        contract_group, cfg.getThreads().value_or(std::thread::hardware_concurrency()));

    // Http2Protocol requires non-empty cert/key even for an outbound client connection
    // (which never reads them) — fall back to the repo-root dev certs when the config
    // leaves them empty, rather than hitting the ctor's hard throw.
    core::config::PluginConfig http2_cfg;
    http2_cfg.add_field("host", cfg.getEngineHost());
    http2_cfg.add_field("port", std::to_string(cfg.getEnginePort()));
    http2_cfg.add_field("cert", cfg.getEngineCert().empty() ? "server.crt" : cfg.getEngineCert());
    http2_cfg.add_field("key", cfg.getEngineKey().empty() ? "server.key" : cfg.getEngineKey());

    // This worker never registers extensions — the Server's own empty registry makes every
    // extension seam a no-op, same as if the mechanism didn't exist.
    auto protocol = io::layer::http2::Http2Protocol{&http2_cfg};

    // Create dispatch callback that routes engine responses to pending promises
    auto dispatch = ctx.make_dispatch();

    // Create the http2 client
    auto engine_client = protocol.get_client(std::move(dispatch));

    // Empty engine_cert/engine_key means "no CA to trust" — per worker.toml's own documented
    // contract ("empty strings disable TLS verification"), skip peer verification rather than
    // failing every handshake against a self-signed/dev engine cert.
    bool verify_peer = !cfg.getEngineCert().empty() || !cfg.getEngineKey().empty();

    // Connect through the Client abstraction itself — it owns the socket transport
    // (ClientFlowSocket) internally, so this code never touches that type directly.
    io::base::socket::Endpoint engine_endpoint{cfg.getEngineHost(),
                                               static_cast<std::uint16_t>(cfg.getEnginePort())};
    auto *http2_client = dynamic_cast<io::layer::http2::Client *>(engine_client.get());
    if (http2_client == nullptr) {
        std::println(stderr, "[worker] engine client is not an HTTP/2 client");
        return 1;
    }
    // Set once the connect callback (below) fires — waited on by the main thread before
    // touching ClientRuntime, never inside the callback itself: that callback runs on one of
    // the http2 client's own I/O threads, and blocking there on a response this exact
    // connection needs to pump back (call_typed_blocking's future.get()) is a re-entrancy
    // hazard — the read that would resolve the promise may never get serviced by that same
    // thread. Blocking the main thread instead is safe: it isn't part of the contract group's
    // thread pool that services the connection.
    // The engine may not be reachable the instant this worker boots — DNS not yet resolvable, the
    // engine still coming up, a transient network blip. Rather than failing on the first attempt,
    // retry on a fixed delay until the connect lands or an overall deadline elapses. Both knobs
    // come from config (connect_retry_delay_ms / connect_timeout_ms), defaulted here.
    const auto retry_delay = std::chrono::milliseconds{
        cfg.getConnectRetryDelayMs().value_or(congelado::worker::consts::connect_retry_delay_ms)};
    // Sentinel: connect_timeout_ms == 0 means "retry forever, never give up" — for a worker that
    // must wait out an engine that comes up arbitrarily late. This is the default when the key is
    // absent, so a worker waits for its engine indefinitely unless a finite deadline is set.
    const auto connect_timeout_ms =
        cfg.getConnectTimeoutMs().value_or(congelado::worker::consts::connect_timeout_ms);
    const bool retry_forever = connect_timeout_ms == 0;
    const auto connect_deadline =
        std::chrono::steady_clock::now() + std::chrono::milliseconds{connect_timeout_ms};

    // One promise for the whole connect/retry sequence, not one per attempt: connect() wires
    // add_on_accept() a single time, and every subsequent retry()/on_released()-driven attempt
    // reuses that same callback — whichever attempt eventually succeeds resolves this promise.
    auto connected_promise = std::make_shared<std::promise<void>>();
    auto connected_future = connected_promise->get_future();

    // Only wire the engine pointer into ctx once the connect/handshake actually lands —
    // connect() kicks the async sequence off and returns immediately, well before that.
    // Registering it any earlier would let a poll thread call send() while m_flow is still
    // null (straight segfault, not a clean "no engine set" error).
    auto connect_result = http2_client->connect(
        engine_endpoint, leverager, contract_group, verify_peer,
        [&ctx, connected_promise, http2_client] {
            ctx.set_engine(*http2_client);
            // Point the generated typed client (congelado_api::*) at the same transport
            // call_engine() already uses — both share one connection, correlated by stream id
            // via WorkerContext::make_dispatch() feeding both pending maps (see its own doc
            // comment).
            congelado::client::ClientRuntime::setClient(*http2_client);
            congelado::client::ClientRuntime::setRequestFactory(
                [http2_client](std::uint32_t stream_id) {
                    return http2_client->create_request(stream_id);
                });
            connected_promise->set_value();
        });
    if (!connect_result) {
        std::println(stderr, "[worker] {}", connect_result.error());
        return 1;
    }

    bool connected = false;
    while (!connected) {
        // connect()/retry() only kick off the async handshake — bound the wait so a stalled
        // attempt that never fires the callback still counts as a failed attempt.
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

    // ── 4.5. Self-register configured tasks ────────────────────────────
    // worker.toml's [[tasks]] entries — register each as a TaskDef on the engine so its
    // queue-claim JOIN has something to match. Best-effort: a failure here just means this
    // task type won't get claimed until it's registered some other way; the worker still
    // boots and polls normally either way.
    for (const auto &task_cfg : cfg.getTasks()) {
        congelado_api_dto::TaskDef task_def;
        task_def.setName(task_cfg.getName());
        task_def.setWorkerType(task_cfg.getWorkerType());
        task_def.setType("SIMPLE");
        auto reg_result = worker::WorkerContext::call_typed_blocking<congelado_api_dto::TaskDef>(
            [&task_def](auto onResponse, auto onError) {
                congelado_api::tasks::post(task_def, std::move(onResponse), std::move(onError));
            });
        if (!reg_result) {
            std::println(stderr, "[worker] task self-registration failed for '{}': {}",
                         task_cfg.getName(), reg_result.error());
        } else {
            std::println("[worker] registered task '{}'", task_cfg.getName());
        }
    }

    // ── 5. Bind handlers ──────────────────────────────────────────────
    worker::PollHandler::bind(ctx, *engine_client);
    worker::ExecutionHandler::bind(ctx, *engine_client);
    worker::StatusHandler::bind(ctx);

    // ── 6. Stand up the worker's own inbound HTTP/2 server ─────────────
    // Separate PluginConfig/Http2Protocol from the outbound one above — get_server()'s bind
    // address is this worker's own listen address, unrelated to the engine endpoint the
    // outbound Client::connect() call above already targets explicitly.
    core::router::RouterContext<> router;

    // Resolved above (best-effort, alongside the serde/logger/otel capabilities) — nullptr here
    // just means no OpenAPI generator plugin loaded, matching this file's existing
    // optional-with-a-warning severity for every other plugin dependency (unlike
    // congelado_heart::ServerRunner, which treats the same capability as mandatory-or-abort).
    interfaces::IOpenApiGenerator *openapi_generator =
        openapi_generator_registry.has_generator()
            ? openapi_generator_registry.get_generators().front().get()
            : nullptr;
    if (openapi_generator != nullptr) {
        // Registered before register_routes() so the trie the protocol compiles eagerly
        // already has it — same ordering constraint congelado_heart::ServerRunner documents
        // for the engine's own openapi.json route.
        router.add_route(openapi_generator->serve_document("Congelado Worker API", "1.0.0"));
    }
    worker::register_routes(router);

    core::config::PluginConfig server_cfg;
    server_cfg.add_field("host", cfg.getBindHost());
    server_cfg.add_field("port", std::to_string(cfg.getBindPort()));
    server_cfg.add_field("cert", cfg.getEngineCert().empty() ? "server.crt" : cfg.getEngineCert());
    server_cfg.add_field("key", cfg.getEngineKey().empty() ? "server.key" : cfg.getEngineKey());

    auto server_protocol = io::layer::http2::Http2Protocol{&server_cfg};
    auto server = server_protocol.get_server();
    server->build(&router);

    io::base::socket::Endpoint bind_endpoint{cfg.getBindHost(),
                                             static_cast<std::uint16_t>(cfg.getBindPort())};
    io::base::flow::sync::ServerFlowSocket<core::contract::ContractGroup<>,
                                           io::base::socket::Protocol::TLS>
        server_flow{bind_endpoint, leverager, contract_group};
    server_flow.add_on_accept(
        [&server](shared::SendCallback send, shared::CloseCallback close) -> shared::ReadCallback {
            return server->on_connect(std::move(send), std::move(close));
        });

    try {
        server_flow.build();
    } catch (const std::exception &e) {
        std::println(stderr, "[worker] failed to bind own server at {}:{}: {}", cfg.getBindHost(),
                     cfg.getBindPort(), e.what());
        return 1;
    }

    std::println("[worker] listening on {}:{}", cfg.getBindHost(), cfg.getBindPort());

    if (openapi_generator != nullptr) {
        if (auto write_res =
                openapi_generator->write_document("Congelado Worker API", "1.0.0", "openapi.json");
            !write_res) {
            std::println(stderr, "[worker] failed to write openapi document: {}",
                         write_res.error());
        } else {
            std::println("[worker] generated openapi document");
        }
    }

    // ── 7. Register poll-cycle contracts on the same contract_group/thread_pool ───────
    // Same scheduling primitive the engine/main server uses for its own I/O (see
    // Http2Plugin::on_load / ServerFlowSocket) instead of a bespoke std::jthread pool:
    // each slot is a self-rescheduling Contract, run by whichever thread_pool thread picks
    // it up next. No schedule-after-delay primitive exists on ContractGroup today, so
    // pacing is done the same way the raw-thread loop did it — block the pool thread on
    // the 100ms sleep, then re-arm via shared::this_handler::shedule() before returning.
    std::println("[worker] registering {} poll contracts...", cfg.getConcurrency());

    for (std::uint32_t i = 0; i < cfg.getConcurrency(); ++i) {
        contract_group.create(
            std::format("poll-{}", i),
            [&ctx, i]() {
                poll_cycle(ctx);
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                shared::this_handler::shedule();
            },
            nullptr,
            [i](std::exception_ptr eptr) {
                try {
                    std::rethrow_exception(eptr);
                } catch (const std::exception &e) {
                    core::logger::error("worker/poll", "poll-{} error: {}", i, e.what());
                    core::events::publish("worker.poll.contract_error",
                                          {{"slot", std::to_string(i)}, {"error", e.what()}});
                }
                // Errors don't auto-reschedule — re-arm explicitly so one bad cycle doesn't
                // permanently stop this slot from polling again.
                shared::this_handler::shedule();
            });
    }

    // ── 8. Wait for shutdown ──────────────────────────────────────────
    // Polling this flag instead of blocking forever on a future: a plain infinite
    // future.wait() never returns on SIGTERM/SIGINT (the default disposition just kills the
    // process outright), which skips every local destructor including plugin_store's — the one
    // that actually calls congelado_on_unload() and flushes OTel's batched exporters. Polling
    // instead of blocking on it keeps request_shutdown() a plain function, matching what
    // std::signal needs as a handler.
    std::signal(SIGINT, request_shutdown);
    std::signal(SIGTERM, request_shutdown);
    std::println("[worker] running. Press Ctrl+C to stop.");
    while (!g_shutdown_requested.load(std::memory_order_relaxed)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    std::println("[worker] shutdown requested — tearing down");

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
