module;

#include <format>
#include <utility>

export module core_contract:types;

import std;
import shared;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::contract {

enum class ContractState : std::uint8_t
{
    IDLE = 0x00'00'00'00ULL,      // No flags set - initial/unscheduled state
    SCHEDULED = 0x00'00'00'01ULL, // schedule_flag
    EXECUTING = 0x00'00'00'02ULL, // execute_flag
    RELEASED = 0x00'00'00'04ULL,  // release_flag
};

constexpr ContractState operator~(ContractState state)
{
    return static_cast<ContractState>(~std::to_underlying(state));
}

constexpr ContractState operator|(ContractState first_state, ContractState second_state)
{
    return static_cast<ContractState>(
        std::to_underlying(first_state) | std::to_underlying(second_state)
    );
}

constexpr ContractState operator&(ContractState first_state, ContractState second_state)
{
    return static_cast<ContractState>(
        std::to_underlying(first_state) & std::to_underlying(second_state)
    );
}

} // namespace core::contract

export template<>
struct std::formatter<core::contract::ContractState>
{
    /**
     * @brief No custom format-spec parsing needed — just accepts whatever's there and bails
     * immediately.
     * @param ctx the parse context.
     * @return iterator at the parse context's begin, unchanged.
     */
    static constexpr auto parse(std::format_parse_context& ctx)
    {
        return ctx.begin();
    }

    /**
     * @brief Formats a ContractState as its pipe-joined flag names (e.g.
     * "SCHEDULED|EXECUTING"), or "IDLE" for the zero state.
     * @tparam FormatContext the format context type, deduced from `ctx`.
     * @param state the state to format.
     * @param ctx the format context to write into.
     * @return iterator past the written output.
     */
    template<typename FormatContext>
    auto format(core::contract::ContractState state, FormatContext& ctx) const
    {
        using core::contract::ContractState;

        // IDLE is the zero state — none of the flag checks below would fire, so short-circuit.
        if (state == ContractState::IDLE) {
            return std::format_to(ctx.out(), "IDLE");
        }

        // Otherwise check each flag bit and append its name, pipe-separated.
        std::string result;
        auto val = std::to_underlying(state);
        if (val & std::to_underlying(ContractState::SCHEDULED)) {
            result += "SCHEDULED|";
        }
        if (val & std::to_underlying(ContractState::EXECUTING)) {
            result += "EXECUTING|";
        }
        if (val & std::to_underlying(ContractState::RELEASED)) {
            result += "RELEASED|";
        }
        // Trailing "|" from the last append has to go.
        if (!result.empty()) {
            result.pop_back();
        }

        return std::format_to(ctx.out(), "{}", result);
    }
};

export namespace core::contract {

class Worker
{
public:
    /**
     * @brief Default ctor — empty callable, IDLE flags (0). An unset slot, basically.
     */
    Worker() :
        m_worker{nullptr},
        m_flags{0}
    {
    }

    /**
     * @brief Builds a worker around `worker` starting in `state`.
     * @param worker the callable this worker runs when invoked.
     * @param state the initial ContractState flags.
     */
    Worker(shared::WorkerFunction worker, ContractState state) noexcept :
        m_worker{std::move(worker)},
        m_flags{std::to_underlying(state)}
    {
    }

    /**
     * @brief Move ctor — moves the callable over and atomically swaps `other`'s flags out for
     * 0, so the moved-from Worker ends up looking like a fresh, empty slot instead of a
     * dangling copy of live state. Lowkey important for the array-of-Worker storage in
     * ContractGroup, where slots get moved around constantly.
     * @param other the worker being moved from; left with flags reset to 0.
     */
    Worker(Worker&& other) noexcept :
        m_worker{std::move(other.m_worker)},
        m_flags{other.m_flags.exchange(0, std::memory_order_acq_rel)}
    {
    }

    /**
     * @brief Move assign — same "steal callable, reset other's flags" deal as the move ctor,
     * self-assignment guarded.
     * @param other the worker being moved from.
     * @return `*this`.
     */
    Worker& operator=(Worker&& other) noexcept
    {
        // Guard against self-move before touching either side.
        if (this != &other) {
            // Steal the callable, then copy the flags value over (not swap — `other` keeps
            // whatever it had, unlike the move ctor's exchange-to-zero).
            m_worker = std::move(other.m_worker);
            m_flags.store(other.m_flags.load(std::memory_order_relaxed), std::memory_order_relaxed);
        }
        return *this;
    }

    /**
     * @brief Clears the callable and zeroes the flags if a worker was actually installed.
     * Empty slots (already-moved-from or default-constructed) skip the work entirely.
     */
    ~Worker()
    {
        // Only tear down if there's actually something installed — already-empty slots skip
        // straight through, no cap.
        if (m_worker) {
            m_worker = nullptr;
            m_flags.store(0, std::memory_order_release);
        }
    }

    /**
     * @brief Deleted — no copying a worker slot, atomics plus an owned callable don't play
     * nice with that.
     */
    Worker(const Worker&) = delete;
    /**
     * @brief Deleted, same reasoning as the copy ctor.
     */
    Worker& operator=(const Worker&) = delete;

