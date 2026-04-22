export module core_contract:signal_tree;

import std;
import core_logger;
import :consts;
import :types;

export namespace core::contract {

/**
 * @brief Unified Node template that acts either as a Router or a Branch.
 * @tparam IsRouter If true, handles 8 child branches. If false, handles 64 individual bits.
 */
template <bool IsRouter>
class Node {
  public:
    using ChildType = std::conditional_t<IsRouter, Node<false>, std::monostate>;

    Node() noexcept : m_value{0} {
        core::logger::debug("SignalTree - Node", "Created a new `{}` node", IsRouter ? "Router" : "Branch");
        if constexpr (IsRouter) {
            m_children.reserve(8);
            for (int i = 0; i < 8; ++i) {
                m_children.emplace_back();
            }
        }
    }

    // Custom move semantics for atomic compatibility
    Node(Node &&other) noexcept : m_children{std::move(other.m_children)} {
        m_value.store(other.m_value.load(std::memory_order_relaxed), std::memory_order_relaxed);
    }

    Node &operator=(Node &&other) noexcept {
        if (this != &other) {
            m_value.store(other.m_value.load(std::memory_order_relaxed), std::memory_order_relaxed);
            m_children = std::move(other.m_children);
        }
        return *this;
    }

    Node(const Node &) = delete;
    Node &operator=(const Node &) = delete;

    [[nodiscard]] std::uint64_t value() const noexcept { return m_value.load(std::memory_order_acquire); }

    void schedule(std::uint32_t local_id) noexcept {
        if constexpr (IsRouter) {
            const std::uint8_t branch_idx = static_cast<std::uint8_t>((local_id >> 6) & 0x07);
            std::uint64_t expected = m_value.load(std::memory_order_acquire);

            while (true) {
                const std::uint8_t count = static_cast<std::uint8_t>((expected >> (branch_idx * 8)) & 0xFF);
                if (count >= 64) {
                    break;
                }

                std::uint64_t desired = expected & ~(0xFFULL << (branch_idx * 8));
                desired |= (static_cast<std::uint64_t>(count + 1) << (branch_idx * 8));

                if (m_value.compare_exchange_weak(expected, desired, std::memory_order_acq_rel,
                                                  std::memory_order_acquire)) {
                    core::logger::debug("SignalTree - Node", "Scheduled local ID `{}` from branch `{}`", local_id,
                                        branch_idx);
                    break;
                }
            }
            m_children[branch_idx].schedule(local_id & 0x3F);
        } else {
            const std::uint8_t bit = static_cast<std::uint8_t>(local_id & 0x3F);
            m_value.fetch_or(1ULL << bit, std::memory_order_release);
            core::logger::debug("SignalTree - Node", "Scheduled local ID `{}` by setting bit `{}`", local_id, bit);
        }
    }

    void deschedule(std::uint32_t local_id) noexcept {
        if constexpr (IsRouter) {
            const std::uint8_t branch_idx = static_cast<std::uint8_t>((local_id >> 6) & 0x07);
            std::uint64_t expected = m_value.load(std::memory_order_acquire);

            while (true) {
                const std::uint8_t count = static_cast<std::uint8_t>((expected >> (branch_idx * 8)) & 0xFF);
                if (count == 0) {
                    break;
                }

                std::uint64_t desired = expected & ~(0xFFULL << (branch_idx * 8));
                desired |= (static_cast<std::uint64_t>(count - 1) << (branch_idx * 8));

                if (m_value.compare_exchange_weak(expected, desired, std::memory_order_acq_rel,
                                                  std::memory_order_acquire)) {
                    core::logger::debug("SignalTree - Node", "Descheduled local ID `{}` from branch `{}`", local_id,
                                        branch_idx);
                    break;
                }
            }
            m_children[branch_idx].deschedule(local_id & 0x3F);
        } else {
            const std::uint8_t bit = static_cast<std::uint8_t>(local_id & 0x3F);
            m_value.fetch_and(~(1ULL << bit), std::memory_order_release);
            core::logger::debug("SignalTree - Node", "Descheduled local ID `{}` by clearing bit `{}`", local_id, bit);
        }
    }

