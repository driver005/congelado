module;

#include <immintrin.h>

export module hashmap:swiss;

import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

constexpr std::size_t GROUP_WIDTH = 16;

export namespace hashmap::swiss {

template <typename K, typename V>
struct Entry {
    /**
     * @brief Builds an entry from a key/value pair, moving both in.
     * @param key_arg the key to store.
     * @param value_arg the value to store.
     */
    Entry(K key_arg, V value_arg) : m_key(std::move(key_arg)), m_value(std::move(value_arg)) {}

    /**
     * @brief Grabs the key.
     * @return a mutable reference to the stored key.
     */
    K &key() { return m_key; }
    /**
     * @brief Const overload of key().
     * @return a read-only reference to the stored key.
     */
    [[nodiscard]] const K &key() const { return m_key; }
    /**
     * @brief Grabs the value.
     * @return a mutable reference to the stored value.
     */
    V &value() { return m_value; }
    /**
     * @brief Const overload of value().
     * @return a read-only reference to the stored value.
     */
    [[nodiscard]] const V &value() const { return m_value; }

    // NOTE: used by compiler-generated get<I> for structured bindings, not intended for direct use.
    /**
     * @brief Structured-bindings hook — `I == 0` gets the key, anything else gets the value.
     * Not meant to be called directly, that `NOTE` above isn't playing.
     * @tparam I the tuple-like index (0 for key, 1 for value).
     * @return a mutable reference to the key or value depending on `I`.
     */
    template <std::size_t I>
    auto &get() {
        if constexpr (I == 0) {
            return m_key;
        } else {
            return m_value;
        }
    }

    // NOTE: used by compiler-generated get<I> for structured bindings, not intended for direct use.
    /**
     * @brief Const overload of the structured-bindings get<I>() hook.
     * @tparam I the tuple-like index (0 for key, 1 for value).
     * @return a read-only reference to the key or value depending on `I`.
     */
    template <std::size_t I>
    const auto &get() const {
        if constexpr (I == 0) {
            return m_key;
        } else {
            return m_value;
        }
    }

  private:
    K m_key;
    V m_value;
};

// NOTE: used by compiler-generated get<I> for structured bindings, not intended for direct use.
/**
 * @brief ADL-found free function backing the tuple-like protocol for `Entry<K, V>` — forwards
 * straight to the member get<I>().
 * @note This one's a free function sitting in `namespace hashmap::swiss`, not a class static
 * method — flagging the convention mismatch (this codebase's rule is class-only, no free
 * functions). Also worth noting: since `Entry` already has a member `get<I>()`, structured
 * bindings resolve to the member version per the language rules anyway, making this free
 * overload largely redundant — it's not actually the one doing the work for `auto [k, v] = entry`.
 * Leaving the structure alone, this pass is comment-only.
 * @tparam I the tuple-like index (0 for key, 1 for value).
 * @tparam K the entry's key type.
 * @tparam V the entry's value type.
 * @param entry the entry to pull from.
 * @return a mutable reference to the key or value depending on `I`.
 */
template <std::size_t I, typename K, typename V>
auto &get(Entry<K, V> &entry) {
    return entry.template get<I>();
}

// NOTE: used by compiler-generated get<I> for structured bindings, not intended for direct use.
/**
 * @brief Const overload of the free get<I>() above, same convention-mismatch note applies.
 * @tparam I the tuple-like index (0 for key, 1 for value).
 * @tparam K the entry's key type.
 * @tparam V the entry's value type.
 * @param entry the entry to pull from.
 * @return a read-only reference to the key or value depending on `I`.
 */
template <std::size_t I, typename K, typename V>
const auto &get(const Entry<K, V> &entry) {
    return entry.template get<I>();
}

template <typename K, typename V, typename Hash = std::hash<K>, typename KeyEqual = std::equal_to<K>,
          typename ValueEqual = std::equal_to<V>>
class SwissHashMap {
  public:
    using Entry = Entry<K, V>;

    // Raw uninitialised storage for one Entry — no K or V construction until
    // placement new fires in insert_impl. alignas ensures placement new is valid.
    class alignas(alignof(Entry)) RawSlot {
      public:
        // Destructor declared first (and defaulted on its only declaration) so the compiler
        // treats this as trivially destructible — it's raw byte storage, nothing to tear down.
        ~RawSlot() = default;
        RawSlot() = default;
        RawSlot(const RawSlot &) = default;
        RawSlot &operator=(const RawSlot &) = default;
        RawSlot(RawSlot &&) = default;
        RawSlot &operator=(RawSlot &&) = default;

