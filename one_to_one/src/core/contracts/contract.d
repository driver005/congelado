module core.contracts.contract;
@nogc nothrow:

import core.contracts.types      : ContractState, Worker;
import core.contracts.signal_tree : SignalTree;
import core.contracts.consts     : BIAS_FLAG;
import core.logger.logger;
import shared.handler : WorkerFunction, ReleaseFunction, ErrorHandler, HandlerInterface,
                        this_handler;
import util.alloc : make, dispose;

// Forward declaration
class ContractGroup(size_t MaxCapacity = 1024);

class Contract(size_t MaxCapacity = 1024) {
  public:
    this(ContractGroup!MaxCapacity group, uint local_id) {
        m_group   = group;
        m_local_id = local_id;
    }

    void schedule()   { m_group.schedule(m_local_id); }
    void deschedule() { m_group.deschedule(m_local_id); }
    void release()    { m_group.release(m_local_id); }

  private:
    ContractGroup!MaxCapacity m_group;
    uint m_local_id;
}

class ContractGroup(size_t MaxCapacity = 1024) : HandlerInterface {
  public:
    this() {
        m_signal_tree = make!(SignalTree!MaxCapacity)();
        for (size_t i = 0; i < MaxCapacity; ++i) {
            m_workers[i]   = make!Worker();
            m_releasers[i] = null;
            m_errors[i]    = null;
        }
        init();
    }

    ~this() {
        dispose(m_signal_tree);
        for (size_t i = 0; i < MaxCapacity; ++i)
            dispose(m_workers[i]);
    }

    void init() {
        this_handler.current = this;
    }

    Contract!MaxCapacity create(const(char)[] name, WorkerFunction worker,
                                ReleaseFunction releaser   = null,
                                ErrorHandler    error_handler = null,
                                ContractState   state      = ContractState.SCHEDULED) {
        const uint id = add_worker(worker, state, releaser, error_handler);
        switch (state) {
        case ContractState.SCHEDULED:
            m_signal_tree.schedule(id);
            break;
        case ContractState.RELEASED:
            m_signal_tree.schedule(id);
            break;
        case ContractState.IDLE:
            break;
        case ContractState.EXECUTING:
            error_("core/contract", "add_contract in EXECUTING state");
            break;
        default:
            break;
        }
        debug_("core/contract", "worker {} added", id);
        return make!(Contract!MaxCapacity)(this, id);
    }

    override void schedule(uint id) {
        if (m_workers[id].is_scheduled()) {
            fatal_("core/contract", "worker {} already scheduled", id);
            return;
        }

        debug_("core/contract", "worker {} scheduled", id);
        if (m_workers[id].schedule()) {
            m_signal_tree.schedule(id);
        }
    }

    override void deschedule(uint id) {
        if (m_workers[id].is_scheduled()) {
            debug_("core/contract", "worker {} descheduling", id);
            m_signal_tree.deschedule(id);
            m_workers[id].add_flags(ContractState.IDLE);
            return;
        }

        fatal_("core/contract", "worker {} not found on deschedule", id);
    }

    override void release(uint id) {
        debug_("core/contract", "worker {} released", id);
        m_workers[id].add_flags(ContractState.RELEASED);

        if (!m_workers[id].is_scheduled()) {
            schedule(id);
        }
    }

