export module core_contract;

export import :signal_tree;
export import :types;

import std;
import core_events;
import core_logger;
import shared;
import :consts;


export namespace core::contract {

template <std::size_t MaxCapacity = 1024>
class ContractGroup;

template <std::size_t MaxCapacity = 1024>
class Contract {
  public:
    /**
     * @brief Wraps a handle to one contract slot inside `group`, identified by `local_id`.
     * Thin reference type — the actual state lives in the group's arrays, this is just a
     * handle onto it.
     * @param group the owning ContractGroup this contract's state actually lives in.
     * @param local_id this contract's id/slot within `group`.
     */
    Contract(ContractGroup<MaxCapacity> &group, std::uint32_t local_id) noexcept
        : m_group{group}, m_local_id{local_id} {}

    /**
     * @brief Puts this contract back on the run queue.
     */
    void schedule() { m_group.get().schedule(m_local_id); }

    /**
     * @brief Pulls this contract off the run queue — parked, not released.
     */
    void deschedule() { m_group.get().deschedule(m_local_id); }

    /**
     * @brief Marks this contract for teardown. No coming back from this one.
     */
    void release() { m_group.get().release(m_local_id); }

    /**
     * @brief Whether this contract has been marked released.
     * @return true if released.
     */
    [[nodiscard]] bool is_released() const noexcept { return m_group.get().is_released(m_local_id); }

    /**
     * @brief Whether this contract is fully idle — done running and released/erased (or never
     * started). Owning code polls this to know when a self-releasing contract has finished.
     * @return true if idle.
     */
    [[nodiscard]] bool is_idle() const noexcept { return m_group.get().is_idle(m_local_id); }

  private:
    std::reference_wrapper<ContractGroup<MaxCapacity>> m_group;
    std::uint32_t m_local_id;
};


template <std::size_t MaxCapacity = 1024>
class ContractGroup : shared::HandlerInterface {
  public:
    /**
     * @brief Default ctor — empty signal tree, empty worker/releaser/error arrays, and
     * registers this group as the current thread's handler via init().
     */
    ContractGroup() { init(); }

    /**
     * @brief Registers this group as the thread-local `shared::this_handler::current` handler
     * so free functions like `shared::this_handler::shedule()` route into it. Gotta be called
     * from every thread that's actually gonna touch this group, not just once globally.
     */
    void init() { shared::this_handler::current = this; }

    /**
     * @brief Registers a new worker and hands back a Contract handle for it. Straight up the
     * main entry point for adding motion to this group.
     * @param name the worker's name, used purely for logging right now.
     * @param worker the callable that actually does the work when this contract executes.
     * @param releaser optional cleanup callable that runs once this contract gets released.
     * @param error_handler optional handler for exceptions thrown out of `worker`/`releaser`;
     * if null, the exception just propagates/rethrows instead of getting swallowed.
     * @param state the initial state to create the contract in — SCHEDULED/RELEASED both get
     * scheduled immediately, IDLE doesn't, and EXECUTING logs an error since that's not a
     * valid initial state to hand in.
     * @return a Contract handle bound to the newly registered worker's slot.
     */
    Contract<MaxCapacity> create(std::string_view name, const shared::WorkerFunction &worker,
                                 shared::ReleaseFunction releaser = nullptr,
                                 shared::ErrorHandler error_handler = nullptr,
                                 ContractState state = ContractState::SCHEDULED) {
        // Claim a slot first, then decide what to do with it based on the requested state.
        const std::uint32_t CONTRACT_ID =
            add_worker(Worker{worker, state}, std::move(releaser), std::move(error_handler));
        switch (state) {
        case ContractState::SCHEDULED:
        case ContractState::RELEASED: {
            // Both SCHEDULED and RELEASED start life on the run queue right away.
            m_signal_tree.schedule(CONTRACT_ID);
            break;
        }
        case ContractState::IDLE:
            // IDLE just sits there until something schedules it later.
            break;
        case ContractState::EXECUTING:
            // Not a valid state to hand in at creation time — log it instead of scheduling.
            core::logger::error("core/contract", "add_contract in EXECUTING state");
            core::events::publish("contract.worker.invalid_state",
                                  {{"contract_id", std::to_string(CONTRACT_ID)}});
        }

        return Contract<MaxCapacity>{*this, CONTRACT_ID};
    }