      private:
        std::byte m_data[sizeof(Entry)];
    };

    enum class ControlByte : std::uint8_t {
        EMPTY = 0xFF,
        DELETED = 0x7E,
        SENTINEL = 0xFE,
    };

    template <bool IsConst>
    class IteratorBase {
        using MapPtr = std::conditional_t<IsConst, const SwissHashMap *, SwissHashMap *>;
        using EntryRef = std::conditional_t<IsConst, const Entry &, Entry &>;
        using EntryPtr = std::conditional_t<IsConst, const Entry *, Entry *>;

        MapPtr m_map;
        std::size_t m_idx;

        /**
         * @brief Skips forward over empty/deleted/sentinel control bytes until it lands on a live
         * slot (or runs off the end of the table).
         */
        void advance() {
            // Walk forward until we land on a live slot or fall off the end of the table.
            while (m_idx < m_map->m_capacity) {
                std::uint8_t byte_value = m_map->m_control[m_idx];  // FIXME(clang-tidy): unchecked operator[], consider .at()
                if (byte_value != static_cast<std::uint8_t>(SwissHashMap::ControlByte::EMPTY) &&
                    byte_value != static_cast<std::uint8_t>(SwissHashMap::ControlByte::DELETED) &&
                    byte_value != static_cast<std::uint8_t>(SwissHashMap::ControlByte::SENTINEL)) {
                    break;
                }
                ++m_idx;
            }
        }

      public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = Entry;
        using difference_type = std::ptrdiff_t;
        using reference = EntryRef;
        using pointer = EntryPtr;

        /**
         * @brief Defaulted destructor — declared first (and only) so the class stays trivially
         * destructible instead of relying on an implicit one.
         */
        ~IteratorBase() = default;

        /**
         * @brief Builds an iterator at slot `idx` in `map`, then immediately advance()s past any
         * dead slots sitting right there.
         * @param map the table this iterator walks.
         * @param idx the starting slot index.
         */
        IteratorBase(MapPtr map, std::size_t idx) : m_map(map), m_idx(idx) { advance(); }
        IteratorBase(const IteratorBase &) = default;
        IteratorBase &operator=(const IteratorBase &) = default;
        IteratorBase(IteratorBase &&) = default;
        IteratorBase &operator=(IteratorBase &&) = default;

        /**
         * @brief Dereferences the entry at the current slot.
         * @return a reference to the live entry.
         */
        reference operator*() const { return m_map->slot(m_idx); }
        /**
         * @brief Arrow overload, mirrors operator*().
         * @return a pointer to the live entry.
         */
        pointer operator->() const { return &m_map->slot(m_idx); }

        /**
         * @brief Steps to the next live slot, skipping dead ones via advance().
         * @return `*this`, advanced.
         */
        IteratorBase &operator++() {
            ++m_idx;
            advance();
            return *this;
        }
        /**
         * @brief Postfix advance — copies the current state out before stepping forward.
         * @return the iterator's state before this call.
         */
        IteratorBase operator++(int) {
            auto tmp = *this;
            ++(*this);
            return tmp;
        }

        /**
         * @brief Equality check against another iterator.
         * @param other the iterator to compare against.
         * @return true if both are parked at the same slot index.
         */
        bool operator==(const IteratorBase &other) const { return m_idx == other.m_idx; }
        /**
         * @brief Inequality check against another iterator.
         * @param other the iterator to compare against.
         * @return true if the slot indices differ.
         */
        bool operator!=(const IteratorBase &other) const { return m_idx != other.m_idx; }
    };

    using iterator = IteratorBase<false>;
    using const_iterator = IteratorBase<true>;

    /**
     * @brief Start of the table.
     * @return a mutable iterator at slot 0, already advanced past any dead leading slots.
     */
    iterator begin() { return {this, 0}; }
    /**
     * @brief End of the table.
     * @return a mutable iterator parked at `m_capacity`, the past-the-end sentinel position.
     */
    iterator end() { return {this, m_capacity}; }
    /**
     * @brief Const overload of begin().
     * @return a read-only iterator at slot 0.
     */
    [[nodiscard]] const_iterator begin() const { return {this, 0}; }
    /**
     * @brief Const overload of end().
     * @return a read-only iterator at the past-the-end position.
     */
    [[nodiscard]] const_iterator end() const { return {this, m_capacity}; }
    /**
     * @brief Explicit const begin(), for when you want cbegin() over begin() on a mutable map.
     * @return a read-only iterator at slot 0.
     */
    [[nodiscard]] const_iterator cbegin() const { return begin(); }
    /**
     * @brief Explicit const end().
     * @return a read-only iterator at the past-the-end position.
     */
    [[nodiscard]] const_iterator cend() const { return end(); }