    /**
     * @brief Runs the wrapped callable, if there is one.
     * @note Empty (unset) workers are a silent no-op here, not an error.
     */
    void operator()() const
    {
        if (m_worker) {
            m_worker();
        }
    }

    /**
     * @brief Atomically ORs `state`'s bits into the flags.
     * @param state the flag bits to OR in.
     * @return the flags value from right before the OR.
     */
    auto fetch_or(ContractState state) noexcept
    {
        return static_cast<ContractState>(
            m_flags.fetch_or(std::to_underlying(state), std::memory_order_acq_rel)
        );
    }

    /**
     * @brief Atomically ANDs `state`'s bits into the flags. Bet — this is the primitive
     * remove_flags() rides on to clear bits out.
     * @param state the flag bits to AND in.
     * @return the flags value from right before the AND.
     */
    auto fetch_and(ContractState state) noexcept
    {
        return static_cast<ContractState>(
            m_flags.fetch_and(std::to_underlying(state), std::memory_order_acq_rel)
        );
    }

    /**
     * @brief Atomic load of the current flags.
     * @return the current ContractState flags.
     */
    [[nodiscard]] auto load() const noexcept
    {
        return static_cast<ContractState>(m_flags.load(std::memory_order_acquire));
    }

    /**
     * @brief Atomic compare-and-swap on the flags.
     * @param expected in/out — the flags value expected going in; updated to the actual value
     * on failure so callers can retry with fresh state.
     * @param desired the flags value to swap in if `expected` still matches.
     * @return true if the swap succeeded, false otherwise (and `expected` got refreshed).
     */
    bool compare_exchange(ContractState& expected, ContractState desired) noexcept
    {
        // Unwrap to the raw underlying type — the atomic itself only speaks uint8_t.
        auto expected_val = std::to_underlying(expected);
        bool success = m_flags.compare_exchange_weak(
            expected_val, std::to_underlying(desired), std::memory_order_acq_rel,
            std::memory_order_acquire
        );
        // Whether it succeeded or not, `expected_val` now holds the current value — rewrap it
        // so the caller sees fresh state on a failed swap too.
        expected = static_cast<ContractState>(expected_val);
        return success;
    }

    // Adds a flag (e.g., used by schedule())
    /**
     * @brief Thin wrapper over fetch_or() for setting one or more flag bits.
     * @param state the flag bits to add.
     */
    void add_flags(ContractState state) noexcept
    {
        fetch_or(state);
    }

    // Removes a flag (e.g., used to clear EXECUTING after run)
    /**
     * @brief Clears one or more flag bits.
     * @param state the flag bits to remove.
     */
    void remove_flags(ContractState state) noexcept
    {
        // Use bitwise NOT on the underlying type to clear the bit
        m_flags.fetch_and(~std::to_underlying(state), std::memory_order_acq_rel);
    }

    /**
     * @brief Tries to move this worker from IDLE into SCHEDULED.
     * @return true if it was neither scheduled nor executing before this call (a clean W —
     * the schedule actually took), false if it was already scheduled or currently executing.
     */
    bool schedule() noexcept
    {
        // Set the bit unconditionally, but check what it *was* right before that to know
        // whether this call actually changed anything.
        auto prev = fetch_or(ContractState::SCHEDULED);
        return std::to_underlying(prev & (ContractState::SCHEDULED | ContractState::EXECUTING)) ==
               0;
    }

    /**
     * @brief CAS-loop that swaps SCHEDULED for EXECUTING, so only one caller ever wins the
     * claim on a given ready worker.
     * @return true once this call successfully claimed execution, false if the worker wasn't
     * scheduled to begin with (nothing to claim).
     */
    bool try_claim_execution() noexcept
    {
        // Snapshot the flags once up front, bet — the CAS loop below refreshes it on every
        // failed attempt, so this is just the starting point.
        auto current = load();

        // Much cleaner and self-documenting
        while (is_scheduled()) {
            // Still need to calculate DESIRED based on what 'current' is right now
            const auto DESIRED = (current & ~ContractState::SCHEDULED) | ContractState::EXECUTING;

            if (compare_exchange(current, DESIRED)) {
                return true;
            }
        }
        return false;
    }

    /**
     * @brief Clears EXECUTING and folds in IDLE once a run's finished up.
     */
    void complete_execution() noexcept
    {
        // Clear the EXECUTING bit first, then fold IDLE (0) in — lowkey a no-op bit-wise, it's
        // really just documenting intent that this slot's back to resting.
        remove_flags(ContractState::EXECUTING);
        add_flags(ContractState::IDLE);
    }

    /**
     * @brief Checks whether any bit in `state` is currently set on the flags.
     * @param state the flag bits to test for.
     * @return true if at least one of `state`'s bits is set.
     */
    [[nodiscard]] bool has_any(ContractState state) const noexcept
    {
        return (std::to_underlying(load()) & std::to_underlying(state)) != 0;
    }

