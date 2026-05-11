module;

#include <format>
#include <utility>

export module core_contract:types;

import std;
import shared;
import core_logger;

export namespace core::contract {

enum class ContractState : std::uint64_t {
    IDLE = 0x00000000ull,      // No flags set - initial/unscheduled state
    SCHEDULED = 0x00000001ull, // schedule_flag
    EXECUTING = 0x00000002ull, // execute_flag
    RELEASED = 0x00000004ull,  // release_flag
};
constexpr ContractState operator~(ContractState a) { return static_cast<ContractState>(~std::to_underlying(a)); }
constexpr ContractState operator|(ContractState a, ContractState b) {
    return static_cast<ContractState>(std::to_underlying(a) | std::to_underlying(b));
}
constexpr ContractState operator&(ContractState a, ContractState b) {
    return static_cast<ContractState>(std::to_underlying(a) & std::to_underlying(b));
}

} // namespace core::contract

export template <>
struct std::formatter<core::contract::ContractState> {
    constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(core::contract::ContractState state, FormatContext &ctx) const {
        using core::contract::ContractState;

        if (state == ContractState::IDLE)
            return std::format_to(ctx.out(), "IDLE");

        std::string s;
        auto val = std::to_underlying(state);
        if (val & std::to_underlying(ContractState::SCHEDULED))
            s += "SCHEDULED|";
        if (val & std::to_underlying(ContractState::EXECUTING))
            s += "EXECUTING|";
        if (val & std::to_underlying(ContractState::RELEASED))
            s += "RELEASED|";
        if (!s.empty())
            s.pop_back();

        return std::format_to(ctx.out(), "{}", s);
    }
};

export namespace core::contract {

class Worker {
  public:
    Worker() : m_worker{nullptr}, m_flags{0} {}

    Worker(shared::WorkerFunction worker, ContractState state) noexcept
        : m_worker{std::move(worker)}, m_flags{std::to_underlying(state)} {
        core::logger::debug("Worker", "Created with initial state `{}`", state);
    }


    Worker(Worker &&other) noexcept
        : m_worker{std::move(other.m_worker)}, m_flags{other.m_flags.exchange(0, std::memory_order_acq_rel)} {}

    Worker &operator=(Worker &&other) noexcept {
        if (this != &other) {
            m_worker = std::move(other.m_worker);
            m_flags.store(other.m_flags.load(std::memory_order_relaxed), std::memory_order_relaxed);
        }
        return *this;
    }

    ~Worker() {
        core::logger::debug("Worker", "Destructor called, clearing worker function and flags");
        if (m_worker) {
            m_worker = nullptr;
            m_flags.store(0, std::memory_order_release);
        }
    }

    Worker(const Worker &) = delete;
    Worker &operator=(const Worker &) = delete;

    void operator()() const {
        if (m_worker) {
            m_worker();
        }
    }

    auto fetch_or(ContractState state) noexcept {
        return static_cast<ContractState>(m_flags.fetch_or(std::to_underlying(state), std::memory_order_acq_rel));
    }

    auto fetch_and(ContractState state) noexcept {
        return static_cast<ContractState>(m_flags.fetch_and(std::to_underlying(state), std::memory_order_acq_rel));
    }

    auto load() const noexcept { return static_cast<ContractState>(m_flags.load(std::memory_order_acquire)); }

    bool compare_exchange(ContractState &expected, ContractState desired) noexcept {
        auto expected_val = std::to_underlying(expected);
        bool success = m_flags.compare_exchange_weak(expected_val, std::to_underlying(desired),
                                                     std::memory_order_acq_rel, std::memory_order_acquire);
        expected = static_cast<ContractState>(expected_val);
        return success;
    }

    // Adds a flag (e.g., used by schedule())
    void add_flags(ContractState state) noexcept { fetch_or(state); }

    // Removes a flag (e.g., used to clear EXECUTING after run)
    void remove_flags(ContractState state) noexcept {
        // Use bitwise NOT on the underlying type to clear the bit
        m_flags.fetch_and(~std::to_underlying(state), std::memory_order_acq_rel);
    }

    bool schedule() noexcept {
        auto prev = fetch_or(ContractState::SCHEDULED);
        if (std::to_underlying(prev & (ContractState::SCHEDULED | ContractState::EXECUTING)) == 0) {
            return true;
        }
        return false;
    }

    bool try_claim_execution() noexcept {
        auto current = load();

        core::logger::debug("Worker", "Trying to claim execution with current state `{}`", current);

        // Much cleaner and self-documenting
        while (is_scheduled()) {
            // Still need to calculate desired based on what 'current' is right now
            const auto desired = (current & ~ContractState::SCHEDULED) | ContractState::EXECUTING;

            if (compare_exchange(current, desired)) {
                core::logger::debug("Worker", "Successfully claimed execution, new state `{}`", desired);
                return true;
            }
        }
        return false;
    }

    void complete_execution() noexcept {
        remove_flags(ContractState::EXECUTING);
        add_flags(ContractState::IDLE);
    }

    [[nodiscard]] bool has_any(ContractState state) const noexcept {
        return (std::to_underlying(load()) & std::to_underlying(state)) != 0;
    }

    [[nodiscard]] bool is_scheduled() const noexcept { return has_any(ContractState::SCHEDULED); }
    [[nodiscard]] bool is_released() const noexcept { return has_any(ContractState::RELEASED); }

  private:
    shared::WorkerFunction m_worker;
    std::atomic<std::uint64_t> m_flags;
};

} // namespace core::contract