    /**
     * @brief Builds an empty table — no capacity allocated yet, first insert() triggers the
     * initial rehash().
     */
    SwissHashMap() = default;

    /**
     * @brief Runs every live entry's destructor via destroy_all().
     */
    ~SwissHashMap() { destroy_all(); }

    /**
     * @brief Deleted — no copying, this table owns raw uninitialized storage with manually
     * managed entry lifetimes, a deep copy isn't free and isn't implemented.
     */
    SwissHashMap(const SwissHashMap &) = delete;
    /**
     * @brief Deleted, same reasoning as the copy ctor.
     */
    SwissHashMap &operator=(const SwissHashMap &) = delete;

    /**
     * @brief Move ctor — steals `other`'s control bytes and raw slot storage outright, leaves
     * `other` at capacity/size zero (an empty, safely-destructible table).
     * @param other the table to move from.
     */
    SwissHashMap(SwissHashMap &&other) noexcept
        : m_control(std::move(other.m_control)), m_raw_slots(std::move(other.m_raw_slots)),
          m_capacity(other.m_capacity), m_size(other.m_size) {
        other.m_capacity = 0;
        other.m_size = 0;
    }

    /**
     * @brief Move assignment — destroys whatever `*this` already held via destroy_all(), then
     * steals `other`'s state.
     * @param other the table to move from, left empty after.
     * @return `*this`, now holding `other`'s table.
     */
    SwissHashMap &operator=(SwissHashMap &&other) noexcept {
        if (this != &other) {
            // Tear down whatever entries `*this` already owned first, or they'd leak once the
            // backing vectors below get overwritten.
            destroy_all();
            m_control = std::move(other.m_control);
            m_raw_slots = std::move(other.m_raw_slots);
            m_capacity = other.m_capacity;
            m_size = other.m_size;
            other.m_capacity = 0;
            other.m_size = 0;
        }
        return *this;
    }

    /**
     * @brief Inserts `key`/`value` unconditionally — grows the table first via rehash() if the
     * load factor's about to cross 7/8, then places the entry through insert_impl().
     * @warning No duplicate-key check here — insert() doesn't look for an existing key first,
     * that's what upsert() is for. Insert a key that's already present and you get two live
     * slots with the same key, which is a straight L for anything that assumes uniqueness (find()
     * will just return whichever one probing happens to hit first).
     * @param key the key to insert.
     * @param value the value to insert.
     */
    void insert(K key, V &&value) {
        // Grow first if we're empty or about to cross a 7/8 load factor — insert_impl() has no
        // growth logic of its own, this is the only gate keeping probes from running long.
        if (m_capacity == 0 || m_size >= (m_capacity * 7) / 8) {
            rehash();
        }

        insert_impl(std::move(key), std::move(value));
    }

    /**
     * @brief Looks up a value by heterogeneous key args — probes groups via SIMD fingerprint
     * matching (match_byte()), stops at the first group with an empty slot (standard open
     * addressing early-exit).
     * @warning `Args` gets forwarded straight into both `Hash{}(args...)` and
     * `KeyEqual{}(slot(s).key(), args...)` — whatever you pass here has to produce the exact same
     * hash as whatever `K` value was actually inserted, or you're silently missing entries that
     * are sitting right there in the table. No cap, heterogeneous lookup is powerful but easy to
     * get subtly wrong.
     * @tparam Args the heterogeneous lookup key argument pack.
     * @param args the key (or key-equivalent args) to look up.
     * @return the matching value if found, nullopt otherwise.
     */
    template <typename... Args>
    // NOLINTNEXTLINE(cppcoreguidelines-missing-std-forward) — args is read multiple times (once per probed slot); forwarding would use-after-move on the second probe
    [[nodiscard]] std::optional<V> find(Args &&...args) const {
        // Nothing's ever been allocated, no point probing.
        if (m_capacity == 0) {
            return std::nullopt;
        }

        // Hash splits into a group index (which 16-slot group to start probing) and a 7-bit
        // fingerprint (the cheap per-slot pre-check before touching real keys).
        // NOTE: args is a forwarding-reference pack, but it's used for read-only comparisons
        // below and may be evaluated multiple times (once per probed slot) — forwarding it
        // (i.e. casting to an rvalue) here would make any later use a use-after-move, so it's
        // passed as an lvalue everywhere in this function instead.
        std::size_t hash_value = Hash{}(args...); // FIXME(clang-tidy): array-to-pointer-decay — flagged only for instantiations where a caller passes a fixed-size array as the key; Hash{}/KeyEqual{} forwarding args generically, not a mechanical fix
        std::size_t grp = h1(hash_value);
        std::uint8_t fp = h2(hash_value);

        for (std::size_t probe = 0; probe < (m_capacity / GROUP_WIDTH); ++probe) {
            std::size_t cur = (grp + probe) % (m_capacity / GROUP_WIDTH);
            std::uint32_t match = match_byte(cur, fp);

            // Check every slot in this group whose fingerprint matched — could still be a false
            // positive on the fingerprint alone, so confirm with the real key comparison.
            while (match != 0) {
                int bit = __builtin_ctz(match);
                std::size_t slot_index = (cur * GROUP_WIDTH) + bit;
                if (KeyEqual{}(slot(slot_index).key(), args...)) { // FIXME(clang-tidy): array-to-pointer-decay — flagged only for instantiations where a caller passes a fixed-size array as the key; Hash{}/KeyEqual{} forwarding args generically, not a mechanical fix
                    return slot(slot_index).value();
                }
                match &= ~(1U << bit);
            }

            // Hit an empty slot in this group — open addressing says the key can't be any further
            // along the probe sequence, so bail early instead of scanning the whole table, no cap.
            if (match_byte(cur, static_cast<std::uint8_t>(ControlByte::EMPTY)) != 0) {
                break;
            }
        }

        return std::nullopt;
    }

