module;

#include <immintrin.h>

export module hashmap:swiss;

import std;

constexpr std::size_t kGroupWidth = 16;

export namespace hashmap::swiss {

template <typename K, typename V>
struct Entry {
    Entry(K k, V v) : m_key(std::move(k)), m_value(std::move(v)) {}

    K &key() { return m_key; }
    const K &key() const { return m_key; }
    V &value() { return m_value; }
    const V &value() const { return m_value; }

    // NOTE: used by compiler-generated get<I> for structured bindings, not intended for direct use.
    template <std::size_t I>
    auto &get() {
        if constexpr (I == 0)
            return m_key;
        else
            return m_value;
    }

    // NOTE: used by compiler-generated get<I> for structured bindings, not intended for direct use.
    template <std::size_t I>
    const auto &get() const {
        if constexpr (I == 0)
            return m_key;
        else
            return m_value;
    }

  private:
    K m_key;
    V m_value;
};

// NOTE: used by compiler-generated get<I> for structured bindings, not intended for direct use.
template <std::size_t I, typename K, typename V>
auto &get(Entry<K, V> &e) {
    return e.template get<I>();
}

// NOTE: used by compiler-generated get<I> for structured bindings, not intended for direct use.
template <std::size_t I, typename K, typename V>
const auto &get(const Entry<K, V> &e) {
    return e.template get<I>();
}

template <typename K, typename V, typename Hash = std::hash<K>, typename KeyEqual = std::equal_to<K>,
          typename ValueEqual = std::equal_to<V>>
class SwissHashMap {
  public:
    using Entry = Entry<K, V>;

    // Raw uninitialised storage for one Entry — no K or V construction until
    // placement new fires in insert_impl. alignas ensures placement new is valid.
    struct alignas(alignof(Entry)) RawSlot {
        std::byte data[sizeof(Entry)];
    };

    enum ControlByte : std::uint8_t {
        kEmpty = 0xFF,
        kDeleted = 0x7E,
        kSentinel = 0xFE,
    };

    template <bool IsConst>
    class IteratorBase {
        using MapPtr = std::conditional_t<IsConst, const SwissHashMap *, SwissHashMap *>;
        using EntryRef = std::conditional_t<IsConst, const Entry &, Entry &>;
        using EntryPtr = std::conditional_t<IsConst, const Entry *, Entry *>;

        MapPtr m_map;
        std::size_t m_idx;

        void advance() {
            while (m_idx < m_map->capacity_) {
                std::uint8_t c = m_map->ctrl_[m_idx];
                if (c != SwissHashMap::kEmpty && c != SwissHashMap::kDeleted && c != SwissHashMap::kSentinel)
                    break;
                ++m_idx;
            }
        }

      public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = Entry;
        using difference_type = std::ptrdiff_t;
        using reference = EntryRef;
        using pointer = EntryPtr;

        IteratorBase(MapPtr map, std::size_t idx) : m_map(map), m_idx(idx) { advance(); }

        reference operator*() const { return m_map->slot(m_idx); }
        pointer operator->() const { return &m_map->slot(m_idx); }

        IteratorBase &operator++() {
            ++m_idx;
            advance();
            return *this;
        }
        IteratorBase operator++(int) {
            auto tmp = *this;
            ++(*this);
            return tmp;
        }

        bool operator==(const IteratorBase &o) const { return m_idx == o.m_idx; }
        bool operator!=(const IteratorBase &o) const { return m_idx != o.m_idx; }
    };

    using iterator = IteratorBase<false>;
    using const_iterator = IteratorBase<true>;

    iterator begin() { return {this, 0}; }
    iterator end() { return {this, capacity_}; }
    const_iterator begin() const { return {this, 0}; }
    const_iterator end() const { return {this, capacity_}; }
    const_iterator cbegin() const { return begin(); }
    const_iterator cend() const { return end(); }

    SwissHashMap() : capacity_(0), size_(0) {}

    ~SwissHashMap() { destroy_all(); }

    SwissHashMap(const SwissHashMap &) = delete;
    SwissHashMap &operator=(const SwissHashMap &) = delete;

    SwissHashMap(SwissHashMap &&other) noexcept
        : ctrl_(std::move(other.ctrl_)), slots_raw_(std::move(other.slots_raw_)), capacity_(other.capacity_),
          size_(other.size_) {
        other.capacity_ = 0;
        other.size_ = 0;
    }

