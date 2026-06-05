export module core_contract;

export import :signal_tree;
export import :types;

import std;
import core_logger;
import shared;
import :consts;


export namespace core::contract {

template <std::size_t MaxCapacity = 1024>
class ContractGroup;

template <std::size_t MaxCapacity = 1024>
class Contract {
  public:
    Contract(ContractGroup<MaxCapacity> &group, std::uint32_t local_id) noexcept
        : m_group{group}, m_local_id{local_id} {}

    void schedule() { m_group.get().schedule(m_local_id); }

    void deschedule() { m_group.get().deschedule(m_local_id); }

    void release() { m_group.get().release(m_local_id); }

  private:
    std::reference_wrapper<ContractGroup<MaxCapacity>> m_group;
    std::uint32_t m_local_id;
};


template <std::size_t MaxCapacity = 1024>
class ContractGroup : shared::HandlerInterface {
  public:
    ContractGroup() : m_signal_tree{}, m_workers{}, m_releasers{}, m_errors{} {
        init();
    }

    void init() {
        shared::this_handler::current = this;
    }

    Contract<MaxCapacity> create(std::string_view name, shared::WorkerFunction worker,
                                 shared::ReleaseFunction releaser = nullptr,
                                 shared::ErrorHandler error_handler = nullptr,
                                 ContractState state = ContractState::SCHEDULED) {
        const std::uint32_t id = add_worker(Worker{worker, state}, releaser, error_handler);
        switch (state) {
        case ContractState::SCHEDULED: {
            m_signal_tree.schedule(id);
            break;
        }
        case ContractState::RELEASED: {
            m_signal_tree.schedule(id);
            break;
        }
        case ContractState::IDLE:
            break;
        case ContractState::EXECUTING:
            core::logger::error("core/contract", "add_contract in EXECUTING state");
        }
        core::logger::debug("core/contract", "worker {} added", id);

        return Contract<MaxCapacity>{*this, id};
    }

    void schedule(std::uint32_t id) override {
        if (m_workers[id].is_scheduled()) {
            core::logger::fatal("core/contract", "worker {} already scheduled", id);
            return;
        }

        core::logger::debug("core/contract", "worker {} scheduled", id);
        if (m_workers[id].schedule()) {
            m_signal_tree.schedule(id);
        };
    }

    void deschedule(std::uint32_t id) override {
        if (m_workers[id].is_scheduled()) {
            core::logger::debug("core/contract", "worker {} descheduling", id);
            m_signal_tree.deschedule(id);
            m_workers[id].add_flags(ContractState::IDLE);
            return;
        }

        core::logger::fatal("core/contract", "worker {} not found on deschedule", id);
    }

    void release(std::uint32_t id) override {
        core::logger::debug("core/contract", "worker {} released", id);
        m_workers[id].add_flags(ContractState::RELEASED);

        if (!m_workers[id].is_scheduled()) {
            schedule(id);
        }
    }


    void process_next_contract(std::uint64_t &bias_flags) {
        // This is a fast, lock-free bit-scan.
        auto ready_id = m_signal_tree.next(bias_flags);

        if (!ready_id) {
            return;
        }

        core::logger::debug("core/contract", "worker {} ready", *ready_id);

        auto &worker = m_workers[*ready_id];
        auto &releaser = m_releasers[*ready_id];
        auto &error = m_errors[*ready_id];

        if (worker.try_claim_execution()) {
            m_signal_tree.deschedule(*ready_id);

            shared::this_handler::current_id = *ready_id;

            if (worker.is_released()) {
                core::logger::debug("core/contract", "worker {} releasing", *ready_id);
                AutoEraseContract erase_guard{*ready_id, worker, releaser, error, m_signal_tree};
                try {
                    if (releaser) {
                        releaser();
                    }
                } catch (...) {
                    core::logger::warning("core/contract", "released worker {} error", *ready_id);
                    if (error) {
                        error(std::current_exception());
                    } else {
                        // Rethrow if no error handler is set, allowing it to propagate to the
                        // caller or terminate the program
                        throw;
                    }
                }
            } else {
                core::logger::debug("core/contract", "worker {} executing", *ready_id);
                AutoClearExecuteFlag clear_guard{worker, *ready_id, m_signal_tree};
                try {
                    worker();
                } catch (...) {
                    core::logger::warning("core/contract", "scheduled worker {} error", *ready_id);
                    if (error) {
                        error(std::current_exception());
                    } else {
                        // Rethrow if no error handler is set, allowing it to propagate to the
                        // caller or terminate the program
                        throw;
                    }
                }
            }

            shared::this_handler::current_id = std::numeric_limits<std::uint32_t>::max();
        }
    }


  private:
    std::uint32_t add_worker(Worker worker, shared::ReleaseFunction releaser,
                             shared::ErrorHandler error_handler) {
        std::uint32_t id = m_signal_tree.free_contract_id();
        m_workers[id] = std::move(worker);
        m_releasers[id] = std::move(releaser);
        m_errors[id] = std::move(error_handler);
        return std::move(id);
    }


    struct AutoEraseContract {
        std::uint32_t id;
        std::reference_wrapper<Worker> worker;
        std::reference_wrapper<shared::ReleaseFunction> releaser;
        std::reference_wrapper<shared::ErrorHandler> error;
        std::reference_wrapper<SignalTree<MaxCapacity>> signal_tree;

        ~AutoEraseContract() noexcept {
            worker.get() = Worker{};
            releaser.get() = nullptr;
            error.get() = nullptr;
        }
    };

    struct AutoClearExecuteFlag {
        std::reference_wrapper<Worker> worker;
        std::uint32_t id;
        std::reference_wrapper<SignalTree<MaxCapacity>> signal_tree;

        ~AutoClearExecuteFlag() noexcept {
            auto prev = worker.get().fetch_and(~ContractState::EXECUTING);

            if ((prev & ContractState::SCHEDULED) == ContractState::SCHEDULED) {
                signal_tree.get().schedule(id);
            }
        }
    };

    SignalTree<MaxCapacity> m_signal_tree;
    std::array<Worker, MaxCapacity> m_workers;
    std::array<shared::ReleaseFunction, MaxCapacity> m_releasers;
    std::array<shared::ErrorHandler, MaxCapacity> m_errors;
};

template <std::size_t MaxCapacity = 1024>
class ContractThreadPool {
  public:
    explicit ContractThreadPool(ContractGroup<MaxCapacity> &group,
                                std::size_t thread_count = std::thread::hardware_concurrency())
        : m_group{group}, m_running{true} {
        core::logger::important("core/thread_pool", "starting {} threads", thread_count);
        for (std::size_t i = 0; i < thread_count; ++i) {
            m_workers.emplace_back(&ContractThreadPool::worker_loop, this);
        }
    }

    ~ContractThreadPool() {
        m_running = false;
        for (auto &t : m_workers) {
            if (t.joinable()) {
                core::logger::debug("core/thread_pool", "joining thread {}", t.get_id());
                t.join();
            }
        }
    }

  private:
    void worker_loop() {
        core::logger::debug("core/thread_pool", "thread {} started", std::this_thread::get_id());
        std::uint64_t dynamic_bias = 0;
        // Ensure the ContractGroup is initialized in this thread context
        m_group.get().init();
        while (m_running) {
            m_group.get().process_next_contract(dynamic_bias);

            // Optional: Sleep briefly to prevent busy-waiting when no contracts are ready
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
    }

    std::reference_wrapper<ContractGroup<MaxCapacity>> m_group;
    std::vector<std::thread> m_workers;
    std::atomic<bool> m_running;
};
} // namespace core::contract