    // Upsert — insert if not present, update value if already present.
    // Returns true if inserted, false if updated.
    /**
     * @brief Inserts `key`/`value` if the key's not already present, otherwise overwrites the
     * existing entry's value in place. The dedup-safe sibling of insert().
     * @warning Same heterogeneous-hashing caveat as find() — `Hash{}(key)` has to line up with
     * whatever produced the hash for any already-inserted equivalent key.
     * @tparam KeyArg the forwarding-reference key argument type.
     * @tparam ValueArg the forwarding-reference value argument type.
     * @param key the key to look up or insert.
     * @param value the value to set or insert.
     * @return true if this inserted a brand-new entry, false if it updated an existing one.
     */
    template <typename KeyArg, typename ValueArg>
    std::optional<bool> upsert(KeyArg &&key, ValueArg &&value) {
        // Same grow-if-needed gate as insert().
        if (m_capacity == 0 || m_size >= (m_capacity * 7) / 8) {
            rehash();
        }

        std::size_t hash_value = Hash{}(key);
        std::size_t grp = h1(hash_value);
        std::uint8_t fp = h2(hash_value);

        // Probe exactly like find() — but if a matching key turns up, overwrite its value in
        // place and report an update instead of just handing the value back.
        for (std::size_t probe = 0; probe < (m_capacity / GROUP_WIDTH); ++probe) {
            std::size_t cur = (grp + probe) % (m_capacity / GROUP_WIDTH);
            std::uint32_t match = match_byte(cur, fp);

            while (match != 0) {
                int bit = __builtin_ctz(match);
                std::size_t slot_index = (cur * GROUP_WIDTH) + bit;
                if (KeyEqual{}(slot(slot_index).key(), key)) {
                    slot(slot_index).value() = std::forward<ValueArg>(value);
                    return false;
                }
                match &= ~(1U << bit);
            }

            if (match_byte(cur, static_cast<std::uint8_t>(ControlByte::EMPTY)) != 0) {
                break;
            }
        }

        // No existing entry found within the probe sequence — place a brand-new one.
        insert_impl(std::forward<KeyArg>(key), std::forward<ValueArg>(value));
        return true;
    }

