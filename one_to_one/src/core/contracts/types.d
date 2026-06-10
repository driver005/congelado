module core.contracts.types;
@nogc nothrow:

import core.atomic;
import shared.handler : WorkerFunction;

// PORT-NOTE: std::formatter<ContractState> → plain to_string helper; no std::format in D
// PORT-NOTE: std::atomic<uint64_t> m_flags → shared ulong + core.atomic ops

enum ContractState : ulong {
    IDLE      = 0x00000000UL, // No flags set - initial/unscheduled state
    SCHEDULED = 0x00000001UL, // schedule_flag
    EXECUTING = 0x00000002UL, // execute_flag
    RELEASED  = 0x00000004UL, // release_flag
}

// Bitwise helpers — replicate C++ constexpr operators
ContractState opNot(ContractState a) { return cast(ContractState)(~cast(ulong)a); }
ContractState opOr(ContractState a, ContractState b)  { return cast(ContractState)(cast(ulong)a | cast(ulong)b); }
ContractState opAnd(ContractState a, ContractState b) { return cast(ContractState)(cast(ulong)a & cast(ulong)b); }

import core.logger.logger : debug_ = debug_, logger_error = error;

class Worker {
  public:
    this() { m_worker = null; atomicStore!(MemoryOrder.raw)(m_flags, 0UL); }

    this(WorkerFunction worker, ContractState state) {
        m_worker = worker;
        atomicStore!(MemoryOrder.raw)(m_flags, cast(ulong)state);
    }

    // Copy disabled
    @disable this(ref Worker);

    ~this() {
        if (m_worker !is null) {
            m_worker = null;
            atomicStore!(MemoryOrder.rel)(m_flags, 0UL);
        }
    }

    void opCall() const {
        if (m_worker !is null) {
            m_worker();
        }
    }

    ContractState fetch_or(ContractState state) {
        return cast(ContractState) atomicFetchOr!(MemoryOrder.acq_rel)(m_flags, cast(ulong)state);
    }

    ContractState fetch_and(ContractState state) {
        return cast(ContractState) atomicFetchAnd!(MemoryOrder.acq_rel)(m_flags, cast(ulong)state);
    }

    ContractState load() const {
        return cast(ContractState) atomicLoad!(MemoryOrder.acq)(m_flags);
    }

    bool compare_exchange(ref ContractState expected, ContractState desired) {
        ulong exp = cast(ulong)expected;
        bool success = cas!(MemoryOrder.acq_rel, MemoryOrder.acq)(&m_flags, exp, cast(ulong)desired);
        expected = cast(ContractState)exp;
        return success;
    }

    // Adds a flag (e.g., used by schedule())
    void add_flags(ContractState state) { fetch_or(state); }

    // Removes a flag (e.g., used to clear EXECUTING after run)
    void remove_flags(ContractState state) {
        // Use bitwise NOT on the underlying type to clear the bit
        atomicFetchAnd!(MemoryOrder.acq_rel)(m_flags, ~cast(ulong)state);
    }

    bool schedule() {
        auto prev = fetch_or(ContractState.SCHEDULED);
        if ((cast(ulong)(prev.opAnd(ContractState.SCHEDULED.opOr(ContractState.EXECUTING)))) == 0) {
            return true;
        }
        return false;
    }

    bool try_claim_execution() {
        auto current = load();

        // Much cleaner and self-documenting
        while (is_scheduled()) {
            // Still need to calculate desired based on what 'current' is right now
            const desired = current.opAnd(ContractState.EXECUTING.opNot()).opOr(ContractState.EXECUTING);

            if (compare_exchange(current, desired)) {
                import core.logger.logger : debug_ = debug_;
                // core::logger::debug("core/worker", "claimed, state → {}", desired);
                return true;
            }
        }
        return false;
    }

    void complete_execution() {
        remove_flags(ContractState.EXECUTING);
        add_flags(ContractState.IDLE);
    }

    bool has_any(ContractState state) const {
        return (cast(ulong)load() & cast(ulong)state) != 0;
    }

    bool is_scheduled() const { return has_any(ContractState.SCHEDULED); }
    bool is_released()  const { return has_any(ContractState.RELEASED); }

  private:
    WorkerFunction m_worker;
    shared ulong   m_flags;
}
