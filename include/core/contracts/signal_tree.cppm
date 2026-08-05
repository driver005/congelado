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

    /**
     * @brief Builds an empty node — zeroed atomic value, and for a router, pre-allocates and
     * default-constructs its 8 child branches.
     */
    Node() noexcept : m_value{0} {
        // Leaves have no children — only routers need to pre-build their 8 branches.
        if constexpr (IsRouter) {
            m_children.reserve(8);
            for (int i = 0; i < 8; ++i) {
                m_children.emplace_back();
            }
        }
    }

    /**
     * @brief Default destructor — no manual cleanup needed, atomic and vector clean up
     * themselves.
     */
    ~Node() = default;

    // Custom move semantics for atomic compatibility
    /**
     * @brief Move ctor — atomics aren't movable by default, so this loads `other`'s value with
     * relaxed ordering and moves the children vector over by hand.
     * @param other the node being moved from.
     */
    Node(Node &&other) noexcept : m_children{std::move(other.m_children)} {
        m_value.store(other.m_value.load(std::memory_order_relaxed), std::memory_order_relaxed);
    }

    /**
     * @brief Move assign — same manual-atomic-move deal as the move ctor, self-assignment
     * guarded.
     * @param other the node being moved from.
     * @return `*this`.
     */
    Node &operator=(Node &&other) noexcept {
        if (this != &other) {
            m_value.store(other.m_value.load(std::memory_order_relaxed), std::memory_order_relaxed);
            m_children = std::move(other.m_children);
        }
        return *this;
    }

    /**
     * @brief Deleted — atomics plus a vector of child nodes aren't something you want silently
     * deep-copied, so copying's off the table.
     */
    Node(const Node &) = delete;
    /**
     * @brief Deleted, same reasoning as the copy ctor.
     */
    Node &operator=(const Node &) = delete;

    /**
     * @brief Grabs this node's raw packed value — 8 branch counters for a router, or a 64-bit
     * scheduled-bit mask for a leaf.
     * @return the current packed value, loaded with acquire ordering.
     */
    [[nodiscard]] std::uint64_t get_value() const noexcept { return m_value.load(std::memory_order_acquire); }

    /**
     * @brief Marks `local_id` as scheduled. For a router, bumps the relevant branch's 8-bit
     * counter (capped at 64, since that's the max a leaf can hold) and recurses into that
     * branch's child; for a leaf, just flips the matching bit.
     * @param local_id the id to schedule, relative to this node's own bit-space.
     */
    void schedule(std::uint32_t local_id) noexcept {
        if constexpr (IsRouter) {
            // Which of the 8 branches owns this id.
            const auto BRANCH_IDX = static_cast<std::uint8_t>((local_id >> 6) & 0x07);
            std::uint64_t expected = m_value.load(std::memory_order_acquire);

            // CAS loop — bump that branch's own 8-bit counter by one, capped at 64 (a leaf's
            // full bit-width) so it never bleeds into the neighboring branch's byte.
            while (true) {
                const auto COUNT = static_cast<std::uint8_t>((expected >> (BRANCH_IDX * 8)) & 0xFF);
                if (COUNT >= 64) {
                    break;
                }

                std::uint64_t desired = expected & ~(0xFFULL << (BRANCH_IDX * 8));
                desired |= (static_cast<std::uint64_t>(COUNT + 1) << (BRANCH_IDX * 8));

                if (m_value.compare_exchange_weak(expected, desired, std::memory_order_acq_rel,
                                                  std::memory_order_acquire)) {
                    break;
                }
            }
            // Counter's updated — bet, now recurse into the actual branch to flip the real bit.
            m_children[BRANCH_IDX].schedule(local_id & 0x3F);  // FIXME(clang-tidy): unchecked operator[], consider .at()
        } else {
            // Leaf — no counter, just flip the matching bit directly.
            const auto BIT = static_cast<std::uint8_t>(local_id & 0x3F);
            m_value.fetch_or(1ULL << BIT, std::memory_order_release);
        }
    }

    /**
     * @brief Marks `local_id` as descheduled. Mirror image of schedule() — recurses into the
     * child first for a router (decrementing that branch's counter after), or clears the
     * matching bit directly for a leaf. Straight up the other half of the schedule/deschedule
     * pair.
     * @param local_id the id to deschedule, relative to this node's own bit-space.
     */
    void deschedule(std::uint32_t local_id) noexcept {
        if constexpr (IsRouter) {
            const auto BRANCH_IDX = static_cast<std::uint8_t>((local_id >> 6) & 0x07);

            // Recurse into the branch first this time (mirror of schedule()'s order) to clear
            // the real bit before touching the counter.
            m_children[BRANCH_IDX].deschedule(local_id & 0x3F);  // FIXME(clang-tidy): unchecked operator[], consider .at()

            // CAS loop — decrement that branch's counter by one now that the bit's cleared.
            std::uint64_t expected = m_value.load(std::memory_order_acquire);
            while (true) {
                const auto COUNT = static_cast<std::uint8_t>((expected >> (BRANCH_IDX * 8)) & 0xFF);
                if (COUNT == 0) {
                    break;
                }

                std::uint64_t desired = expected & ~(0xFFULL << (BRANCH_IDX * 8));
                desired |= (static_cast<std::uint64_t>(COUNT - 1) << (BRANCH_IDX * 8));

                if (m_value.compare_exchange_weak(expected, desired, std::memory_order_acq_rel,
                                                  std::memory_order_acquire)) {
                    break;
                }
            }
        } else {
            // Leaf — just clear the matching bit directly.
            const auto BIT = static_cast<std::uint8_t>(local_id & 0x3F);
            m_value.fetch_and(~(1ULL << BIT), std::memory_order_release);
        }
    }

    /**
     * @brief Picks one scheduled id out from under this node, biasing which branch/bit gets
     * picked so load doesn't just pile up on the same corner every call. Bet — this is the
     * whole fairness engine behind the tree.
     * @warning `bias` gets mutated in place as part of the bias-tracking/rotation, so this
     * isn't a pure read — feeding the same `bias` through repeated calls is what keeps things
     * fair over time.
     * @param bias in/out bias state — read to pick a preferred branch/rotation point, then
     * updated to reflect the choice made.
     * @param accumulator the id bits accumulated from parent routers so far, shifted/OR'd
     * together as recursion descends.
     * @param bias_bit which bit of `bias` this node's level reads/writes; shifts right by one
     * per level of recursion.
     * @return the full scheduled id found under this node, or `std::nullopt` if nothing's
     * scheduled here.
     */
    [[nodiscard]] std::optional<std::uint32_t> select_child_index(std::uint64_t &bias, std::uint32_t accumulator = 0,
                                                                  std::uint64_t bias_bit = BIAS_FLAG) const noexcept {
        // Nothing scheduled anywhere under this node — dead end, nothing to find.
        const auto VAL = m_value.load(std::memory_order_acquire);
        if (VAL == 0) {
            return std::nullopt;
        }


        if constexpr (IsRouter) {
            // Try the biased/preferred branch first.
            auto idx = calculate_bias(VAL, bias, bias_bit);

            if (auto result = m_children[idx].select_child_index(bias, (accumulator << 3) | idx, bias_bit >> 1)) {  // FIXME(clang-tidy): unchecked operator[], consider .at()
                return result;
            }

            // Preferred branch came up empty (raced/emptied since the counter was read) —
            // fall back to scanning every other branch that still shows a nonzero count.
            for (std::uint8_t i = 0; i < 8; ++i) {
                if (i == idx) {
                    continue;
                }

                auto count = static_cast<std::uint8_t>((VAL >> (i * 8)) & 0xFF);
                if (count > 0) {
                    if (auto result = m_children[i].select_child_index(bias, (accumulator << 3) | i, bias_bit >> 1)) {  // FIXME(clang-tidy): unchecked operator[], consider .at()
                        return result;
                    }
                }
            }
        } else {
            // Rotate-scan: low 6 bits of bias are a rotating cursor (0-63).
            // Start scanning from that position, take the first set bit found.
            // This guarantees every scheduled contract is eventually reached,
            // avoiding the correlated-bias cycles that binary-tree traversal produces.
            const auto START = static_cast<std::uint8_t>(bias & 0x3F);
            const auto ROTATED = std::rotr(VAL, START);
            const auto OFFSET = static_cast<std::uint8_t>(std::countr_zero(ROTATED));
            const std::uint8_t BIT_IDX = (START + OFFSET) & 0x3F;
            bias = (bias & ~0x3FULL) | static_cast<std::uint64_t>((BIT_IDX + 1) & 0x3F);

            return (accumulator << 6) | BIT_IDX;
        }

        return std::nullopt;
    }

  private:
    /**
     * @brief Recursive halving helper for select_child_index() — walks `val` in half-sized
     * blocks, using `bias`/`bias_bit` to decide whether to prefer the right or left half at
     * each level, and flips `bias` to reflect whichever way it went.
     * @param val the packed value being searched (or the relevant half of it, on recursive
     * calls).
     * @param bias in/out bias state, read and updated as the recursion picks a direction.
     * @param bias_bit which bit of `bias` the current recursion level reads/writes; shifts
     * right by one per recursive call.
     * @param blocks how many 8-bit blocks wide the current search window is; halves each
     * recursive call.
     * @param base_shift byte offset into `val` where the current search window starts.
     * @return the index of the chosen half/branch within this level's window.
     */
    std::uint8_t calculate_bias(const std::uint64_t &val, std::uint64_t &bias, std::uint64_t &bias_bit,
                                std::uint8_t blocks = 4, std::uint8_t base_shift = 0) const noexcept {
        // Read this level's bias bit to see which half was preferred last time, then split
        // the current window into its right/left halves.
        const bool PREFER_RIGHT = (bias & bias_bit) != 0;
        const std::uint64_t MASK_LOWER = ((1ULL << (blocks * 8)) - 1) << (base_shift * 8);
        const std::uint64_t RIGHT_HALF = val & MASK_LOWER;
        const std::uint64_t LEFT_HALF = (val & ~MASK_LOWER) >> (blocks * 8);

        // Only actually go right if the preferred side has something in it — an empty right
        // half still routes left regardless of preference.
        const bool CHOOSE_RIGHT = ((PREFER_RIGHT && (RIGHT_HALF != 0)) || (LEFT_HALF == 0ULL));
        if (CHOOSE_RIGHT) {
            // Right's getting picked this round — flip the bit off so left's due for a turn
            // next time.
            bias &= ~bias_bit;

            bias_bit >>= 1;

            // Still more than one block wide — keep narrowing down into the right half.
            if (blocks > 1) {
                auto idx = calculate_bias(RIGHT_HALF, bias, bias_bit, blocks / 2, base_shift);
                if (!CHOOSE_RIGHT) {
                    idx += blocks;
                }
                return idx;
            }

            return static_cast<std::uint8_t>(!CHOOSE_RIGHT);
        }

        // Left's getting picked — flip the bit on so right's due for a turn next time.
        bias |= bias_bit;

        bias_bit >>= 1;

        // Still more than one block wide — keep narrowing down into the left half.
        if (blocks > 1) {
            auto idx = calculate_bias(LEFT_HALF, bias, bias_bit, blocks / 2, (base_shift + blocks));
            if (!CHOOSE_RIGHT) {
                idx += blocks;
            }
            return idx;
        }

        return static_cast<std::uint8_t>(!CHOOSE_RIGHT);

        // Test with more than 64!
        // std::print("Router Value: {:064b}\n", val);
        // std::print("Right Half: {:064b}\n", right_half);
        // std::print("Left Half: {:064b}\n", left_half);
        // std::print("Prefer Right: {}, Choose Right: {}\n", prefer_right, choose_right);
        // std::print("Blocks: {}, Base Shift: {}\n", blocks, base_shift);
        // std::print("Bias Before: {:064b}\n", bias);
        // std::print("Bias Bit: {:064b}\n", bias_bit);
        // std::print("Mask Lower: {:064b}\n", mask_lower);

        // bias_bit >>= 1;
        //
        // if (blocks > 1) {
        //     auto idx =
        //         calculate_bias(val, bias, bias_bit, blocks / 2, choose_right ? base_shift : (base_shift + blocks));
        //     if (!choose_right) {
        //         idx += blocks;
        //     }
        //     return idx;
        // }
        // return !choose_right;
    }
    std::atomic<std::uint64_t> m_value;
    [[no_unique_address]] std::conditional_t<IsRouter, std::vector<Node<false>>, std::monostate> m_children;
};