    /**
     * @brief Removes the entry matching `args`, if any — runs the entry's destructor in place and
     * tombstones the slot with `DELETED` (open-addressing tables can't just zero the control
     * byte, that'd break probing for every other key that hashed into the same group).
     * @tparam Args the heterogeneous lookup key argument pack.
     * @param args the key (or key-equivalent args) to erase.
     */
    template <typename... Args>
    // NOLINTNEXTLINE(cppcoreguidelines-missing-std-forward) — args is read multiple times (once per probed slot); forwarding would use-after-move on the second probe
    void erase(Args &&...args) {
        if (m_capacity == 0) {
            return;
        }

        // NOTE: same reasoning as find() — args is read-only here and may be compared against
        // multiple slots, so it's passed as an lvalue rather than forwarded to avoid a
        // use-after-move on repeated evaluation.
        std::size_t hash_value = Hash{}(args...); // FIXME(clang-tidy): array-to-pointer-decay — flagged only for instantiations where a caller passes a fixed-size array as the key; Hash{}/KeyEqual{} forwarding args generically, not a mechanical fix
        std::size_t grp = h1(hash_value);
        std::uint8_t fp = h2(hash_value);

        // Same probe-and-confirm shape as find() — on a hit, run the entry's dtor and tombstone
        // the slot instead of zeroing it, so probing for every OTHER key sharing this group still
        // works right.
        for (std::size_t probe = 0; probe < (m_capacity / GROUP_WIDTH); ++probe) {
            std::size_t cur = (grp + probe) % (m_capacity / GROUP_WIDTH);
            std::uint32_t match = match_byte(cur, fp);

            while (match != 0) {
                int bit = __builtin_ctz(match);
                std::size_t slot_index = (cur * GROUP_WIDTH) + bit;
                if (KeyEqual{}(slot(slot_index).key(), args...)) { // FIXME(clang-tidy): array-to-pointer-decay — flagged only for instantiations where a caller passes a fixed-size array as the key; Hash{}/KeyEqual{} forwarding args generically, not a mechanical fix
                    slot(slot_index).~Entry();
                    m_control[slot_index] = static_cast<std::uint8_t>(ControlByte::DELETED);  // FIXME(clang-tidy): unchecked operator[], consider .at()
                    m_size--;
                    return;
                }
                match &= ~(1U << bit);
            }

            if (match_byte(cur, static_cast<std::uint8_t>(ControlByte::EMPTY)) != 0) {
                break;
            }
        }
    }

    /**
     * @brief Wipes every entry via destroy_all(), then resets every control byte back to
     * `EMPTY` — the table keeps its allocated capacity, just goes back to logically empty.
     */
    void clear() {
        // Run every live entry's dtor first, then reset the control bytes back to all-empty —
        // capacity stays allocated, the table just goes logically empty.
        destroy_all();
        if (m_capacity > 0) {
            m_control.assign(m_capacity + GROUP_WIDTH, static_cast<std::uint8_t>(ControlByte::EMPTY));
        }
        m_size = 0;
    }

    /**
     * @brief Grabs the live entry count.
     * @return how many entries are currently stored.
     */
    [[nodiscard]] std::size_t size() const { return m_size; }
    /**
     * @brief Checks whether the table has any live entries.
     * @return true if size() is zero.
     */
    [[nodiscard]] bool empty() const { return m_size == 0; }

  private:
    std::vector<std::uint8_t> m_control;
    std::vector<RawSlot> m_raw_slots;
    std::size_t m_capacity = 0;
    std::size_t m_size = 0;

    template <bool>
    friend class IteratorBase;

    /**
     * @brief Reinterprets raw slot storage at index `i` as a live `Entry`.
     * @warning No liveness check — call this on a slot whose control byte says empty/deleted/
     * sentinel and you get a reference to an Entry that was never constructed there. `std::launder`
     * makes the reinterpret legal from the object-model's perspective, it does NOT mean there's
     * actually a live object underneath. Caller's on the hook for only calling this on slots the
     * control bytes say are live.
     * @param index the slot index to reinterpret.
     * @return a mutable reference to the entry stored at slot `index`.
     */
    Entry &slot(std::size_t index) { return *std::launder(reinterpret_cast<Entry *>(&m_raw_slots[index])); }  // FIXME(clang-tidy): unchecked operator[], consider .at(); reinterpret_cast usage
    /**
     * @brief Const overload of slot(), same liveness-assumption warning applies.
     * @param index the slot index to reinterpret.
     * @return a read-only reference to the entry stored at slot `index`.
     */
    [[nodiscard]] const Entry &slot(std::size_t index) const { return *std::launder(reinterpret_cast<const Entry *>(&m_raw_slots[index])); }  // FIXME(clang-tidy): unchecked operator[], consider .at(); reinterpret_cast usage

    /**
     * @brief Walks every slot and runs the destructor on whichever ones are actually live
     * (control byte isn't empty/deleted/sentinel). Called from ~SwissHashMap(), clear(), and
     * rehash() — anywhere the whole table's about to get torn down or replaced.
     * @note `std::vector<RawSlot>`'s own destructor only frees raw bytes — it has no idea `Entry`
     * objects are living inside them, so skipping this step would leak every live `K`/`V` pair
     * (file handles, heap allocations, whatever they own) without ever running their dtors.
     */
    void destroy_all() {
        // Nothing allocated, nothing to destroy.
        if (m_capacity == 0) {
            return;
        }
        // Walk every slot and run the dtor on whichever ones are actually live — the backing
        // vector's own destructor only frees raw bytes, it has no idea a K/V pair lives there.
        for (std::size_t i = 0; i < m_capacity; ++i) {
            if (m_control[i] != static_cast<std::uint8_t>(ControlByte::EMPTY) &&
                m_control[i] != static_cast<std::uint8_t>(ControlByte::DELETED) &&
                m_control[i] != static_cast<std::uint8_t>(ControlByte::SENTINEL)) {  // FIXME(clang-tidy): unchecked operator[], consider .at()
                slot(i).~Entry();
            }
        }
    }

