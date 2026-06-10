module core.contracts.signal_tree;
@nogc nothrow:

import core.atomic;
import core.contracts.consts : BIAS_FLAG;
import core.contracts.types  : ContractState;
import core.logger.logger;
import util.alloc : make, dispose;

// PORT-NOTE: C++ template<bool IsRouter> Node → two D classes NodeRouter and NodeBranch.
//   NodeRouter handles 8 child branches (counter-per-branch packed in m_value).
//   NodeBranch handles 64 individual bits in m_value.
//   std::conditional_t and [[no_unique_address]] are not needed.
// PORT-NOTE: std::vector<Node<false>> m_children → heap-allocated NodeBranch[8] via make!

class NodeBranch {
  public:
    this() { atomicStore!(MemoryOrder.raw)(m_value, 0UL); }

    ~this() {}

    ulong get_value() const { return atomicLoad!(MemoryOrder.acq)(m_value); }

    void schedule(uint local_id) {
        const ubyte bit = cast(ubyte)(local_id & 0x3F);
        atomicFetchOr!(MemoryOrder.rel)(m_value, 1UL << bit);
        debug_("core/signal_tree", "scheduled {} at bit {}", local_id, bit);
    }

    void deschedule(uint local_id) {
        const ubyte bit = cast(ubyte)(local_id & 0x3F);
        atomicFetchAnd!(MemoryOrder.rel)(m_value, ~(1UL << bit));
        debug_("core/signal_tree", "descheduled {} at bit {}", local_id, bit);
    }

    // PORT-NOTE: std::optional<uint32_t> → returns bool + sets out param
    bool select_child_index(ref ulong bias, uint accumulator, ulong bias_bit,
                            out uint result) const {
        const ulong val = atomicLoad!(MemoryOrder.acq)(m_value);
        if (val == 0)
            return false;

        // Rotate-scan: low 6 bits of bias are a rotating cursor (0-63).
        // Start scanning from that position, take the first set bit found.
        // This guarantees every scheduled contract is eventually reached,
        // avoiding the correlated-bias cycles that binary-tree traversal produces.
        const ubyte start   = cast(ubyte)(bias & 0x3F);
        const ulong rotated = (val >>> start) | (val << (64 - start));
        import core.bitop : bsf;
        const ubyte offset  = cast(ubyte)(rotated != 0 ? bsf(rotated) : 0);
        const ubyte BIT_IDX = (start + offset) & 0x3F;
        bias = (bias & ~0x3FUL) | cast(ulong)((BIT_IDX + 1) & 0x3F);

        debug_("core/signal_tree", "ready bit {}", BIT_IDX);
        result = (accumulator << 6) | BIT_IDX;
        return true;
    }

  private:
    shared ulong m_value;
}

class NodeRouter {
  public:
    this() {
        atomicStore!(MemoryOrder.raw)(m_value, 0UL);
        for (int i = 0; i < 8; ++i)
            m_children[i] = make!NodeBranch();
    }

    ~this() {
        for (int i = 0; i < 8; ++i) {
            dispose(m_children[i]);
            m_children[i] = null;
        }
    }

    ulong get_value() const { return atomicLoad!(MemoryOrder.acq)(m_value); }

    void schedule(uint local_id) {
        const ubyte branch_idx = cast(ubyte)((local_id >> 6) & 0x07);
        ulong expected = atomicLoad!(MemoryOrder.acq)(m_value);

        while (true) {
            const ubyte count = cast(ubyte)((expected >> (branch_idx * 8)) & 0xFF);
            if (count >= 64)
                break;

            ulong desired = expected & ~(0xFFUL << (branch_idx * 8));
            desired |= (cast(ulong)(count + 1) << (branch_idx * 8));

            if (cas!(MemoryOrder.acq_rel, MemoryOrder.acq)(&m_value, expected, desired)) {
                debug_("core/signal_tree", "scheduled {} from branch {}", local_id, branch_idx);
                break;
            }
        }
        m_children[branch_idx].schedule(local_id & 0x3F);
    }

    void deschedule(uint local_id) {
        const ubyte branch_idx = cast(ubyte)((local_id >> 6) & 0x07);

        m_children[branch_idx].deschedule(local_id & 0x3F);

        ulong expected = atomicLoad!(MemoryOrder.acq)(m_value);
        while (true) {
            const ubyte count = cast(ubyte)((expected >> (branch_idx * 8)) & 0xFF);
            if (count == 0)
                break;

            ulong desired = expected & ~(0xFFUL << (branch_idx * 8));
            desired |= (cast(ulong)(count - 1) << (branch_idx * 8));

            if (cas!(MemoryOrder.acq_rel, MemoryOrder.acq)(&m_value, expected, desired)) {
                debug_("core/signal_tree", "descheduled {} from branch {}", local_id, branch_idx);
                break;
            }
        }
    }