    /**
     * @brief Checks the SCHEDULED flag.
     * @return true if this worker is currently scheduled.
     */
    [[nodiscard]] bool is_scheduled() const noexcept
    {
        return has_any(ContractState::SCHEDULED);
    }

    /**
     * @brief Checks the RELEASED flag. No cap, once this flips true the worker's on its way
     * out for good.
     * @return true if this worker has been marked for release.
     */
    [[nodiscard]] bool is_released() const noexcept
    {
        return has_any(ContractState::RELEASED);
    }

    /**
     * @brief Checks whether no flags are set at all — the IDLE state, i.e. not scheduled, not
     * executing, not released. A worker that has run and then been released/erased lands back
     * here; a still-running one never does (it stays SCHEDULED/EXECUTING).
     * @return true if the worker is fully idle.
     */
    [[nodiscard]] bool is_idle() const noexcept
    {
        return std::to_underlying(load()) == 0;
    }

private:
    shared::WorkerFunction m_worker;
    std::atomic<std::uint8_t> m_flags;
};

} // namespace core::contract

#ifdef CONGELADO_TEST
namespace core::contract::tests {
using namespace boost::ut;

suite<"ContractState_operators"> contract_state_operators_suite = [] {
    "operator| combines flag bits"_test = [] {
        auto combined = ContractState::SCHEDULED | ContractState::EXECUTING;

        expect(
            std::to_underlying(combined) == (std::to_underlying(ContractState::SCHEDULED) |
                                             std::to_underlying(ContractState::EXECUTING))
        );
    };

    "operator& isolates shared flag bits"_test = [] {
        auto combined = ContractState::SCHEDULED | ContractState::EXECUTING;

        expect((combined & ContractState::SCHEDULED) == ContractState::SCHEDULED);
        expect((combined & ContractState::RELEASED) == ContractState::IDLE);
    };

    "operator~ inverts the underlying bits"_test = [] {
        auto inverted = ~ContractState::IDLE;

        expect(std::to_underlying(inverted) == static_cast<std::uint8_t>(~std::uint8_t{0}));
    };
};

suite<"ContractState_formatter"> contract_state_formatter_suite = [] {
    "IDLE formats as the literal string"_test = [] {
        expect(std::format("{}", ContractState::IDLE) == "IDLE");
    };

    "single flag formats as its own name"_test = [] {
        expect(std::format("{}", ContractState::SCHEDULED) == "SCHEDULED");
    };

    "combined flags format pipe-joined in flag order"_test = [] {
        auto combined = ContractState::SCHEDULED | ContractState::RELEASED;

        expect(std::format("{}", combined) == "SCHEDULED|RELEASED");
    };
};

suite<"Worker"> worker_suite = [] {
    "default-constructed worker is idle and running it is a no-op"_test = [] {
        Worker worker;

        expect(worker.is_idle());
        expect(not worker.is_scheduled());
        expect(not worker.is_released());
        worker();
    };

    "constructing with SCHEDULED sets the flag and invoking runs the callable"_test = [] {
        int calls = 0;
        Worker worker{
            [&calls] {
                ++calls;
            },
            ContractState::SCHEDULED
        };

        expect(worker.is_scheduled());
        worker();
        expect(calls == 1);
    };

    "add_flags/remove_flags toggle bits individually"_test = [] {
        Worker worker{[] {}, ContractState::IDLE};

        worker.add_flags(ContractState::SCHEDULED);
        expect(worker.is_scheduled());

        worker.remove_flags(ContractState::SCHEDULED);
        expect(not worker.is_scheduled());
    };

    "schedule() reports whether the call actually changed anything"_test = [] {
        Worker worker{[] {}, ContractState::IDLE};

        expect(worker.schedule());
        expect(not worker.schedule());
    };

    "try_claim_execution swaps SCHEDULED for EXECUTING exactly once"_test = [] {
        Worker worker{[] {}, ContractState::SCHEDULED};

        expect(worker.try_claim_execution());
        expect(not worker.is_scheduled());
        expect(worker.has_any(ContractState::EXECUTING));

        expect(not worker.try_claim_execution());
    };

    "complete_execution clears EXECUTING and returns to idle"_test = [] {
        Worker worker{[] {}, ContractState::SCHEDULED};
        worker.try_claim_execution();

        worker.complete_execution();

        expect(not worker.has_any(ContractState::EXECUTING));
        expect(worker.is_idle());
    };

    "compare_exchange swaps on match and refreshes expected on mismatch"_test = [] {
        Worker worker{[] {}, ContractState::IDLE};

        auto expected = ContractState::IDLE;
        expect(worker.compare_exchange(expected, ContractState::SCHEDULED));
        expect(worker.is_scheduled());

        auto stale_expected = ContractState::IDLE;
        expect(not worker.compare_exchange(stale_expected, ContractState::EXECUTING));
        expect(stale_expected == ContractState::SCHEDULED);
    };
};

} // namespace core::contract::tests
#endif