    /**
     * @brief First-stage hash — picks which group of `GROUP_WIDTH` slots a key probes into.
     * @param hash the full hash value to derive the group index from.
     * @return the starting group index for probing.
     */
    [[nodiscard]] std::size_t h1(std::size_t hash) const { return (hash >> 7) % (m_capacity / GROUP_WIDTH); }

    /**
     * @brief Second-stage hash — a 7-bit fingerprint stored per-slot in the control bytes, used
     * to cheaply reject non-matches via SIMD before ever touching the actual key. Dodges
     * `DELETED`/`EMPTY`'s reserved bit patterns by remapping those two specific fingerprint
     * values so a real fingerprint never collides with a tombstone/empty marker.
     * @param hash the full hash value to derive the fingerprint from.
     * @return a 7-bit fingerprint byte, never equal to `DELETED` or `EMPTY`.
     */
    [[nodiscard]] std::uint8_t h2(std::size_t hash) const {
        // Take the low 7 bits as the fingerprint, then dodge the two reserved control-byte
        // patterns (DELETED, EMPTY) by remapping those specific values — a real fingerprint
        // should never collide with a tombstone/empty marker.
        auto fp = static_cast<std::uint8_t>(hash & 0x7F);
        if (fp == 0x7E) {
            return 0x7D;
        }
        if (fp == 0x7F) {
            return 0x7C;
        }
        return fp;
    }

    /**
     * @brief SIMD group scan — loads 16 control bytes starting at `group_idx * GROUP_WIDTH` and
     * builds a bitmask of which ones equal `target`. This is the whole speed trick behind swiss
     * tables: one SSE2 compare instead of 16 scalar byte comparisons.
     * @warning Reads 16 bytes unconditionally via `_mm_loadu_si128`, no bounds check against
     * `m_control`'s actual size — this only stays safe because `m_control` is always allocated with
     * `m_capacity + GROUP_WIDTH` bytes (see rehash()/clear()), giving every group room to overread
     * into that padding without going past the vector's real allocation. Mess with that
     * `+ GROUP_WIDTH` invariant anywhere and this SIMD load goes straight out of bounds.
     * @param group_idx which group of 16 control bytes to scan.
     * @param target the control byte value to match against.
     * @return a 16-bit mask, bit `i` set if slot `i` in the group matches `target`.
     */
    [[nodiscard]] std::uint32_t match_byte(std::size_t group_idx, std::uint8_t target) const {
        // One SSE2 compare instead of 16 scalar byte comparisons — load 16 control bytes,
        // broadcast the target byte across a second register, compare, and pack the result into
        // a bitmask. This is the whole speed trick behind swiss tables.
        std::size_t off = group_idx * GROUP_WIDTH;
        __m128i grp = _mm_loadu_si128(reinterpret_cast<const __m128i *>(&m_control[off]));  // FIXME(clang-tidy): unchecked operator[], consider .at(); reinterpret_cast usage
        __m128i tgt = _mm_set1_epi8(static_cast<char>(target));
        __m128i cmp = _mm_cmpeq_epi8(grp, tgt);
        return static_cast<std::uint32_t>(_mm_movemask_epi8(cmp));
    }

