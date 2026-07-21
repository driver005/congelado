#include "backward.hpp"

import std;
import worker;
import interfaces;
import core_plugin;
import io_layer_http2;
import core_config;
import core_logger;
import io_base_flow;
import io_base_socket;
import io_base_leverage;
import core_contract;
import shared;
import serde;
import model;

namespace {

std::filesystem::path expand_tilde(const std::filesystem::path &path) {
    std::string path_str = path.string();
    // Only a leading '~' gets expanded — nothing to do for a path that doesn't start with one.
    if (!path_str.empty() && path_str[0] == '~') {  // FIXME(clang-tidy): unchecked operator[], consider .at()
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

} // namespace

/**
 * @brief Worker process entry point.
 * @warning Every real code path here — config load, worker loading, engine connection setup,
 * poll threads, task dispatch — is commented out below. Right now this is a no-op that just
 * returns `0`; the actual worker daemon isn't wired up to run yet. Looks like a finished
 * entrypoint from the signature alone, but it's WIP, not an L in the shipped sense, just not
 * done cooking.
 * @param argc unused right now — every path that would consume it is commented out.
 * @param argv unused right now, same deal.
 * @return `0`, always — nothing here can currently fail since nothing here currently runs.
 */
int main(int argc, char *argv[]) {
    // backward::SignalHandling sh;
    //
    // auto config_path = argc > 1 ? std::string{argv[1]} : "worker.toml";
    // auto workers_dir = argc > 2 ? std::string{argv[2]} : "./workers";
    //
    // // ── 1. Load worker config ──────────────────────────────────────────
    // worker::WorkerConfig cfg;
    // try {
    //     cfg = worker::WorkerConfig::from_file(config_path);
    // } catch (const std::exception &e) {
    //     std::println(stderr, "[worker] config load failed: {}", e.what());
    //     return 1;
    // }
    //
    // std::println("[worker] loaded config: id='{}' engine={}:{} concurrency={}",
    //              cfg.get_worker_id(), cfg.get_engine_host(), cfg.get_engine_port(),
    //              cfg.get_concurrency());
    //
    // // ── 2. Create worker context ───────────────────────────────────────
    // worker::WorkerContext ctx(cfg.get_worker_id());
    //
    // // ── 3. Load FFI workers from directory ─────────────────────────────
    // worker::WorkerLoader loader;
    // try {
    //     loader.load_from_directory(workers_dir, ctx);
    // } catch (const std::exception &e) {
    //     std::println(stderr, "[worker] failed to load workers from '{}': {}", workers_dir,
    //                  e.what());
    //     return 1;
    // }
    //
    // auto task_types = ctx.get_task_types();
    // std::print("[worker] loaded {} task types:", task_types.size());
    // for (auto type : task_types)
    //     std::print(" {}", type);
    // std::println();
    //
    // // ── 4. Set up engine connection via HTTP/2 ─────────────────────────
    // core::contract::ContractGroup<> contract_group;
    // io::base::leverage::Leverager<io::base::leverage::Context> leverager;
    // core::contract::ContractThreadPool<> thread_pool(contract_group, 1);
    //
    // // Build Http2Protocol config from worker config
    // core::config::PluginConfig http2_cfg;
    // http2_cfg.add_field("host", std::string{cfg.get_engine_host()});
    // http2_cfg.add_field("port", std::to_string(cfg.get_engine_port()));
    // if (!cfg.get_engine_cert().empty())
    //     http2_cfg.add_field("cert", std::string{cfg.get_engine_cert()});
    // else
    //     http2_cfg.add_field("cert", "");
    // if (!cfg.get_engine_key().empty())
    //     http2_cfg.add_field("key", std::string{cfg.get_engine_key()});
    // else
    //     http2_cfg.add_field("key", "");
    // http2_cfg.add_field("threads", "1");
    //
    // auto protocol = io::layer::http2::Http2Protocol{&http2_cfg};
    //
    // // Create dispatch callback that routes engine responses to pending promises
    // auto dispatch = ctx.make_dispatch();
    //
    // // Create the http2 client
    // auto engine_client = protocol.get_client(std::move(dispatch));
    //
    // // Set up client socket flow to connect to engine
    // io::base::socket::Endpoint engine_endpoint{std::string{cfg.get_engine_host()},
    //                                             cfg.get_engine_port()};
    //
    // auto *client_flow = new io::base::flow::sync::ClientFlowSocket<
    //     core::contract::ContractGroup<>, io::base::socket::Protocol::TLS>(
    //     engine_endpoint, leverager, contract_group);
    //
    // client_flow->add_on_accept(
    //     [&ctx, &engine_client](
    //         shared::SendCallback send,
    //         shared::CloseCallback close) mutable -> shared::ReadCallback {
    //         // Wire the IClient to the socket transport
    //         ctx.set_engine(*engine_client);
    //
    //         // Establish HTTP/2 connection: sends preface, returns read callback
    //         return engine_client->on_connect(std::move(send), std::move(close));
    //     });
    //
    // client_flow->build();
    //
    // std::println("[worker] connected to engine at {}:{}", cfg.get_engine_host(),
    //              cfg.get_engine_port());
    //
    // // ── 5. Bind handlers ──────────────────────────────────────────────
    // worker::PollHandler::bind(ctx, *engine_client);
    // worker::ExecutionHandler::bind(ctx, *engine_client);
    // worker::StatusHandler::bind(ctx);
    //
    // // ── 6. Run parallel poll loop ─────────────────────────────────────
    // std::println("[worker] starting {} poll threads...", cfg.get_concurrency());
    //
    // std::vector<std::jthread> threads;
    // for (std::uint32_t i = 0; i < cfg.get_concurrency(); ++i) {
    //     threads.emplace_back([&ctx, i](std::stop_token stoken) {
    //         std::println("[worker/thread-{}] started", i);
    //
    //         while (!stoken.stop_requested()) {
    //             auto types = ctx.get_task_types();
    //             for (auto type : types) {
    //                 if (stoken.stop_requested())
    //                     break;
    //
    //                 // Poll for a task of this type
    //                 worker::WorkerContext::EngineResponse engine_res;
    //                 try {
    //                     engine_res = ctx.call_engine(
    //                         "GET", std::format("/api/v1/tasks/queue/{}", type));
    //                 } catch (const std::exception &e) {
    //                     core::logger::error("worker/poll-thread",
    //                                         "poll exception type={}: {}", type, e.what());
    //                     continue;
    //                 }
    //
    //                 if (engine_res.status == 204) {
    //                     // Empty queue — no tasks available
    //                     continue;
    //                 }
    //                 if (engine_res.status != 200) {
    //                     core::logger::error("worker/poll-thread",
    //                                         "poll failed type={} status={}", type,
    //                                         engine_res.status);
    //                     continue;
    //                 }
    //
    //                 // Parse TaskInstance
    //                 auto parsed = serde::Json::decode<model::TaskInstance>(engine_res.body);
    //                 if (!parsed.has_value()) {
    //                     core::logger::error("worker/poll-thread",
    //                                         "parse failed: {}", parsed.error());
    //                     continue;
    //                 }
    //
    //                 auto &instance = *parsed;
    //                 worker::TaskInput input{instance.get_input_data()};
    //                 auto output_opt = ctx.run_task(instance.get_def_name(), input);
    //
    //                 auto result = output_opt.has_value() ? model::TaskResult::SUCCESS
    //                                                      : model::TaskResult::FAILURE;
    //                 const auto &output_data =
    //                     output_opt ? output_opt->get_data()
    //                                : std::unordered_map<std::string, std::string>{};
    //
    //                 // Build submit JSON
    //                 std::string_view result_str = "SUCCESS";
    //                 if (result == model::TaskResult::FAILURE)
    //                     result_str = "FAILURE";
    //                 else if (result == model::TaskResult::TIMEOUT)
    //                     result_str = "TIMEOUT";
    //                 else if (result == model::TaskResult::SKIPPED)
    //                     result_str = "SKIPPED";
    //
    //                 std::string submit_body =
    //                     std::format("{{\"result\":\"{}\",\"output_data\":{{", result_str);
    //                 bool first = true;
    //                 for (const auto &[key, value] : output_data) {
    //                     if (!first)
    //                         submit_body += ',';
    //                     submit_body += std::format("\"{}\":\"{}\"", key, value);
    //                     first = false;
    //                 }
    //                 submit_body += "}}";
    //
    //                 // Submit result to engine
    //                 auto task_id = std::format("{}", instance.get_task_id());
    //                 try {
    //                     ctx.call_engine("POST",
    //                                     "/api/v1/tasks/" + task_id + "/result", submit_body);
    //                 } catch (const std::exception &e) {
    //                     core::logger::error("worker/poll-thread",
    //                                         "submit exception: {}", e.what());
    //                 }
    //             }
    //
    //             // Brief sleep before next poll cycle
    //             std::this_thread::sleep_for(std::chrono::milliseconds(100));
    //         }
    //
    //         std::println("[worker/thread-{}] stopped", i);
    //     });
    // }

    // ── 7. Wait for shutdown ──────────────────────────────────────────
    // std::println("[worker] running. Press Ctrl+C to stop.");
    // std::promise<void>().get_future().wait();

    return 0;
}