    SwissHashMap &operator=(SwissHashMap &&other) noexcept {
        if (this != &other) {
            destroy_all();
            ctrl_ = std::move(other.ctrl_);
            slots_raw_ = std::move(other.slots_raw_);
            capacity_ = other.capacity_;
            size_ = other.size_;
            other.capacity_ = 0;
            other.size_ = 0;
        }
        return *this;
    }

    void insert(K key, V value) {
        if (capacity_ == 0 || size_ >= (capacity_ * 7) / 8)
            rehash();

        insert_impl(std::move(key), std::move(value));
    }

    template <typename... Args>
    [[nodiscard]] std::optional<V> find(Args &&...args) const {
        if (capacity_ == 0)
            return std::nullopt;

        std::size_t h = Hash{}(args...);
        std::size_t grp = h1(h);
        std::uint8_t fp = h2(h);

        for (std::size_t probe = 0; probe < (capacity_ / kGroupWidth); ++probe) {
            std::size_t cur = (grp + probe) % (capacity_ / kGroupWidth);
            std::uint32_t match = match_byte(cur, fp);

            while (match != 0) {
                int bit = __builtin_ctz(match);
                std::size_t s = cur * kGroupWidth + bit;
                if (KeyEqual{}(slot(s).key(), args...))
                    return slot(s).value();
                match &= ~(1u << bit);
            }

            if (match_byte(cur, kEmpty) != 0)
                break;
        }

        return std::nullopt;
    }

    // Upsert — insert if not present, update value if already present.
    // Returns true if inserted, false if updated.
    template <typename KeyArg, typename ValueArg>
    std::optional<bool> upsert(KeyArg &&key, ValueArg &&value) {
        if (capacity_ == 0 || size_ >= (capacity_ * 7) / 8)
            rehash();

        std::size_t h = Hash{}(key);
        std::size_t grp = h1(h);
        std::uint8_t fp = h2(h);

        for (std::size_t probe = 0; probe < (capacity_ / kGroupWidth); ++probe) {
            std::size_t cur = (grp + probe) % (capacity_ / kGroupWidth);
            std::uint32_t match = match_byte(cur, fp);

            while (match != 0) {
                int bit = __builtin_ctz(match);
                std::size_t s = cur * kGroupWidth + bit;
                if (KeyEqual{}(slot(s).key(), key)) {
                    slot(s).value() = std::forward<ValueArg>(value);
                    return false;
                }
                match &= ~(1u << bit);
            }

            if (match_byte(cur, kEmpty) != 0)
                break;
        }

        insert_impl(std::forward<KeyArg>(key), std::forward<ValueArg>(value));
        return true;
    }

    template <typename... Args>
    void erase(Args &&...args) {
        if (capacity_ == 0)
            return;

        std::size_t h = Hash{}(args...);
        std::size_t grp = h1(h);
        std::uint8_t fp = h2(h);

        for (std::size_t probe = 0; probe < (capacity_ / kGroupWidth); ++probe) {
            std::size_t cur = (grp + probe) % (capacity_ / kGroupWidth);
            std::uint32_t match = match_byte(cur, fp);

            while (match != 0) {
                int bit = __builtin_ctz(match);
                std::size_t s = cur * kGroupWidth + bit;
                if (KeyEqual{}(slot(s).key(), args...)) {
                    slot(s).~Entry();
                    ctrl_[s] = kDeleted;
                    size_--;
                    return;
                }
                match &= ~(1u << bit);
            }

            if (match_byte(cur, kEmpty) != 0)
                break;
        }
    }

    void clear() {
        destroy_all();
        if (capacity_ > 0)
            ctrl_.assign(capacity_ + kGroupWidth, static_cast<std::uint8_t>(kEmpty));
        size_ = 0;
    }

    [[nodiscard]] std::size_t size() const { return size_; }
    [[nodiscard]] bool empty() const { return size_ == 0; }

  private:
    std::vector<std::uint8_t> ctrl_;
    std::vector<RawSlot> slots_raw_;
    std::size_t capacity_;
    std::size_t size_;

    template <bool>
    friend class IteratorBase;

    Entry &slot(std::size_t i) { return *std::launder(reinterpret_cast<Entry *>(&slots_raw_[i])); }
    const Entry &slot(std::size_t i) const { return *std::launder(reinterpret_cast<const Entry *>(&slots_raw_[i])); }

