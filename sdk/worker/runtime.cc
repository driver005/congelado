#include <csignal>
#include "congelado/worker.h"
import congelado_worker;
import worker;
import std;

namespace {
std::atomic<bool> g_running{true};
extern "C" void on_signal(int) noexcept { g_running.store(false, std::memory_order_relaxed); }
} // namespace

namespace congelado {

int run_worker(int argc, char **argv) {
    std::string_view config_path = argc > 1 ? argv[1] : "worker.toml";

    // 1. Parse worker.toml
    worker::WorkerConfig cfg;
    try {
        cfg = worker::WorkerConfig::from_file(config_path);
    } catch (std::exception const &e) {
        std::println(std::cerr, "worker: failed to load '{}': {}", config_path, e.what());
        return 1;
    }

    // 2. Get all registered task workers (populated via CONGELADO_TASK static-init)
    auto task_workers = detail::TaskRegistry::instance().all();

    // 3. Fail-fast: every registered task type must have a [[tasks]] config entry
    for (auto *w : task_workers) {
        bool found = std::ranges::any_of(cfg.tasks, [w](auto const &t) {
            return t.worker_type == w->type();
        });
        if (!found) {
            std::println(std::cerr,
                "worker: task type '{}' registered but missing [[tasks]] entry in '{}'",
                w->type(), config_path);
            return 1;
        }
    }

    // 4. Fail-fast: every [[tasks]] entry must have a registered handler
    for (auto const &task_cfg : cfg.tasks) {
        bool found = std::ranges::any_of(task_workers, [&task_cfg](auto *w) {
            return w->type() == task_cfg.worker_type;
        });
        if (!found) {
            std::println(std::cerr,
                "worker: config entry '{}' (type '{}') has no registered handler",
                task_cfg.name, task_cfg.worker_type);
            return 1;
        }
    }

    // 5. Register task defs with engine (stub: always succeeds)
    worker::EngineClient client{cfg.engine_url};
    for (auto const &task_cfg : cfg.tasks) {
        if (!client.upsert_task_def(task_cfg)) {
            std::println(std::cerr, "worker: failed to register task '{}' with engine at '{}'",
                task_cfg.name, cfg.engine_url);
            return 1;
        }
    }

    // 6. Build WorkerContext — ITask* is ITaskWorker* (single-inheritance), add_task_worker accepts it
    worker::WorkerContext ctx;
    ctx.set_worker_id(cfg.worker_id);
    for (auto *w : task_workers) {
        ctx.add_task_worker(w);
    }

    // 7. Register signals before starting poll threads
    std::signal(SIGINT, on_signal);
    std::signal(SIGTERM, on_signal);

    // 8. Start poll threads
    std::uint32_t concurrency =
        cfg.concurrency == 0 ? std::thread::hardware_concurrency() : cfg.concurrency;
    worker::Poller poller{ctx, client, concurrency};
    poller.start();

    std::println("worker '{}' running: {} thread(s), {} task type(s)",
        cfg.worker_id, concurrency, cfg.tasks.size());
    while (g_running.load(std::memory_order_relaxed)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    std::println("worker '{}' shutting down...", cfg.worker_id);
    poller.stop();
    poller.join();
    return 0;
}

} // namespace congelado
