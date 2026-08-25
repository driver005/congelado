export module shared:task_queue;

import std;
import :handler;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace shared {

/// @brief One dedicated, long-lived contract that drains a local queue of jobs one at a time — the
/// generic dispatch mechanism for anything that wants its own contract instead of running inline on
/// the caller's (same `create`/`set_wake` idiom as `connector::Connector`/`core::router::RouterExecutor`).
/// `push()` queues a job and wakes the contract if it's currently parked. A job that throws is logged
/// and dropped — never left to kill this queue's contract.
class TaskQueue final : public HandlerBase {
  public:
    explicit TaskQueue(std::string name) : m_name{std::move(name)} {}

    [[nodiscard]] std::string_view get_name() const noexcept override { return m_name; }

    /// @brief Installs the resume callback — same shape as `Connector::set_wake`, wire it right
    /// after `create()`.
    void set_wake(std::move_only_function<void()> wake) { m_wake = std::move(wake); }

    /// @brief Queues one job to run on this queue's own dedicated contract — never the caller's
    /// thread. Wakes the contract if it's currently parked.
    void push(std::move_only_function<void()> job) {
        bool need_wake = false;
        {
            std::lock_guard lock{m_mutex};
            m_jobs.push_back(std::move(job));
            need_wake = !m_running;
        }
        if (need_wake) {
            wake();
        }
    }

    WorkerFunction on_execute() override {
        return [this] {
            std::move_only_function<void()> job;
            {
                std::lock_guard lock{m_mutex};
                if (m_jobs.empty()) {
                    m_running = false;
                    return; // Park — push() wakes us once more work lands.
                }
                job = std::move(m_jobs.front());
                m_jobs.pop_front();
                m_running = true;
            }
            try {
                job();
            } catch (...) {
                // Never let one bad job kill this queue's contract.
            }
            this_handler::shedule(); // More may have queued while this job ran — check again.
        };
    }

  private:
    void wake() {
        if (m_wake) {
            m_wake();
        }
    }

    std::string m_name;
    std::move_only_function<void()> m_wake;
    std::deque<std::move_only_function<void()>> m_jobs;
    std::mutex m_mutex;
    bool m_running{false};
};

} // namespace shared

#ifdef CONGELADO_TEST
namespace shared::task_queue_tests {

class MockHandlerInterface final : public HandlerInterface {
  public:
    void schedule(std::uint32_t) override { ++m_schedule_count; }
    void deschedule(std::uint32_t) override { ++m_deschedule_count; }
    void release(std::uint32_t) override { ++m_release_count; }

    int m_schedule_count{0};
    int m_deschedule_count{0};
    int m_release_count{0};
};

using namespace boost::ut;

suite<"TaskQueue"> task_queue_suite = [] {
    "get_name returns the constructor-provided name"_test = [] {
        TaskQueue queue{"my-queue"};
        expect(queue.get_name() == "my-queue");
    };

    "push on an idle queue wakes it; push while running does not"_test = [] {
        TaskQueue queue{"wake-test"};
        int wake_count = 0;
        queue.set_wake([&] { ++wake_count; });

        queue.push([] {}); // idle -> running is still false, so this wakes.
        expect(wake_count == 1);
    };

    "on_execute runs the oldest queued job and reschedules via this_handler"_test = [] {
        MockHandlerInterface mock;
        this_handler::current = &mock;
        this_handler::current_id = 7;

        TaskQueue queue{"run-test"};
        bool job_ran = false;
        queue.push([&] { job_ran = true; });

        auto worker = queue.on_execute();
        worker();

        expect(job_ran);
        expect(mock.m_schedule_count == 1);

        this_handler::current = nullptr;
    };

    "on_execute with an empty queue parks without touching this_handler"_test = [] {
        TaskQueue queue{"empty-test"};
        auto worker = queue.on_execute();
        expect(nothrow([&] { worker(); }));
    };

    "a job that throws is swallowed, not propagated"_test = [] {
        MockHandlerInterface mock;
        this_handler::current = &mock;
        this_handler::current_id = 1;

        TaskQueue queue{"throw-test"};
        queue.push([] { throw std::runtime_error{"boom"}; });

        auto worker = queue.on_execute();
        expect(nothrow([&] { worker(); }));

        this_handler::current = nullptr;
    };
};

} // namespace shared::task_queue_tests
#endif
