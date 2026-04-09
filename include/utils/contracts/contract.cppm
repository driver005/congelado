export module contracts;

export import :signal_tree;
export import :types;

import std;
import :consts;

namespace contracts {

class WorkContractGroupInterface {
  public:
    virtual ~WorkContractGroupInterface() = default;

    virtual void schedule(std::uint32_t id) = 0;
    virtual void deschedule(std::uint32_t id) = 0;
    virtual void release(std::uint32_t id) = 0;
};

} // namespace contracts

export namespace contracts::this_contract {

thread_local WorkContractGroupInterface *current = nullptr;

thread_local std::uint32_t current_id = std::numeric_limits<std::uint32_t>::max();

void shedule() {
    if (!current)
        throw std::runtime_error("No current contract context for scheduling");

    current->schedule(current_id);
}

void deschedule() {
    if (!current)
        throw std::runtime_error("No current contract context for descheduling");

    current->deschedule(current_id);
}

void release() {
    if (!current)
        throw std::runtime_error("No current contract context for releasing");

    current->release(current_id);
}

} // namespace contracts::this_contract

export namespace contracts {

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
class ContractGroup : WorkContractGroupInterface {
  public:
    ContractGroup() : m_signal_tree{}, m_workers{}, m_releasers{}, m_errors{} {}

    void init() { this_contract::current = this; }

    Contract<MaxCapacity> create_contract(WorkerFunction worker, ContractState state = ContractState::SCHEDULED,
                                          ReleaseFunction releaser = nullptr, ErrorHandler error_handler = nullptr) {
        const std::uint32_t id = add_worker(Worker{worker, state}, releaser, error_handler);
        return Contract<MaxCapacity>{*this, id};
    }

    void schedule(std::uint32_t id) override {
        if (m_workers[id].is_scheduled())
            throw std::runtime_error("Worker is already scheduled");

        m_signal_tree.schedule(id);
        m_workers[id].add_flags(ContractState::SCHEDULED);
    }

    void deschedule(std::uint32_t id) override {
        if (m_workers[id].is_scheduled()) {
            m_signal_tree.deschedule(id);
            m_workers[id].add_flags(ContractState::IDLE);
        }

        throw std::runtime_error("Worker is not scheduled");
    }

    void release(std::uint32_t id) override {
        m_workers[id].add_flags(ContractState::RELEASED);

        if (!m_workers[id].is_scheduled()) {
            schedule(id);
        }
    }


    void process_next_contract(std::uint64_t bias_flags) {
        // 1. SELECT: Ask the SignalTree which ID is ready for work
        // This is a fast, lock-free bit-scan.
        auto ready_id = m_signal_tree.next(bias_flags);

        if (!ready_id) {
            return;
        }

        auto &worker = m_workers[*ready_id];

        if (worker.try_claim_execution()) {
            m_signal_tree.deschedule(*ready_id);

            this_contract::current_id = *ready_id;

            try {
                if (worker.is_released()) {
                    worker.run_release_callback();
                } else {
                    worker();
                }
            } catch (...) {
                if (auto &handler = m_errors[*ready_id]) {
                    handler(std::current_exception());
                } else {
                    // Rethrow if no error handler is set, allowing it to propagate to the caller or terminate the
                    // program
                    throw;
                }
            }

            this_contract::current_id = std::numeric_limits<std::uint32_t>::max();

            worker.complete_execution();
        }
    }


  private:
    std::uint32_t add_worker(Worker worker, ReleaseFunction releaser, ErrorHandler error_handler) {
        std::uint32_t id = m_signal_tree.free_contract_id();
        m_workers[id] = std::move(worker);
        m_releasers[id] = std::move(releaser);
        m_errors[id] = std::move(error_handler);
        return std::move(id);
    }

    SignalTree<MaxCapacity> m_signal_tree;
    std::array<Worker, MaxCapacity> m_workers;
    std::array<ReleaseFunction, MaxCapacity> m_releasers;
    std::array<ErrorHandler, MaxCapacity> m_errors;
};

class ContractBase {
  public:
    virtual ~ContractBase() = default;

    /**
     * @brief Hooks this class into a WorkContractGroup and returns a managed Contract handle.
     * @tparam MaxCapacity The capacity of the group.
     * @param group Reference to the WorkContractGroup.
     * @param state Initial state (defaults to SCHEDULED).
     * @return A Contract<MaxCapacity> object representing the registered task.
     */
    template <std::size_t MaxCapacity>
    [[nodiscard]] auto register_contract(ContractGroup<MaxCapacity> &group,
                                         ContractState state = ContractState::SCHEDULED) {
        return group.create_contract(on_execute(), state, on_released(), on_error());
    }

  protected:
    virtual WorkerFunction on_execute() = 0;

    virtual ReleaseFunction on_released() noexcept { return nullptr; }

    virtual ErrorHandler on_error() { return nullptr; }
};

} // namespace contracts