    /**
     * @brief Whether the worker `id` has been marked released (on its way out for good).
     * @param contract_id the worker id to query.
     * @return true if released.
     */
    [[nodiscard]] bool is_released(std::uint32_t contract_id) const noexcept {
        return m_workers[contract_id].is_released(); // NOLINT(cppcoreguidelines-pro-bounds-*)
    }

    /**
     * @brief Whether the worker `id` is fully idle (released-and-erased, or never started).
     * @param contract_id the worker id to query.
     * @return true if idle.
     */
    [[nodiscard]] bool is_idle(std::uint32_t contract_id) const noexcept {
        return m_workers[contract_id].is_idle(); // NOLINT(cppcoreguidelines-pro-bounds-*)
    }

    /**
     * @brief Puts the worker identified by `id` on the run queue via the signal tree.
     * @warning Fatal-logs (doesn't throw) if the worker's already scheduled — calling this
     * twice on the same id without a deschedule in between is a logic error, not something
     * this recovers from gracefully.
     * @param contract_id the worker's id to schedule.
     */
    void schedule(std::uint32_t contract_id) override {
        if (m_workers[contract_id].is_released()) { // FIXME(clang-tidy): unchecked operator[],
                                                    // consider .at(); non-constant array index
            return;
        }

        // Guard clause — scheduling an already-scheduled worker's an L, a logic error upstream.
        if (m_workers[contract_id].is_scheduled()) { // FIXME(clang-tidy): unchecked operator[],
                                                     // consider .at(); non-constant array index
            core::events::publish(
                "contract.worker.fatal",
                {{"contract_id", std::to_string(contract_id)}, {"reason", "already_scheduled"}});
            core::logger::fatal("core/contract", "worker {} already scheduled", contract_id);
            return;
        }

        // Flip the worker's own flag first, and only touch the signal tree if that actually
        // took (it won't if another thread raced in and claimed it first).
        if (m_workers[contract_id].schedule()) { // FIXME(clang-tidy): unchecked operator[],
                                                 // consider .at(); non-constant array index
            m_signal_tree.schedule(contract_id);
        };
    }

    /**
     * @brief Pulls the worker identified by `id` off the run queue and marks it IDLE.
     * @warning Fatal-logs if the worker isn't currently scheduled — deschedule() only works on
     * something that's actually scheduled, calling it on an idle/unscheduled id logs loud
     * instead of quietly no-op'ing.
     * @param contract_id the worker's id to deschedule.
     */
    void deschedule(std::uint32_t contract_id) override {
        if (m_workers[contract_id].is_released()) { // FIXME(clang-tidy): unchecked operator[],
                                                    // consider .at(); non-constant array index
            return;
        }
        // Happy path — actually scheduled, so pull it off the tree and mark it IDLE.
        if (m_workers[contract_id].is_scheduled()) { // FIXME(clang-tidy): unchecked operator[],
                                                     // consider .at(); non-constant array index
            m_signal_tree.deschedule(contract_id);
            m_workers[contract_id].add_flags(
                ContractState::IDLE); // FIXME(clang-tidy): unchecked operator[], consider .at();
                                      // non-constant array index
            return;
        }

        // Not scheduled to begin with — nothing to pull off, log loud instead of no-op'ing.
        core::events::publish(
            "contract.worker.fatal",
            {{"contract_id", std::to_string(contract_id)}, {"reason", "not_found_on_deschedule"}});
        core::logger::fatal("core/contract", "worker {} not found on deschedule", contract_id);
    }