    bool select_child_index(ref ulong bias, uint accumulator, ulong bias_bit,
                            out uint result) const {
        const ulong val = atomicLoad!(MemoryOrder.acq)(m_value);
        if (val == 0)
            return false;

        uint idx = calculate_bias(val, bias, bias_bit);
        uint child_result;
        if (m_children[idx].select_child_index(bias, (accumulator << 3) | idx, bias_bit >> 1, child_result)) {
            result = child_result;
            return true;
        }

        for (ubyte i = 0; i < 8; ++i) {
            if (i == idx)
                continue;

            auto count = cast(ubyte)((val >> (i * 8)) & 0xFF);
            if (count > 0) {
                if (m_children[i].select_child_index(bias, (accumulator << 3) | i, bias_bit >> 1, child_result)) {
                    result = child_result;
                    return true;
                }
            }
        }

        return false;
    }

  private:
    ubyte calculate_bias(const ulong val, ref ulong bias, ref ulong bias_bit,
                         ubyte blocks = 4, ubyte base_shift = 0) const {
        const bool prefer_right = (bias & bias_bit) != 0;
        const ulong mask_lower = ((1UL << (blocks * 8)) - 1) << (base_shift * 8);
        const ulong right_half = val & mask_lower;
        const ulong left_half  = (val & ~mask_lower) >> (blocks * 8);

        const bool choose_right = ((prefer_right && (right_half != 0)) || (left_half == 0UL));
        if (choose_right) {
            bias &= ~bias_bit;
            bias_bit >>= 1;

            if (blocks > 1) {
                auto idx2 = calculate_bias(right_half, bias, bias_bit, cast(ubyte)(blocks / 2), base_shift);
                if (!choose_right)
                    idx2 += blocks;
                return idx2;
            }

            return cast(ubyte)(!choose_right);
        } else {
            bias |= bias_bit;
            bias_bit >>= 1;

            if (blocks > 1) {
                auto idx2 = calculate_bias(left_half, bias, bias_bit, cast(ubyte)(blocks / 2), cast(ubyte)(base_shift + blocks));
                if (!choose_right)
                    idx2 += blocks;
                return idx2;
            }

            return cast(ubyte)(!choose_right);
        }
    }

    shared ulong   m_value;
    NodeBranch[8]  m_children;
}

// PORT-NOTE: C++ template<size_t MaxCapacity = 1024> requires(MaxCapacity > 0 && MaxCapacity % 512 == 0)
// → D class SignalTree!(size_t MaxCapacity = 1024) with enum num_routers = MaxCapacity / 512

class SignalTree(size_t MaxCapacity = 1024) {
    static assert(MaxCapacity > 0 && MaxCapacity % 512 == 0,
                  "MaxCapacity must be a positive multiple of 512");
    enum size_t num_routers = MaxCapacity / 512;

  public:
    this() {
        for (size_t i = 0; i < num_routers; ++i)
            m_routers[i] = make!NodeRouter();
        atomicStore!(MemoryOrder.raw)(m_next_id, 0u);
    }

    ~this() {
        for (size_t i = 0; i < num_routers; ++i) {
            dispose(m_routers[i]);
            m_routers[i] = null;
        }
    }

    void schedule(uint id) {
        if (id >= MaxCapacity) {
            // PORT-NOTE: std::out_of_range → @nogc abort path; caller must guard
            return;
        }
        const size_t router_idx = id / 512;
        m_routers[router_idx].schedule(id % 512);
    }

    void deschedule(uint id) {
        if (id >= MaxCapacity)
            return;
        const size_t router_idx = id / 512;
        m_routers[router_idx].deschedule(id % 512);
    }

    uint free_contract_id() {
        uint current = atomicLoad!(MemoryOrder.raw)(m_next_id);
        while (true) {
            if (current >= MaxCapacity) {
                // PORT-NOTE: std::runtime_error → panic/abort; caller must not exceed capacity
                assert(false, "Maximum capacity reached");
            }
            if (cas!(MemoryOrder.raw, MemoryOrder.raw)(&m_next_id, current, current + 1u)) {
                debug_("core/signal_tree", "worker {} selected", current);
                return current;
            }
        }
    }

    // PORT-NOTE: std::optional<uint32_t> → bool + out param
    bool next(ref ulong bias, out uint result) const {
        const bool prefer_right = (bias & BIAS_FLAG) != 0;

        for (size_t i = 0; i < num_routers; ++i) {
            const size_t idx = prefer_right ? (num_routers - 1 - i) : i;
            uint res;
            if (m_routers[idx].select_child_index(bias, cast(uint)idx, BIAS_FLAG >> 1, res)) {
                debug_("core/signal_tree", "next worker {} bias {}", res, bias);
                bias ^= BIAS_FLAG;
                result = res;
                return true;
            }
        }

        debug_("core/signal_tree", "no ready worker, bias {}", bias);
        return false;
    }

  private:
    NodeRouter[num_routers] m_routers;
    shared uint              m_next_id;
}
