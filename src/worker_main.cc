#include "backward.hpp"
#include <csignal>

import std;
import worker;
import congelado_worker;
import interfaces;
import io_layer_http2;
import core_config;
import core_logger;
import core_router;
import utils_openapi;
import io_base_socket;
import io_base_leverage;
import io_flow_socket;
import core_contract;
import shared;
import serde;
import model;

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

/**
 * @brief Hand-rolls the JSON body submitted back to the engine after a task runs — mirrors
 * `worker::PollHandler::build_submit_json`, kept separate since this loop runs outside the
 * HTTP handler surface entirely.
 * @param result the task outcome to serialize as a string.
 * @param data the output key/value pairs to serialize into `output_data`.
 * @return the JSON string, ready to POST as-is.
 */
std::string build_submit_json(model::TaskResult result,
                              const std::unordered_map<std::string, std::string> &data) {
    std::string_view result_str;
    switch (result) {
    case model::TaskResult::SUCCESS:
        result_str = "SUCCESS";
        break;
    case model::TaskResult::FAILURE:
        result_str = "FAILURE";
        break;
    case model::TaskResult::TIMEOUT:
        result_str = "TIMEOUT";
        break;
    case model::TaskResult::SKIPPED:
        result_str = "SKIPPED";
        break;
    }

    std::string json = std::format(R"({{"result":"{}","output_data":{{)", result_str);
    bool first = true;
    for (const auto &[key, value] : data) {
        if (!first) {
            json += ',';
        }
        json += std::format(R"("{}":"{}")", key, value);
        first = false;
    }
    json += "}}";
    return json;
}

/**
 * @brief One poll-execute-submit cycle for every task type this worker has registered — the
 * body of each poll thread's loop, factored out so `main()` just spins threads over it.
 * @param ctx the worker context to poll/execute/submit through.
 */
void poll_cycle(worker::WorkerContext &ctx) {
    for (auto type : ctx.get_task_types()) {
        worker::WorkerContext::EngineResponse engine_res;
        try {
            engine_res = ctx.call_engine("GET", std::format("/api/v1/tasks/queue/{}", type));
        } catch (const std::exception &e) {
            core::logger::error("worker/poll-thread", "poll exception type={}: {}", type, e.what());
            continue;
        }

        // Empty queue for this type — nothing to do, move on to the next registered type.
        if (engine_res.m_status == 204) {
            continue;
        }
        if (engine_res.m_status != 200) {
            core::logger::error("worker/poll-thread", "poll failed type={} status={}", type,
                                engine_res.m_status);
            continue;
        }

        auto parsed = serde::Json::decode<model::TaskInstance>(engine_res.m_body);
        if (!parsed.has_value()) {
            core::logger::error("worker/poll-thread", "parse failed: {}", parsed.error());
            continue;
        }

        auto &instance = *parsed;
        worker::TaskInput input{instance.get_input_data()};
        auto output_opt = ctx.run_task(instance.get_def_name(), input);

        auto result =
            output_opt.has_value() ? model::TaskResult::SUCCESS : model::TaskResult::FAILURE;
        const auto &output_data =
            output_opt ? output_opt->get_data() : std::unordered_map<std::string, std::string>{};

        auto task_id = std::format("{}", instance.get_task_id());
        auto submit_body = build_submit_json(result, output_data);

        try {
            auto submit_res =
                ctx.call_engine("POST", "/api/v1/tasks/" + task_id + "/result", submit_body);
            if (submit_res.m_status != 200) {
                core::logger::error("worker/poll-thread", "submit {} failed status={}", task_id,
                                    submit_res.m_status);
            }
        } catch (const std::exception &e) {
            core::logger::error("worker/poll-thread", "submit exception: {}", e.what());
        }
    }
}

/**
 * @brief Worker process body — loads config, loads task-worker plugins, connects to the engine
 * over HTTP/2, binds the worker's HTTP handlers, and runs one poll thread per configured
 * concurrency slot until killed.
 * @param argc argument count.
 * @param argv `argv[1]` (optional) is the worker config path, defaults to `"worker.toml"`;
 * `argv[2]` (optional) is the task-worker plugin directory, defaults to `"./workers"`.
 * @return `0` on a clean shutdown, `1` on config load or task-worker load failure.
 */
