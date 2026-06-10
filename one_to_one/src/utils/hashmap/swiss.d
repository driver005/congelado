module utils.hashmap.swiss;
@nogc nothrow:

import core.bitop : bsf;
import core.stdc.stdlib : malloc, free, realloc;
import core.stdc.string : memset;
import util.optional;

// LDC SSE2 intrinsics for 16-byte SIMD group matching
version (X86_64) {
    import ldc.gccbuiltins_x86;
}

enum size_t kGroupWidth = 16;

// ---------------------------------------------------------------------------
// Entry<K,V> — key/value pair stored in raw slots.
// PORT-NOTE: value wrapper, exempt from classes-only rule
// (Returned frequently by iterator dereference on hot paths.)
// ---------------------------------------------------------------------------
struct Entry(K, V) {
    // PORT-NOTE: value wrapper, exempt from classes-only rule
    this(K k, V v) { m_key = k; m_value = v; }

    ref K       key()             { return m_key; }
    ref const K key()       const { return m_key; }
    ref V       value()           { return m_value; }
    ref const V value() const     { return m_value; }

    // NOTE: used by get!I for structured bindings, not intended for direct use.
    ref auto get(size_t I)() {
        static if (I == 0) return m_key;
        else               return m_value;
    }

    // NOTE: used by get!I for structured bindings, not intended for direct use.
    ref const auto get(size_t I)() const {
        static if (I == 0) return m_key;
        else               return m_value;
    }

  private:
    K m_key;
    V m_value;
}

// NOTE: free function get!I for structured bindings, not intended for direct use.
ref auto get(size_t I, K, V)(ref Entry!(K, V) e)       { return e.get!I(); }
// NOTE: free function get!I for structured bindings, not intended for direct use.
ref const auto get(size_t I, K, V)(ref const Entry!(K, V) e) { return e.get!I(); }