    void destroy_all() {
        if (capacity_ == 0)
            return;
        for (std::size_t i = 0; i < capacity_; ++i) {
            if (ctrl_[i] != kEmpty && ctrl_[i] != kDeleted && ctrl_[i] != kSentinel)
                slot(i).~Entry();
        }
    }

    [[nodiscard]] std::size_t h1(std::size_t hash) const { return (hash >> 7) % (capacity_ / kGroupWidth); }

    [[nodiscard]] std::uint8_t h2(std::size_t hash) const {
        std::uint8_t fp = static_cast<std::uint8_t>(hash & 0x7F);
        if (fp == 0x7E)
            return 0x7D;
        if (fp == 0x7F)
            return 0x7C;
        return fp;
    }

    [[nodiscard]] std::uint32_t match_byte(std::size_t group_idx, std::uint8_t target) const {
        std::size_t off = group_idx * kGroupWidth;
        __m128i grp = _mm_loadu_si128(reinterpret_cast<const __m128i *>(&ctrl_[off]));
        __m128i tgt = _mm_set1_epi8(static_cast<char>(target));
        __m128i cmp = _mm_cmpeq_epi8(grp, tgt);
        return static_cast<std::uint32_t>(_mm_movemask_epi8(cmp));
    }

    [[nodiscard]] std::uint32_t match_empty_or_deleted(std::size_t group_idx) const {
        std::size_t off = group_idx * kGroupWidth;
        __m128i grp = _mm_loadu_si128(reinterpret_cast<const __m128i *>(&ctrl_[off]));
        __m128i empty_mask = _mm_cmpeq_epi8(grp, _mm_set1_epi8(static_cast<char>(kEmpty)));
        __m128i del_mask = _mm_cmpeq_epi8(grp, _mm_set1_epi8(static_cast<char>(kDeleted)));
        return static_cast<std::uint32_t>(_mm_movemask_epi8(_mm_or_si128(empty_mask, del_mask)));
    }

    void rehash() {
        std::size_t old_capacity = capacity_;
        std::vector<std::uint8_t> old_ctrl = std::move(ctrl_);
        std::vector<RawSlot> old_slots_raw = std::move(slots_raw_);

        capacity_ = (old_capacity == 0) ? 16 * kGroupWidth : old_capacity * 2;

        ctrl_.assign(capacity_ + kGroupWidth, static_cast<std::uint8_t>(kEmpty));
        slots_raw_.resize(capacity_); // raw bytes only — no Entry construction
        size_ = 0;

        for (std::size_t i = 0; i < old_capacity; ++i) {
            if (old_ctrl[i] != kEmpty && old_ctrl[i] != kDeleted && old_ctrl[i] != kSentinel) {
                Entry &e = *std::launder(reinterpret_cast<Entry *>(&old_slots_raw[i]));
                insert_impl(std::move(e.key()), std::move(e.value()));
                e.~Entry(); // explicit dtor — vector<RawSlot> dtor only frees bytes
            }
        }
    }

    template <typename KeyArg, typename ValueArg>
    void insert_impl(KeyArg &&key, ValueArg &&value) {
        std::size_t h = Hash{}(key);
        std::size_t grp = h1(h);
        std::uint8_t fp = h2(h);

        for (std::size_t probe = 0; probe < (capacity_ / kGroupWidth); ++probe) {
            std::size_t cur = (grp + probe) % (capacity_ / kGroupWidth);
            std::uint32_t avail = match_empty_or_deleted(cur);

            if (avail != 0) {
                int bit = __builtin_ctz(avail);
                std::size_t s = cur * kGroupWidth + bit;

                ctrl_[s] = fp;
                // Construct DIRECTLY in the slot without intermediate temporaries
                new (&slots_raw_[s]) Entry(std::forward<KeyArg>(key), std::forward<ValueArg>(value));
                size_++;
                return;
            }
        }
        throw std::runtime_error("Swiss table exhausted");
    }
};

} // namespace hashmap::swiss

namespace std {

// NOTE: this tells the compiler the size to use structured bindings
template <typename K, typename V>
struct tuple_size<hashmap::swiss::Entry<K, V>> : std::integral_constant<std::size_t, 2> {};

// NOTE: this tells the compiler the types to use for structured bindings
template <std::size_t I, typename K, typename V>
struct tuple_element<I, hashmap::swiss::Entry<K, V>> {
    // Simple swith at compile time via std::conditional_t to return the correct type based on the index I
    using type = std::conditional_t<I == 0, K, V>;
};

} // namespace std