    /**
     * @brief SIMD group scan for insertion targets — same trick as match_byte(), but matches
     * *either* `EMPTY` or `DELETED` in one pass so insert_impl() can reuse tombstoned slots
     * instead of only ever landing on untouched ones.
     * @warning Same overread footgun as match_byte() — depends on the `m_capacity + GROUP_WIDTH`
     * padding invariant holding.
     * @param group_idx which group of 16 control bytes to scan.
     * @return a 16-bit mask, bit `i` set if slot `i` in the group is empty or deleted.
     */
    [[nodiscard]] std::uint32_t match_empty_or_deleted(std::size_t group_idx) const {
        // Same SIMD-load trick as match_byte(), but run the compare twice (once against EMPTY,
        // once against DELETED) and OR the two masks together, so insert_impl() can reuse
        // tombstoned slots instead of only ever landing on untouched ones.
        std::size_t off = group_idx * GROUP_WIDTH;
        __m128i grp = _mm_loadu_si128(reinterpret_cast<const __m128i *>(&m_control[off]));  // FIXME(clang-tidy): unchecked operator[], consider .at(); reinterpret_cast usage
        __m128i empty_mask = _mm_cmpeq_epi8(grp, _mm_set1_epi8(static_cast<char>(std::to_underlying(ControlByte::EMPTY))));
        __m128i del_mask = _mm_cmpeq_epi8(grp, _mm_set1_epi8(static_cast<char>(std::to_underlying(ControlByte::DELETED))));
        return static_cast<std::uint32_t>(_mm_movemask_epi8(_mm_or_si128(empty_mask, del_mask)));
    }

    /**
     * @brief Doubles capacity (or starts at `16 * GROUP_WIDTH`) and reinserts every live entry
     * into the fresh table — this also has the side effect of clearing out all tombstones, since
     * the new control array starts all-`EMPTY`.
     * @warning Every `iterator`/`const_iterator` and any raw slot index taken before calling this
     * is dead the instant it returns — the whole backing storage gets replaced. Hold onto an
     * iterator across an insert() that triggers a rehash and that's a straight use-after-free
     * waiting to happen, classic iterator-invalidation footgun, don't get cooked by it.
     */
    void rehash() {
        // Pull the old storage out from under the live members before replacing them, bet —
        // old_ctrl/old_slots_raw keep the existing entries alive long enough to reinsert below.
        std::size_t old_capacity = m_capacity;
        std::vector<std::uint8_t> old_ctrl = std::move(m_control);
        std::vector<RawSlot> old_slots_raw = std::move(m_raw_slots);

        // Double the capacity (or start at 16 groups from a fresh table), fresh control bytes
        // all-empty — this incidentally clears out every tombstone too.
        m_capacity = (old_capacity == 0) ? 16 * GROUP_WIDTH : old_capacity * 2;

        m_control.assign(m_capacity + GROUP_WIDTH, static_cast<std::uint8_t>(ControlByte::EMPTY));
        m_raw_slots.resize(m_capacity); // raw bytes only — no Entry construction
        m_size = 0;

        // Walk the old table and move every live entry into the new one, destroying the old copy
        // in place once it's moved — vector<RawSlot>'s own dtor only frees raw bytes, it has no
        // idea an Entry was living there.
        for (std::size_t i = 0; i < old_capacity; ++i) {
            if (old_ctrl[i] != static_cast<std::uint8_t>(ControlByte::EMPTY) &&
                old_ctrl[i] != static_cast<std::uint8_t>(ControlByte::DELETED) &&
                old_ctrl[i] != static_cast<std::uint8_t>(ControlByte::SENTINEL)) {  // FIXME(clang-tidy): unchecked operator[], consider .at()
                Entry &entry = *std::launder(reinterpret_cast<Entry *>(&old_slots_raw[i]));  // FIXME(clang-tidy): unchecked operator[], consider .at(); reinterpret_cast usage
                insert_impl(std::move(entry.key()), std::move(entry.value()));
                entry.~Entry(); // explicit dtor — vector<RawSlot> dtor only frees bytes
            }
        }
    }

    /**
     * @brief Placement-constructs a new entry into the first empty-or-deleted slot found while
     * probing from `key`'s home group forward.
     * @warning Assumes rehash() already ran if the load factor demanded it — this has no growth
     * logic of its own, it just throws `std::runtime_error` if it probes the *entire* table
     * without finding room. Callers (insert()/upsert()) are the ones responsible for keeping the
     * load factor sane before reaching here.
     * @tparam KeyArg the forwarding-reference key argument type.
     * @tparam ValueArg the forwarding-reference value argument type.
     * @param key the key to construct in place.
     * @param value the value to construct in place.
     * @throws std::runtime_error if every slot in the table is occupied and probing wraps without
     * finding an empty or tombstoned slot — a hard L that should be unreachable given insert()/
     * upsert() always rehash() before this gets called.
     */
    template <typename KeyArg, typename ValueArg>
    void insert_impl(KeyArg &&key, ValueArg &&value) {
        std::size_t hash_value = Hash{}(key);
        std::size_t grp = h1(hash_value);
        std::uint8_t fp = h2(hash_value);

        // Probe forward from the home group until a group with an empty-or-deleted slot turns
        // up, then claim the first such slot.
        for (std::size_t probe = 0; probe < (m_capacity / GROUP_WIDTH); ++probe) {
            std::size_t cur = (grp + probe) % (m_capacity / GROUP_WIDTH);
            std::uint32_t avail = match_empty_or_deleted(cur);

            if (avail != 0) {
                int bit = __builtin_ctz(avail);
                std::size_t slot_index = (cur * GROUP_WIDTH) + bit;

                m_control[slot_index] = fp;  // FIXME(clang-tidy): unchecked operator[], consider .at()
                // Construct DIRECTLY in the slot without intermediate temporaries
                new (&m_raw_slots[slot_index]) Entry(std::forward<KeyArg>(key), std::forward<ValueArg>(value));  // FIXME(clang-tidy): unchecked operator[], consider .at()
                m_size++;
                return;
            }
        }
        // Should be unreachable — insert()/upsert() always rehash() before this is called, so the
        // table should never actually be full here.
        throw std::runtime_error("Swiss table exhausted");
    }
};

} // namespace hashmap::swiss