// ---------------------------------------------------------------------------
// SwissHashMap — open-addressed hash map using SSE2-accelerated SIMD probing.
// Mirrors C++ hashmap::swiss::SwissHashMap.
// ---------------------------------------------------------------------------
class SwissHashMap(K, V,
                   alias Hash     = defaultHash!K,
                   alias KeyEqual = defaultEqual!K,
                   alias ValEqual = defaultEqual!V) {
    @disable this(this);

    alias EntryType = Entry!(K, V);

    // Raw uninitialised storage for one Entry — no K or V construction until
    // placement new fires in insert_impl. alignas ensures placement new is valid.
    align(EntryType.alignof) struct RawSlot {
        ubyte[EntryType.sizeof] data;
    }

    enum ControlByte : ubyte {
        kEmpty    = 0xFF,
        kDeleted  = 0x7E,
        kSentinel = 0xFE,
    }

    // -- IteratorBase --------------------------------------------------------
    class IteratorBase(bool IsConst) {
        static if (IsConst) {
            alias MapRef  = const SwissHashMap;
            alias EntryRef = ref const(EntryType);
            alias EntryPtr = const(EntryType)*;
        } else {
            alias MapRef  = SwissHashMap;
            alias EntryRef = ref EntryType;
            alias EntryPtr = EntryType*;
        }

        MapRef  m_map;
        size_t  m_idx;

        void advance() {
            while (m_idx < m_map.capacity_) {
                ubyte c = m_map.ctrl_[m_idx];
                if (c != ControlByte.kEmpty && c != ControlByte.kDeleted &&
                    c != ControlByte.kSentinel)
                    break;
                ++m_idx;
            }
        }

        this(MapRef map, size_t idx) {
            m_map = map;
            m_idx = idx;
            advance();
        }

        EntryRef front()  { return m_map._slot(m_idx); }
        bool     empty()  const { return m_idx >= m_map.capacity_; }

        void popFront() {
            ++m_idx;
            advance();
        }

        bool opEquals(const IteratorBase other) const {
            return m_idx == other.m_idx;
        }
    }

    alias Iterator      = IteratorBase!false;
    alias ConstIterator = IteratorBase!true;

    Iterator      begin()  { return make!Iterator(this, 0); }
    Iterator      end()    { return make!Iterator(this, capacity_); }
    ConstIterator begin()  const { return make!ConstIterator(this, 0); }
    ConstIterator end()    const { return make!ConstIterator(this, capacity_); }
    ConstIterator cbegin() const { return begin(); }
    ConstIterator cend()   const { return end(); }
    // -- End IteratorBase ----------------------------------------------------

    this() {
        capacity_ = 0;
        size_     = 0;
        ctrl_     = null;
        slots_raw_ = null;
    }

    ~this() { destroy_all(); }

    void insert(K key, V value) {
        if (capacity_ == 0 || size_ >= (capacity_ * 7) / 8)
            rehash();
        insert_impl(key, value);
    }

    // find — returns pointer to value or null.
    // PORT-NOTE: null = empty (replaces std::optional)
    V* find(Args...)(Args args) const {
        if (capacity_ == 0) return null;

        size_t h    = Hash(args);
        size_t grp  = h1(h);
        ubyte  fp   = h2(h);

        for (size_t probe = 0; probe < (capacity_ / kGroupWidth); ++probe) {
            size_t cur   = (grp + probe) % (capacity_ / kGroupWidth);
            uint   match = match_byte(cur, fp);

            while (match != 0) {
                int    bit = bsf(match);
                size_t s   = cur * kGroupWidth + bit;
                if (KeyEqual(_slot(s).key(), args))
                    return &_slot(s).value();
                match &= ~(1u << bit);
            }

            if (match_byte(cur, ControlByte.kEmpty) != 0)
                break;
        }

        return null;
    }

    // upsert — insert if not present, update value if already present.
    // Returns some(true) if inserted, some(false) if updated, none() if no slot.
    // PORT-NOTE: C++ returns std::optional<bool>; D uses Optional!bool (value struct, no GC allocation)
    Optional!bool upsert(KeyArg, ValueArg)(KeyArg key, ValueArg value) {
        if (capacity_ == 0 || size_ >= (capacity_ * 7) / 8)
            rehash();

        size_t h   = Hash(key);
        size_t grp = h1(h);
        ubyte  fp  = h2(h);

        for (size_t probe = 0; probe < (capacity_ / kGroupWidth); ++probe) {
            size_t cur   = (grp + probe) % (capacity_ / kGroupWidth);
            uint   match = match_byte(cur, fp);

            while (match != 0) {
                int    bit = bsf(match);
                size_t s   = cur * kGroupWidth + bit;
                if (KeyEqual(_slot(s).key(), key)) {
                    _slot(s).value() = value;
                    return Optional!bool.some(false);
                }
                match &= ~(1u << bit);
            }

            if (match_byte(cur, ControlByte.kEmpty) != 0)
                break;
        }

        insert_impl(key, value);
        return Optional!bool.some(true);
    }

    void erase(Args...)(Args args) {
        if (capacity_ == 0) return;

        size_t h   = Hash(args);
        size_t grp = h1(h);
        ubyte  fp  = h2(h);

        for (size_t probe = 0; probe < (capacity_ / kGroupWidth); ++probe) {
            size_t cur   = (grp + probe) % (capacity_ / kGroupWidth);
            uint   match = match_byte(cur, fp);

            while (match != 0) {
                int    bit = bsf(match);
                size_t s   = cur * kGroupWidth + bit;
                if (KeyEqual(_slot(s).key(), args)) {
                    destroy(_slot(s));
                    ctrl_[s] = ControlByte.kDeleted;
                    size_--;
                    return;
                }
                match &= ~(1u << bit);
            }

            if (match_byte(cur, ControlByte.kEmpty) != 0)
                break;
        }
    }

    void clear() {
        destroy_all();
        if (capacity_ > 0)
            memset(ctrl_, ControlByte.kEmpty, capacity_ + kGroupWidth);
        size_ = 0;
    }

    size_t size()  const { return size_; }
    bool   empty() const { return size_ == 0; }

  private:
    ubyte*    ctrl_;
    RawSlot*  slots_raw_;
    size_t    capacity_;
    size_t    size_;

    ref EntryType _slot(size_t i) {
        return *cast(EntryType*) &slots_raw_[i];
    }
    ref const(EntryType) _slot(size_t i) const {
        return *cast(const EntryType*) &slots_raw_[i];
    }

    void destroy_all() {
        if (capacity_ == 0) return;
        for (size_t i = 0; i < capacity_; ++i) {
            if (ctrl_[i] != ControlByte.kEmpty &&
                ctrl_[i] != ControlByte.kDeleted &&
                ctrl_[i] != ControlByte.kSentinel)
                destroy(_slot(i));
        }
        free(ctrl_);
        free(slots_raw_);
        ctrl_      = null;
        slots_raw_ = null;
        capacity_  = 0;
        size_      = 0;
    }

    size_t h1(size_t hash) const { return (hash >> 7) % (capacity_ / kGroupWidth); }

    ubyte h2(size_t hash) const {
        ubyte fp = cast(ubyte)(hash & 0x7F);
        if (fp == 0x7E) return 0x7D;
        if (fp == 0x7F) return 0x7C;
        return fp;
    }

    uint match_byte(size_t group_idx, ubyte target) const {
        size_t off = group_idx * kGroupWidth;
        version (X86_64) {
            __m128i grp = __builtin_ia32_loaddqu(cast(const(char)*) &ctrl_[off]);
            __m128i tgt = __builtin_ia32_vec_set_v16qi(
                __builtin_ia32_vec_init_v16qi(), cast(char) target, 0);
            // broadcast target across all lanes
            tgt = __builtin_ia32_pshufb128(tgt, __builtin_ia32_vec_init_v16qi());
            __m128i cmp = __builtin_ia32_pcmpeqb128(grp, tgt);
            return cast(uint) __builtin_ia32_pmovmskb128(cmp);
        } else {
            // Scalar fallback for non-x86 targets
            uint result = 0;
            for (int i = 0; i < cast(int) kGroupWidth; ++i) {
                if (ctrl_[off + i] == target)
                    result |= (1u << i);
            }
            return result;
        }
    }

    uint match_empty_or_deleted(size_t group_idx) const {
        size_t off = group_idx * kGroupWidth;
        version (X86_64) {
            __m128i grp       = __builtin_ia32_loaddqu(cast(const(char)*) &ctrl_[off]);
            __m128i empty_tgt = __builtin_ia32_vec_init_v16qi();
            // broadcast kEmpty
            __m128i e = __builtin_ia32_vec_set_v16qi(
                empty_tgt, cast(char) ControlByte.kEmpty, 0);
            e = __builtin_ia32_pshufb128(e, empty_tgt);
            __m128i d = __builtin_ia32_vec_set_v16qi(
                empty_tgt, cast(char) ControlByte.kDeleted, 0);
            d = __builtin_ia32_pshufb128(d, empty_tgt);
            __m128i empty_mask = __builtin_ia32_pcmpeqb128(grp, e);
            __m128i del_mask   = __builtin_ia32_pcmpeqb128(grp, d);
            return cast(uint) __builtin_ia32_pmovmskb128(
                __builtin_ia32_por128(empty_mask, del_mask));
        } else {
            uint result = 0;
            for (int i = 0; i < cast(int) kGroupWidth; ++i) {
                ubyte c = ctrl_[off + i];
                if (c == ControlByte.kEmpty || c == ControlByte.kDeleted)
                    result |= (1u << i);
            }
            return result;
        }
    }

    void rehash() {
        size_t    old_capacity = capacity_;
        ubyte*    old_ctrl     = ctrl_;
        RawSlot*  old_slots    = slots_raw_;

        capacity_ = (old_capacity == 0) ? 16 * kGroupWidth : old_capacity * 2;

        ctrl_ = cast(ubyte*) malloc(capacity_ + kGroupWidth);
        memset(ctrl_, ControlByte.kEmpty, capacity_ + kGroupWidth);

        slots_raw_ = cast(RawSlot*) malloc(capacity_ * RawSlot.sizeof);
        size_ = 0;

        for (size_t i = 0; i < old_capacity; ++i) {
            if (old_ctrl[i] != ControlByte.kEmpty &&
                old_ctrl[i] != ControlByte.kDeleted &&
                old_ctrl[i] != ControlByte.kSentinel) {
                EntryType* e = cast(EntryType*) &old_slots[i];
                insert_impl(e.key(), e.value());
                destroy(*e); // explicit dtor — malloc only frees bytes
            }
        }

        free(old_ctrl);
        free(old_slots);
    }

    void insert_impl(KeyArg, ValueArg)(KeyArg key, ValueArg value) {
        size_t h   = Hash(key);
        size_t grp = h1(h);
        ubyte  fp  = h2(h);

        for (size_t probe = 0; probe < (capacity_ / kGroupWidth); ++probe) {
            size_t cur   = (grp + probe) % (capacity_ / kGroupWidth);
            uint   avail = match_empty_or_deleted(cur);

            if (avail != 0) {
                int    bit = bsf(avail);
                size_t s   = cur * kGroupWidth + bit;

                ctrl_[s] = fp;
                // Construct DIRECTLY in the slot without intermediate temporaries
                import core.lifetime : emplace;
                emplace(cast(EntryType*) &slots_raw_[s], key, value);
                size_++;
                return;
            }
        }
        // Swiss table exhausted — this should not happen if load factor is managed
        assert(false, "Swiss table exhausted");
    }
}

// ---------------------------------------------------------------------------
// Default hash / equality helpers (free functions, @nogc nothrow).
// ---------------------------------------------------------------------------
size_t defaultHash(K)(K key) {
    // Simple FNV-1a for scalar types; specialise as needed.
    static if (is(K == size_t) || is(K == uint) || is(K == ulong)) {
        return cast(size_t) key;
    } else static if (is(K : const(char)[])) {
        size_t h = 14695981039346656037UL;
        foreach (c; key) {
            h ^= cast(size_t) c;
            h *= 1099511628211UL;
        }
        return h;
    } else {
        // Fallback: hash the raw bytes.
        auto bytes = (cast(const(ubyte)*) &key)[0 .. K.sizeof];
        size_t h = 14695981039346656037UL;
        foreach (b; bytes) {
            h ^= cast(size_t) b;
            h *= 1099511628211UL;
        }
        return h;
    }
}

bool defaultEqual(K)(K a, K b) { return a == b; }