    void process_next_contract(ref ulong bias_flags) {
        // This is a fast, lock-free bit-scan.
        uint ready_id;
        if (!m_signal_tree.next(bias_flags, ready_id))
            return;

        debug_("core/contract", "worker {} ready", ready_id);

        auto worker  = m_workers[ready_id];
        auto releaser = m_releasers[ready_id];
        auto error   = m_errors[ready_id];

        if (worker.try_claim_execution()) {
            m_signal_tree.deschedule(ready_id);

            this_handler.current_id = ready_id;

            if (worker.is_released()) {
                debug_("core/contract", "worker {} releasing", ready_id);
                // AutoEraseContract: manually execute releaser then erase
                scope(exit) {
                    // Erase worker on scope exit (mirrors AutoEraseContract dtor)
                    m_workers[ready_id] = make!Worker();
                    m_releasers[ready_id] = null;
                    m_errors[ready_id]    = null;
                }
                if (releaser !is null) {
                    releaser();
                }
            } else {
                debug_("core/contract", "worker {} executing", ready_id);
                scope(exit) {
                    // AutoClearExecuteFlag: clear EXECUTING, re-schedule if still SCHEDULED
                    auto prev = worker.fetch_and(
                        cast(ContractState)(~cast(ulong)ContractState.EXECUTING));
                    import core.contracts.types : opAnd;
                    if ((cast(ulong)(prev.opAnd(ContractState.SCHEDULED))) ==
                        cast(ulong)ContractState.SCHEDULED) {
                        m_signal_tree.schedule(ready_id);
                    }
                }
                worker();
            }

            this_handler.current_id = uint.max;
        }
    }

  private:
    uint add_worker(WorkerFunction worker, ContractState state,
                    ReleaseFunction releaser, ErrorHandler error_handler) {
        uint id = m_signal_tree.free_contract_id();
        // Replace the placeholder Worker with one that has the actual function
        dispose(m_workers[id]);
        m_workers[id]   = make!Worker(worker, state);
        m_releasers[id] = releaser;
        m_errors[id]    = error_handler;
        return id;
    }

    SignalTree!MaxCapacity  m_signal_tree;
    Worker[MaxCapacity]     m_workers;
    ReleaseFunction[MaxCapacity] m_releasers;
    ErrorHandler[MaxCapacity]    m_errors;
}

// PORT-NOTE: ContractThreadPool uses OS threads (core.thread.osthread).
// std::thread::hardware_concurrency() → totalCPUs from core.cpuid.
// std::chrono::milliseconds(1000) sleep → Thread.sleep(dur!"msecs"(1000)).
// std::atomic<bool> m_running → shared bool with atomicLoad/atomicStore.
import core.atomic;

class ContractThreadPool(size_t MaxCapacity = 1024) {
  public:
    this(ContractGroup!MaxCapacity group,
         size_t thread_count = 0 /* 0 = hardware_concurrency */) {
        import core.cpuid : threadsPerCPU;
        m_group   = group;
        if (thread_count == 0)
            thread_count = threadsPerCPU;
        atomicStore!(MemoryOrder.raw)(m_running, true);
        important_("core/thread_pool", "starting {} threads", thread_count);
        // PORT-NOTE: std::vector<std::thread> → dynamic array of Thread
        import core.thread.osthread : Thread;
        m_workers.length = thread_count;
        for (size_t i = 0; i < thread_count; ++i) {
            m_workers[i] = new Thread(&worker_loop);
            m_workers[i].start();
        }
    }

    ~this() {
        atomicStore!(MemoryOrder.raw)(m_running, false);
        import core.thread.osthread : Thread;
        foreach (ref t; m_workers) {
            if (t !is null) {
                debug_("core/thread_pool", "joining thread");
                t.join();
            }
        }
    }

  private:
    void worker_loop() {
        debug_("core/thread_pool", "thread started");
        ulong dynamic_bias = 0;
        // Ensure the ContractGroup is initialized in this thread context
        m_group.init();
        while (atomicLoad!(MemoryOrder.raw)(m_running)) {
            m_group.process_next_contract(dynamic_bias);

            // Optional: Sleep briefly to prevent busy-waiting when no contracts are ready
            import core.time : dur;
            import core.thread.osthread : Thread;
            Thread.sleep(dur!"msecs"(1000));
        }
    }

    ContractGroup!MaxCapacity m_group;
    import core.thread.osthread : Thread;
    Thread[]   m_workers;
    shared bool m_running;
}
