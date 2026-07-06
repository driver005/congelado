#define UUID_SYSTEM_GENERATOR
#include <catch2/catch_test_macros.hpp>
#include <uuid.h>

#include <atomic>
#include <chrono>
#include <mutex>
#include <thread>

import engine;
import worker;
import model;
import connector;

namespace {

class EchoWorker : public worker::ITaskWorker {
public:
    std::string_view get_task_type() const noexcept override { return "echo"; }
    worker::TaskOutput execute(worker::TaskInput const &input) override {
        worker::TaskOutput out;
        for (auto &[k, v] : input.get_data_map())
            out.set(k, v);
        return out;
    }
};

} // namespace

TEST_CASE("Three-thread engine+worker integration", "[integration]") {
    constexpr int NUM_TASKS = 50;

    engine::EngineContext ectx;
    worker::WorkerContext wctx{"test-worker"};

    EchoWorker echo_worker;
    wctx.add_task_worker(&echo_worker);

    // Serialize connector access since local-store mode is not thread-safe
    std::recursive_mutex conn_mtx;

    auto with_conn = [&](auto fn) {
        std::lock_guard lock{conn_mtx};
        fn(ectx.get_connector());
    };

    // Register task definition
    with_conn([&](auto &conn) {
        model::TaskDef def;
        def.set_name("echo-task");
        def.set_type(model::TaskType::SIMPLE);
        def.set_worker_type("echo");
        bool ok{false};
        conn.insert(def, [&](bool v) { ok = v; });
        REQUIRE(ok);
    });

    std::atomic<int> enqueued{0};
    std::atomic<int> processed{0};
    std::atomic<bool> running{true};
    std::atomic<int> server_seen{0};

    // ── Thread 1: Enqueuer ──
    std::thread enqueuer{[&]() {
        for (int i = 0; i < NUM_TASKS; ++i) {
            model::TaskInstance inst;
            inst.set_task_id(model::generate_id());
            inst.set_def_name("echo-task");
            inst.set_status(model::TaskStatus::SCHEDULED);
            inst.set_seq(static_cast<std::uint32_t>(i));
            inst.add_input_data("n", std::to_string(i));
            bool done{false};
            with_conn([&](auto &conn) {
                conn.insert(inst, [&](bool ok) {
                    if (ok)
                        enqueued.fetch_add(1);
                    done = true;
                });
            });
            while (!done)
                std::this_thread::yield();
        }
    }};

    // ── Thread 2: Worker (poll + execute + submit) ──
    std::thread worker{[&]() {
        while (running || enqueued.load() > processed.load()) {
            std::optional<model::TaskInstance> task;

            // Find next SCHEDULED task
            with_conn([&](auto &conn) {
                conn.template find_first<model::TaskInstance>(
                    serde::QueryOptions{}.add_order_by("seq"),
                    [](auto const &inst) {
                        return inst.get_status() == model::TaskStatus::SCHEDULED;
                    },
                    [](auto const &a, auto const &b) {
                        return a.get_seq() < b.get_seq();
                    },
                    [&](auto found) { task = std::move(found); });
            });

            if (!task) {
                std::this_thread::sleep_for(std::chrono::milliseconds(5));
                continue;
            }

            // Claim (IN_PROGRESS)
            with_conn([&](auto &conn) {
                task->set_status(model::TaskStatus::IN_PROGRESS);
                conn.update(*task, [](bool) {});
            });

            // Execute
            worker::TaskInput input{task->get_input_data()};
            auto output = wctx.run_task(task->get_def_name(), input);

            // Submit result
            with_conn([&](auto &conn) {
                task->set_status(output ? model::TaskStatus::COMPLETED
                                        : model::TaskStatus::FAILED);
                if (output)
                    task->set_output_data(output->get_data());
                conn.update(*task, [](bool) {});
            });

            processed.fetch_add(1);
        }
    }};

    // ── Thread 3: Server (monitor / verify) ──
    std::thread server{[&]() {
        while (running || processed.load() < NUM_TASKS) {
            int completed{0};
            with_conn([&](auto &conn) {
                conn.template find_all<model::TaskInstance>(
                    [&](std::vector<model::TaskInstance> all) {
                        for (auto &inst : all)
                            if (inst.get_status() == model::TaskStatus::COMPLETED)
                                ++completed;
                    });
            });
            server_seen.store(completed);
            if (completed >= NUM_TASKS)
                break;
            std::this_thread::sleep_for(std::chrono::milliseconds(2));
        }
    }};

    // Wait for completion
    enqueuer.join();
    while (processed.load() < NUM_TASKS)
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    running = false;
    worker.join();
    server.join();

    CHECK(enqueued.load() == NUM_TASKS);
    CHECK(processed.load() == NUM_TASKS);
    CHECK(server_seen.load() == NUM_TASKS);
}