template <std::size_t MaxCapacity = 1024>
    requires(MaxCapacity > 0 && MaxCapacity % 512 == 0)
class SignalTree {
    static constexpr std::size_t NUM_ROUTERS = MaxCapacity / 512;

  public:
    /**
     * @brief Default ctor — all routers zero-initialized, nothing scheduled yet.
     */
    SignalTree() = default;

    /**
     * @brief Schedules `contract_id` by routing it down to the right router/branch/bit.
     * @throws std::out_of_range if `contract_id` is >= MaxCapacity.
     * @param contract_id the contract id to schedule.
     */
    void schedule(std::uint32_t contract_id) {
        // Guard clause — an id past MaxCapacity's cooked, no router exists to route it to.
        if (contract_id >= MaxCapacity) {
            throw std::out_of_range("ID exceeds maximum capacity");
        }
        // Split into which router owns it and its local id within that router's 512-wide span.
        const std::size_t ROUTER_IDX = contract_id / 512;
        m_routers[ROUTER_IDX].schedule(contract_id % 512);  // FIXME(clang-tidy): unchecked operator[], consider .at(); non-constant array index
    }

    /**
     * @brief Deschedules `contract_id`, mirror image of schedule().
     * @throws std::out_of_range if `contract_id` is >= MaxCapacity.
     * @param contract_id the contract id to deschedule.
     */
    void deschedule(std::uint32_t contract_id) {
        // Guard clause, same deal as schedule() above.
        if (contract_id >= MaxCapacity) {
            throw std::out_of_range("ID exceeds maximum capacity");
        }
        const std::size_t ROUTER_IDX = contract_id / 512;
        m_routers[ROUTER_IDX].deschedule(contract_id % 512);  // FIXME(clang-tidy): unchecked operator[], consider .at(); non-constant array index
    }