    [[nodiscard]] std::optional<std::uint32_t> select_child_index(std::uint64_t bias_flags,
                                                                  std::uint32_t accumulator = 0) const noexcept {
        const std::uint64_t val = m_value.load(std::memory_order_acquire);
        if (val == 0) {
            return std::nullopt;
        }

        const bool prefer_right = (bias_flags & BIAS_FLAG) != 0;

        if constexpr (IsRouter) {
            for (int i = 0; i < 8; ++i) {
                const std::uint8_t idx = prefer_right ? (7 - i) : i;
                const std::uint8_t count = static_cast<std::uint8_t>((val >> (idx * 8)) & 0xFF);

                if (count > 0) {
                    if (auto result = m_children[idx].select_child_index(bias_flags, (accumulator << 3) | idx)) {
                        return result;
                    }
                }
            }
        } else {
            const std::uint8_t bit_idx = prefer_right ? static_cast<std::uint8_t>(63 - std::countl_zero(val))
                                                      : static_cast<std::uint8_t>(std::countr_zero(val));
            core::logger::debug("SignalTree - Node", "Found ready bit `{}`", bit_idx);
            return (accumulator << 6) | bit_idx;
        }

        return std::nullopt;
    }

  private:
    std::atomic<std::uint64_t> m_value;
    [[no_unique_address]] std::conditional_t<IsRouter, std::vector<Node<false>>, std::monostate> m_children;
};

template <std::size_t MaxCapacity = 1024>
    requires(MaxCapacity > 0 && MaxCapacity % 512 == 0)
class SignalTree {
    static constexpr std::size_t num_routers = MaxCapacity / 512;

  public:
    SignalTree() : m_routers{} {};

    void schedule(std::uint32_t id) {
        if (id >= MaxCapacity) {
            throw std::out_of_range("ID exceeds maximum capacity");
        }
        const std::size_t router_idx = id / 512;
        m_routers[router_idx].schedule(id % 512);
    }

    void deschedule(std::uint32_t id) {
        if (id >= MaxCapacity) {
            throw std::out_of_range("ID exceeds maximum capacity");
        }
        const std::size_t router_idx = id / 512;
        m_routers[router_idx].deschedule(id % 512);
    }

    [[nodiscard]] std::uint32_t free_contract_id() {
        std::uint32_t current = m_next_id.load(std::memory_order_relaxed);
        while (true) {
            if (current >= MaxCapacity) {
                throw std::runtime_error("Maximum capacity reached");
            }
            if (m_next_id.compare_exchange_weak(current, current + 1u, std::memory_order_relaxed,
                                                std::memory_order_relaxed)) {
                core::logger::debug("SignalTree", "Worker ID `{}` will be used", current);
                return current;
            }
        }
    }

    [[nodiscard]] std::optional<std::uint32_t> next(std::uint64_t bias) const {
        const bool prefer_right = (bias & BIAS_FLAG) != 0;

        for (std::size_t i = 0; i < num_routers; ++i) {
            const std::size_t idx = prefer_right ? (num_routers - 1 - i) : i;
            if (auto result = m_routers[idx].select_child_index(bias)) {
                core::logger::debug("SignalTree", "Next ready worker ID is `{}` with bias `{}`", *result + (idx * 512),
                                    bias);
                return static_cast<std::uint32_t>(*result + (idx * 512));
            }
        }

        core::logger::debug("SignalTree", "No ready worker found with bias `{}`", bias);
        return std::nullopt;
    }

  private:
    std::array<Node<true>, num_routers> m_routers;
    std::atomic<std::uint32_t> m_next_id;
};
} // namespace core::contract