#ifdef CONGELADO_TEST
namespace hashmap::swiss::tests {
using namespace boost::ut;

suite<"SwissHashMap"> swiss_hash_map_suite = [] {
    "starts empty"_test = [] {
        SwissHashMap<std::string, int> map;
        expect(map.empty());
        expect(map.size() == 0);
        expect(not map.find("missing").has_value());
    };
    "insert then find round-trips a value"_test = [] {
        SwissHashMap<std::string, int> map;
        map.insert("a", 1);

        expect(not map.empty());
        expect(map.size() == 1);
        expect(map.find("a") == 1);
    };
    "upsert inserts new keys and updates existing ones"_test = [] {
        SwissHashMap<std::string, int> map;

        auto inserted = map.upsert("a", 1);
        expect(inserted.value());
        expect(map.find("a") == 1);

        auto updated = map.upsert("a", 2);
        expect(not updated.value());
        expect(map.find("a") == 2);
        expect(map.size() == 1);
    };
    "erase removes an entry"_test = [] {
        SwissHashMap<std::string, int> map;
        map.insert("a", 1);
        map.erase("a");

        expect(map.empty());
        expect(not map.find("a").has_value());
    };
    "clear empties the table but keeps it usable"_test = [] {
        SwissHashMap<std::string, int> map;
        map.insert("a", 1);
        map.insert("b", 2);
        map.clear();

        expect(map.empty());
        map.insert("c", 3);
        expect(map.find("c") == 3);
    };
    "grows past the initial capacity via rehash and keeps every entry"_test = [] {
        SwissHashMap<int, int> map;
        for (int i = 0; i < 500; ++i) {
            map.insert(i, i * 2);
        }

        expect(map.size() == 500);
        expect(map.find(0) == 0);
        expect(map.find(250) == 500);
        expect(map.find(499) == 998);
    };
    "iteration visits every live entry exactly once"_test = [] {
        SwissHashMap<int, int> map;
        map.insert(1, 10);
        map.insert(2, 20);
        map.insert(3, 30);

        std::size_t count = 0;
        int sum = 0;
        for (auto &entry : map) {
            ++count;
            sum += entry.value();
        }

        expect(count == 3);
        expect(sum == 60);
    };
};

} // namespace hashmap::swiss::tests
#endif

namespace std {

// FIXME(clang-tidy): bugprone-std-namespace-modification — this reopens `namespace std` to
// explicitly specialize `tuple_size`/`tuple_element` for a program-defined type, which is
// exactly the pattern [namespace.std] carves out as permitted (explicit specialization of a
// standard library template for a user-defined type). There's no safe alternative that still
// enables structured bindings on `hashmap::swiss::Entry`, so this is left as-is.
// NOTE: this tells the compiler the size to use structured bindings
template <typename K, typename V>
struct tuple_size<hashmap::swiss::Entry<K, V>> : std::integral_constant<std::size_t, 2> {};  // NOLINT(bugprone-std-namespace-modification) — required tuple_size specialization for structured bindings, permitted under [namespace.std]

// FIXME(clang-tidy): bugprone-std-namespace-modification — same rationale as tuple_size above.
// NOTE: this tells the compiler the types to use for structured bindings
template <std::size_t I, typename K, typename V>
struct tuple_element<I, hashmap::swiss::Entry<K, V>> {  // NOLINT(bugprone-std-namespace-modification) — required tuple_element specialization for structured bindings, permitted under [namespace.std]
    // Simple swith at compile time via std::conditional_t to return the correct type based on the index I
    using type = std::conditional_t<I == 0, K, V>;
};

} // namespace std