    /**
     * @brief Atomically claims and hands back the next free contract id.
     * @throws std::runtime_error if MaxCapacity's already been hit — no ids left to hand out,
     * that's an L for whoever's calling create() one too many times.
     * @return a freshly claimed, previously-unused contract id.
     */
    [[nodiscard]] std::uint32_t free_contract_id() {
        std::uint32_t current = m_next_id.load(std::memory_order_relaxed);
        // CAS loop — claim the current counter value and bump it, retrying if another thread
        // beat us to it (in which case `current` gets refreshed automatically by the failed CAS).
        while (true) {
            if (current >= MaxCapacity) {
                throw std::runtime_error("Maximum capacity reached");
            }
            if (m_next_id.compare_exchange_weak(current, current + 1U, std::memory_order_relaxed,
                                                std::memory_order_relaxed)) {
                return current;
            }
        }
    }

    /**
     * @brief Finds the next ready (scheduled) contract id across all routers, alternating
     * which end of the router array gets checked first based on `bias` so it's not always the
     * same router winning ties.
     * @param bias in/out bias state threaded through to select_child_index()/calculate_bias(),
     * flipped once a result's found so the next call favors the other side.
     * @return the next ready contract id, or `std::nullopt` if nothing's scheduled anywhere.
     */
    [[nodiscard]] std::optional<std::uint32_t> next(std::uint64_t &bias) const {
        const bool PREFER_RIGHT = (bias & BIAS_FLAG) != 0;

        // Walk every router, starting from whichever end the top bias bit prefers, so
        // ties don't always resolve toward the same router.
        for (std::size_t i = 0; i < NUM_ROUTERS; ++i) {
            const std::size_t IDX = PREFER_RIGHT ? (NUM_ROUTERS - 1 - i) : i;
            if (auto result = m_routers[IDX].select_child_index(bias, IDX, BIAS_FLAG >> 1)) {  // FIXME(clang-tidy): unchecked operator[], consider .at(); non-constant array index
                // Found one — flip the top bias bit so next call favors the other end.
                bias ^= BIAS_FLAG;
                return result;
            }
        }

        return std::nullopt;
    }

  private:
    std::array<Node<true>, NUM_ROUTERS> m_routers{};
    std::atomic<std::uint32_t> m_next_id;
};
} // namespace core::contract
