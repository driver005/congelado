#include <csignal>
import worker;
import std;

// Linux/ELF: linker generates __start_congelado_tasks and __stop_congelado_tasks to bound
// the "congelado_tasks" section, which is populated by CONGELADO_TASK(T) in task .cc files.
// TODO(macos): use getsectbyname("__DATA", "congelado_tasks") instead.
extern "C" {
    using CTaskFactory = void *(*)();
    extern CTaskFactory __start_congelado_tasks[];
    extern CTaskFactory __stop_congelado_tasks[];
}

namespace {
std::atomic<bool> g_running{true};
extern "C" void on_signal(int) noexcept { g_running.store(false, std::memory_order_relaxed); }
} // namespace

int main(int argc, char **argv) {
    std::string_view config_path = argc > 1 ? argv[1] : "worker.toml";

    // 1. Parse worker.toml
    worker::WorkerConfig cfg;
    try {
        cfg = worker::WorkerConfig::from_file(config_path);
    } catch (std::exception const &e) {
        std::println(std::cerr, "worker: failed to load '{}': {}", config_path, e.what());
        return 1;
    }

    // 2. Enumerate compiled-in task types from ELF linker section
    std::vector<worker::ITaskWorker *> task_workers;
    for (CTaskFactory *f = __start_congelado_tasks; f != __stop_congelado_tasks; ++f) {
        task_workers.push_back(static_cast<worker::ITaskWorker *>((*f)()));
    }

    // 3. Fail-fast: every compiled-in task type must have a [[tasks]] config entry
    for (auto *w : task_workers) {
        bool found = std::ranges::any_of(cfg.tasks, [w](auto const &t) {
            return t.worker_type == w->get_task_type();
        });
        if (!found) {
            std::println(std::cerr,
                "worker: task type '{}' compiled in but missing [[tasks]] entry in '{}'",
                w->get_task_type(), config_path);
            return 1;
        }
    }

    // 4. Fail-fast: every [[tasks]] entry must have a compiled-in handler
    for (auto const &task_cfg : cfg.tasks) {
        bool found = std::ranges::any_of(task_workers, [&task_cfg](auto *w) {
            return w->get_task_type() == task_cfg.worker_type;
        });
        if (!found) {
            std::println(std::cerr,
                "worker: config entry '{}' (type '{}') has no compiled-in handler",
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

    // 6. Build WorkerContext — singletons owned by TU-statics; context holds non-owning refs
    worker::WorkerContext ctx;
    ctx.set_worker_id(cfg.worker_id);
    for (auto *w : task_workers) {
        ctx.add_task_worker(std::unique_ptr<worker::ITaskWorker>(w, [](worker::ITaskWorker *) noexcept {}));
    }

    // 7. Start poll threads (stub: no-op)
    std::uint32_t concurrency =
        cfg.concurrency == 0 ? std::thread::hardware_concurrency() : cfg.concurrency;
    worker::Poller poller{ctx, client, concurrency};
    poller.start();

    std::println("worker '{}' running: {} thread(s), {} task type(s)",
        cfg.worker_id, concurrency, cfg.tasks.size());

    std::signal(SIGINT, on_signal);
    std::signal(SIGTERM, on_signal);
    while (g_running.load(std::memory_order_relaxed)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    std::println("worker '{}' shutting down...", cfg.worker_id);
    poller.stop();
    poller.join();
    return 0;
}