    /**
     * @brief Marks the worker identified by `id` for release and makes sure it's scheduled so
     * process_next_contract() actually gets to run its releaser.
     * @param contract_id the worker's id to release.
     */
    void release(std::uint32_t contract_id) override {
        // Mark it released first — that's the flag process_next_contract() checks to decide
        // whether to run the releaser instead of the worker.
        m_workers[contract_id].add_flags(
            ContractState::RELEASED); // FIXME(clang-tidy): unchecked operator[], consider .at();
                                      // non-constant array index

        // Not already on the run queue — force it there so the release actually gets processed.
        if (!m_workers[contract_id].is_scheduled()) { // FIXME(clang-tidy): unchecked operator[],
                                                      // consider .at(); non-constant array index
            schedule(contract_id);
        }
    }


    /**
     * @brief Pulls the next ready contract off the signal tree and runs it — either its
     * releaser (if it was marked released) or its worker function, whichever applies. This is
     * the actual engine that makes contracts go, gotta get called in a loop to keep the motion
     * flowing.
     * @warning Exceptions thrown by the worker/releaser get caught and handed to that
     * contract's error handler if one's set; with none set, they get rethrown and can
     * terminate the calling thread. Don't skip installing an error_handler on anything that
     * might throw, that's an L waiting to happen.
     * @param bias_flags rotating bias state threaded through the signal tree's bit-scan, kept
     * across calls so scheduling stays fair instead of always favoring the same branch.
     */
    void process_next_contract(std::uint64_t &bias_flags) {
        // This is a fast, lock-free bit-scan.
        auto ready_id = m_signal_tree.next(bias_flags);

        // Nothing ready this pass — bail, nothing to run.
        if (!ready_id) {
            return;
        }

        auto &worker = m_workers[*ready_id]; // FIXME(clang-tidy): unchecked operator[], consider
                                             // .at(); non-constant array index
        auto &releaser = m_releasers[*ready_id]; // FIXME(clang-tidy): unchecked operator[],
                                                 // consider .at(); non-constant array index
        auto &error = m_errors[*ready_id]; // FIXME(clang-tidy): unchecked operator[], consider
                                           // .at(); non-constant array index

        // Only proceed if this call actually wins the claim, no cap — another thread could've
        // grabbed the same ready id first, in which case there's nothing left for us to do here.
        if (worker.try_claim_execution()) {
            m_signal_tree.deschedule(*ready_id);

            shared::this_handler::current_id = *ready_id;

            // Released contracts get torn down via the releaser instead of re-running worker().
            if (worker.is_released()) {
                core::logger::debug("core/contract", "cycle: worker {} released", *ready_id);
                AutoEraseContract erase_guard{*ready_id, worker, releaser, error, m_signal_tree};
                try {
                    if (releaser) {
                        releaser();
                    }
                } catch (...) {
                    core::logger::warning("core/contract", "released worker {} error", *ready_id);
                    core::events::publish(
                        "contract.worker.execution_failed",
                        {{"contract_id", std::to_string(*ready_id)}, {"phase", "released"}});
                    if (error) {
                        error(std::current_exception());
                    } else {
                        // Rethrow if no error handler is set, allowing it to propagate to the
                        // caller or terminate the program
                        throw;
                    }
                }
            } else {
                // Normal scheduled run — call the worker itself.
                core::logger::debug("core/contract", "cycle: worker {} executing", *ready_id);
                AutoClearExecuteFlag clear_guard{worker, *ready_id, m_signal_tree};
                try {
                    worker();
                } catch (...) {
                    core::logger::warning("core/contract", "scheduled worker {} error", *ready_id);
                    core::events::publish(
                        "contract.worker.execution_failed",
                        {{"contract_id", std::to_string(*ready_id)}, {"phase", "executing"}});
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
    /**
     * @brief Grabs a free contract id off the signal tree and installs the worker/releaser/
     * error handler into their respective slot arrays.
     * @param worker the worker to install.
     * @param releaser the releaser to install.
     * @param error_handler the error handler to install.
     * @return the freshly claimed contract id.
     */
    std::uint32_t add_worker(Worker worker, shared::ReleaseFunction releaser,
                             shared::ErrorHandler error_handler) {
        // Claim a fresh id, then drop each piece into its matching slot array.
        std::uint32_t id = m_signal_tree.free_contract_id();
        m_workers[id] = std::move(worker);     // FIXME(clang-tidy): unchecked operator[], consider
                                               // .at(); non-constant array index
        m_releasers[id] = std::move(releaser); // FIXME(clang-tidy): unchecked operator[], consider
                                               // .at(); non-constant array index
        m_errors[id] = std::move(error_handler); // FIXME(clang-tidy): unchecked operator[],
                                                 // consider .at(); non-constant array index
        return id;
    }


    class AutoEraseContract {
      public:
        AutoEraseContract(std::uint32_t contract_id, Worker &worker,
                          shared::ReleaseFunction &releaser, shared::ErrorHandler &error,
                          SignalTree<MaxCapacity> &signal_tree) noexcept
            : m_id{contract_id}, m_worker{worker}, m_releaser{releaser}, m_error{error},
              m_signal_tree{signal_tree} {}

        AutoEraseContract(const AutoEraseContract &) = delete;
        AutoEraseContract &operator=(const AutoEraseContract &) = delete;
        AutoEraseContract(AutoEraseContract &&) = delete;
        AutoEraseContract &operator=(AutoEraseContract &&) = delete;

        /**
         * @brief RAII guard dtor — resets the worker/releaser/error slots back to empty once a
         * released contract finishes running, clean or thrown. Erases the slot for real, no
         * partial cleanup left hanging around.
         */
        ~AutoEraseContract() noexcept {
            m_worker.get() = Worker{};
            m_releaser.get() = nullptr;
            m_error.get() = nullptr;
        }

      private:
        std::uint32_t m_id;
        std::reference_wrapper<Worker> m_worker;
        std::reference_wrapper<shared::ReleaseFunction> m_releaser;
        std::reference_wrapper<shared::ErrorHandler> m_error;
        std::reference_wrapper<SignalTree<MaxCapacity>> m_signal_tree;
    };

    class AutoClearExecuteFlag {
      public:
        AutoClearExecuteFlag(Worker &worker, std::uint32_t contract_id,
                             SignalTree<MaxCapacity> &signal_tree) noexcept
            : m_worker{worker}, m_id{contract_id}, m_signal_tree{signal_tree} {}

        AutoClearExecuteFlag(const AutoClearExecuteFlag &) = delete;
        AutoClearExecuteFlag &operator=(const AutoClearExecuteFlag &) = delete;
        AutoClearExecuteFlag(AutoClearExecuteFlag &&) = delete;
        AutoClearExecuteFlag &operator=(AutoClearExecuteFlag &&) = delete;

        /**
         * @brief RAII guard dtor — clears the EXECUTING flag once a scheduled contract's
         * worker call returns (clean or via exception), and re-schedules it on the signal tree
         * if a SCHEDULED flag got set again while it was mid-run.
         */
        ~AutoClearExecuteFlag() noexcept {
            try {
                // Clear EXECUTING and grab the pre-clear flags in one shot.
                auto prev = m_worker.get().fetch_and(~ContractState::EXECUTING);

                // A SCHEDULED flag got set again mid-run (someone called schedule() while this
                // was executing) — put it back on the tree so that request doesn't get dropped.
                if ((prev & ContractState::SCHEDULED) == ContractState::SCHEDULED) {
                    m_signal_tree.get().schedule(m_id);
                }
            } catch (...) {
                // Never let an exception escape a destructor — log it loud instead.
                core::events::publish("contract.autoclear.fatal",
                                      {{"contract_id", std::to_string(m_id)}});
                core::logger::fatal("core/contract", "exception in ~AutoClearExecuteFlag");
            }
        }

      private:
        std::reference_wrapper<Worker> m_worker;
        std::uint32_t m_id;
        std::reference_wrapper<SignalTree<MaxCapacity>> m_signal_tree;
    };

    SignalTree<MaxCapacity> m_signal_tree{};
    std::array<Worker, MaxCapacity> m_workers{};
    std::array<shared::ReleaseFunction, MaxCapacity> m_releasers{};
    std::array<shared::ErrorHandler, MaxCapacity> m_errors{};
};

template <std::size_t MaxCapacity = 1024>
class ContractThreadPool {
  public:
    /**
     * @brief Spins up `thread_count` worker threads, each looping process_next_contract() on
     * `group` until told to stop.
     * @param group the ContractGroup these threads pull work from.
     * @param thread_count how many worker threads to spin up, defaults to
     * `std::thread::hardware_concurrency()`.
     */
    explicit ContractThreadPool(ContractGroup<MaxCapacity> &group,
                                std::size_t thread_count = std::thread::hardware_concurrency())
        : m_group{group}, m_running{true} {
        core::logger::important("core/thread_pool", "starting {} threads", thread_count);
        core::events::publish("contract.thread_pool.started",
                              {{"thread_count", std::to_string(thread_count)}});
        // Spin up each worker thread, all running the same loop against the shared group.
        for (std::size_t i = 0; i < thread_count; ++i) {
            m_workers.emplace_back(&ContractThreadPool::worker_loop, this);
        }
    }

    ContractThreadPool(const ContractThreadPool &) = delete;
    ContractThreadPool &operator=(const ContractThreadPool &) = delete;
    ContractThreadPool(ContractThreadPool &&) = delete;
    ContractThreadPool &operator=(ContractThreadPool &&) = delete;

    /**
     * @brief Flips the running flag off and joins every worker thread. Blocks until they've
     * all actually wrapped up.
     */
    ~ContractThreadPool() {
        // Flip the flag first so every worker_loop() sees it and exits its own loop...
        m_running = false;
        // ...then join each thread, waiting for them to actually wrap up.
        for (auto &worker_thread : m_workers) {
            if (worker_thread.joinable()) {
                core::logger::debug("core/thread_pool", "joining thread {}",
                                    worker_thread.get_id());
                worker_thread.join();
            }
        }
    }

  private:
    /**
     * @brief Per-thread loop — inits this thread's handler context, then repeatedly pulls and
     * runs the next ready contract with a 1s sleep between polls while `m_running` stays true.
     * @warning That sleep is a flat 1000ms busy-wait guard, not a condvar/wakeup — so there's
     * up to a full second of latency between a contract becoming ready and this thread
     * actually picking it up. Lowkey not great for anything latency-sensitive.
     */
    void worker_loop() {
        core::logger::debug("core/thread_pool", "thread {} started", std::this_thread::get_id());
        std::uint64_t dynamic_bias = 0;
        // Ensure the ContractGroup is initialized in this thread context
        m_group.get().init();
        while (m_running) {
            // Pull the next ready contract, if any, and run it.
            m_group.get().process_next_contract(dynamic_bias);

            // Optional: Sleep briefly to prevent busy-waiting when no contracts are ready
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
    }

    std::reference_wrapper<ContractGroup<MaxCapacity>> m_group;
    std::vector<std::thread> m_workers;
    std::atomic<bool> m_running;
};

/**
 * @brief Registry for every `Contract<>` a controller creates. Keeps handles in one place so
 * teardown can release them in bulk before the contract thread pool stops.
 */
class ContractRegistry {
  public:
    /**
     * @brief Stores a contract handle. The deque is chosen so handles stay stable while the
     * registry grows; releasing is done explicitly via `release_all()`.
     * @param contract the handle to keep alive.
     */
    void add(Contract<> contract) { m_contracts.push_back(std::move(contract)); }

    /// @brief Releases every stored contract and clears the registry. Idempotent.
    void release_all() noexcept {
        for (auto &contract : m_contracts) {
            contract.release();
        }
        m_contracts.clear();
    }

    /// @brief Number of currently stored contracts.
    [[nodiscard]] std::size_t size() const noexcept { return m_contracts.size(); }

    /// @brief True if no contracts are currently stored.
    [[nodiscard]] bool empty() const noexcept { return m_contracts.empty(); }

  private:
    std::deque<Contract<>> m_contracts;
};

} // namespace core::contract