int run_worker(int argc, char *argv[]) {
    backward::SignalHandling sh;

    // The engine (or a dropped connection) can close its end mid-write, raising SIGPIPE on the
    // next send — default disposition kills the whole process. Ignore it; call_engine()'s own
    // try/catch around the send already handles a failed write via the thrown exception path.
    std::signal(SIGPIPE, SIG_IGN);

    auto config_path = argc > 1 ? std::string{argv[1]} : "worker.toml";
    auto workers_dir = argc > 2 ? std::string{argv[2]} : "./workers";

    // ── 1. Load worker config ──────────────────────────────────────────
    auto cfg_result = congelado::worker::WorkerConfig::from_file(expand_tilde(config_path));
    if (!cfg_result) {
        std::println(stderr, "[worker] config load failed: {}", cfg_result.error());
        return 1;
    }
    auto &cfg = *cfg_result;

    std::println("[worker] loaded config: id='{}' engine={}:{} concurrency={}",
                 cfg.getWorkerId(), cfg.getEngineHost(), cfg.getEnginePort(),
                 cfg.getConcurrency());

    // ── 2. Create worker context ───────────────────────────────────────
    worker::WorkerContext ctx(cfg.getWorkerId());

    // ── 3. Load FFI task-worker plugins from directory ─────────────────
    try {
        ctx.load_workers(workers_dir);
    } catch (const std::exception &e) {
        std::println(stderr, "[worker] failed to load workers from '{}': {}", workers_dir,
                     e.what());
        return 1;
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
    core::contract::ContractThreadPool<> thread_pool(contract_group, 4);

    // Http2Protocol requires non-empty cert/key even for an outbound client connection
    // (which never reads them) — fall back to the repo-root dev certs when the config
    // leaves them empty, rather than hitting the ctor's hard throw.
    core::config::PluginConfig http2_cfg;
    http2_cfg.add_field("host", cfg.getEngineHost());
    http2_cfg.add_field("port", std::to_string(cfg.getEnginePort()));
    http2_cfg.add_field("cert", cfg.getEngineCert().empty() ? "server.crt" : cfg.getEngineCert());
    http2_cfg.add_field("key", cfg.getEngineKey().empty() ? "server.key" : cfg.getEngineKey());
    http2_cfg.add_field("threads", "1");

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
    try {
        // Only wire the engine pointer into ctx once the connect/handshake actually lands —
        // connect() kicks the async sequence off and returns immediately, well before that.
        // Registering it any earlier would let a poll thread call send() while m_flow is still
        // null (straight segfault, not a clean "no engine set" error).
        http2_client->connect(engine_endpoint, leverager, contract_group, verify_peer,
                              [&ctx, http2_client] { ctx.set_engine(*http2_client); });
    } catch (const std::exception &e) {
        std::println(stderr, "[worker] failed to connect to engine at {}:{}: {}",
                     cfg.getEngineHost(), cfg.getEnginePort(), e.what());
        return 1;
    }

    std::println("[worker] connected to engine at {}:{}", cfg.getEngineHost(),
                 cfg.getEnginePort());

    // ── 5. Bind handlers ──────────────────────────────────────────────
    worker::PollHandler::bind(ctx, *engine_client);
    worker::ExecutionHandler::bind(ctx, *engine_client);
    worker::StatusHandler::bind(ctx);

    // ── 6. Stand up the worker's own inbound HTTP/2 server ─────────────
    // Separate PluginConfig/Http2Protocol from the outbound one above — get_server()'s bind
    // address is this worker's own listen address, unrelated to the engine endpoint the
    // outbound Client::connect() call above already targets explicitly.
    core::router::RouterContext<> router;

    auto generator = utils::openapi::Generator{}.title("Congelado Worker API").version("1.0.0");
    // Registered before register_routes() so the trie the protocol compiles eagerly
    // already has it — same ordering constraint congelado_heart::ServerRunner documents
    // for the engine's own openapi.json route.
    router.add_route(generator.serve());
    worker::register_routes(router);

    core::config::PluginConfig server_cfg;
    server_cfg.add_field("host", cfg.getBindHost());
    server_cfg.add_field("port", std::to_string(cfg.getBindPort()));
    server_cfg.add_field("cert", cfg.getEngineCert().empty() ? "server.crt" : cfg.getEngineCert());
    server_cfg.add_field("key", cfg.getEngineKey().empty() ? "server.key" : cfg.getEngineKey());
    server_cfg.add_field("threads", "1");

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
        std::println(stderr, "[worker] failed to bind own server at {}:{}: {}",
                     cfg.getBindHost(), cfg.getBindPort(), e.what());
        return 1;
    }

    std::println("[worker] listening on {}:{}", cfg.getBindHost(), cfg.getBindPort());

    if (auto write_res = generator.write(generator.generate()); !write_res) {
        std::println(stderr, "[worker] failed to write openapi document: {}", write_res.error());
    } else {
        std::println("[worker] generated openapi document");
    }

    // ── 7. Run parallel poll loop ─────────────────────────────────────
    std::println("[worker] starting {} poll threads...", cfg.getConcurrency());

    std::vector<std::jthread> threads;
    threads.reserve(cfg.getConcurrency());
    for (std::uint32_t i = 0; i < cfg.getConcurrency(); ++i) {
        threads.emplace_back([&ctx, i](const std::stop_token &stoken) {
            std::println("[worker/thread-{}] started", i);

            while (!stoken.stop_requested()) {
                poll_cycle(ctx);
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }

            std::println("[worker/thread-{}] stopped", i);
        });
    }

    // ── 8. Wait for shutdown ──────────────────────────────────────────
    std::println("[worker] running. Press Ctrl+C to stop.");
    std::promise<void>().get_future().wait();

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
