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
    } catch (std::exception const &ex) {
        std::println(std::cerr, "worker: failed to load '{}': {}", config_path, ex.what());
        return 1;
    }

    // 2. Get all registered task workers (populated via CONGELADO_TASK static-init)
    auto task_workers = detail::TaskRegistry::instance().get_all();

    // 3. Fail-fast: every registered task type must have a [[tasks]] config entry
    for (auto *task : task_workers) {
        bool found = std::ranges::any_of(cfg.get_tasks(), [task](auto const &task_cfg) {
            return task_cfg.get_worker_type() == task->get_type();
        });
        if (!found) {
            std::println(std::cerr,
                "worker: task type '{}' registered but missing [[tasks]] entry in '{}'",
                task->get_type(), config_path);
            return 1;
        }
    }

    // 4. Fail-fast: every [[tasks]] entry must have a registered handler
    for (auto const &task_cfg : cfg.get_tasks()) {
        bool found = std::ranges::any_of(task_workers, [&task_cfg](auto *task) {
            return task->get_type() == task_cfg.get_worker_type();
        });
        if (!found) {
            std::println(std::cerr,
                "worker: config entry '{}' (type '{}') has no registered handler",
                task_cfg.get_name(), task_cfg.get_worker_type());
            return 1;
        }
    }

    // 5. Build WorkerContext
    worker::WorkerContext ctx;
    ctx.set_worker_id(cfg.get_worker_id());
    for (auto *task : task_workers) {
        ctx.add_task_worker(task);
    }

    // 6. Register signals
    std::signal(SIGINT, on_signal);
    std::signal(SIGTERM, on_signal);

    std::println("worker '{}' running: {} task type(s)", cfg.get_worker_id(), cfg.get_tasks().size());
    while (g_running.load(std::memory_order_relaxed)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    std::println("worker '{}' shutting down...", cfg.get_worker_id());
    return 0;
}

} // namespace congelado
