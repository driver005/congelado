export module shared:table;
import std;
import :types;

namespace codec::shared::table {

struct SvHash {
    using is_transparent = void;
    std::size_t operator()(std::string_view sv) const noexcept {
        std::size_t h = 14695981039346656037ULL;
        for (unsigned char c : sv) {
            h ^= c;
            h *= 1099511628211ULL;
        }
        return h;
    }
};

struct PairHash {
    using is_transparent = void;
    std::size_t operator()(const auto &p) const noexcept {
        SvHash h;
        std::size_t h1 = h(std::string_view{p.first});
        std::size_t h2 = h(std::string_view{p.second});
        return h1 ^ (h2 * 0x9e3779b97f4a7c15ULL + 0x6c62272e07bb0142ULL + (h1 << 6) + (h1 >> 2));
    }
};

struct PairEqual {
    using is_transparent = void;
    bool operator()(const auto &a, const auto &b) const noexcept {
        return std::string_view{a.first} == std::string_view{b.first} &&
               std::string_view{a.second} == std::string_view{b.second};
    }
};

using FullMap = std::unordered_map<std::pair<std::string_view, std::string_view>, std::size_t, PairHash, PairEqual>;
using NameMap = std::unordered_map<std::string_view, std::size_t, SvHash>;

} // namespace codec::shared::table

export namespace codec::shared::table {

template <const auto &Table>
class StaticTable {
  public:
    static constexpr std::size_t STATIC_SIZE = std::size(Table);

    static SearchResult find(std::string_view name, std::string_view value) noexcept {
        auto it = FULL_MAP.find({name, value});
        if (it != FULL_MAP.end())
            return SearchResult{it->second, true, true};
        return SearchResult::none();
    }

    static SearchResult find_name(std::string_view name) noexcept {
        auto it = NAME_MAP.find(name);
        if (it != NAME_MAP.end())
            return SearchResult{it->second, true, false};
        return SearchResult::none();
    }

    static std::optional<std::pair<std::string_view, std::string_view>> at(std::size_t idx) noexcept {
        if (idx >= STATIC_SIZE)
            return std::nullopt;
        return Table[idx];
    }

    static constexpr std::size_t size() noexcept { return STATIC_SIZE; }

  private:
    // Runtime-initialized static maps for O(1) lookups
    static inline const FullMap FULL_MAP = [] {
        FullMap m;
        m.reserve(STATIC_SIZE);
        for (std::size_t i = 0; i < STATIC_SIZE; ++i) {
            // HPACK/QPACK: Earlier entries have priority in the static table
            m.try_emplace({Table[i].first, Table[i].second}, i);
        }
        return m;
    }();

    static inline const NameMap NAME_MAP = [] {
        NameMap m;
        m.reserve(STATIC_SIZE);
        for (std::size_t i = 0; i < STATIC_SIZE; ++i) {
            // try_emplace ensures we keep the first occurrence (lowest index)
            m.try_emplace(Table[i].first, i);
        }
        return m;
    }();
};

class DynamicTable {
  public:
    static constexpr std::size_t ENTRY_OVERHEAD = 32;

    explicit DynamicTable(std::size_t max_size = 4096) : m_max_size{max_size} {
        m_full.reserve(128);
        m_name.reserve(128);
    }

    std::uint64_t insert(std::string name, std::string value) {
        const std::size_t esz = name.size() + value.size() + ENTRY_OVERHEAD;
        if (esz > m_max_size) {
            evict_all();
            return 0;
        }

        while (!m_deque.empty() && m_current_size + esz > m_max_size) {
            evict_oldest();
        }

        ++m_generation;
        m_current_size += esz;

        m_deque.emplace_front(std::move(name), std::move(value));
        const auto &front = m_deque.front();

        m_full.insert_or_assign({front.name(), front.value()}, m_generation);
        m_name.emplace(front.name(), m_generation);

        return m_generation;
    }

    SearchResult search(std::string_view name, std::string_view value) const noexcept {
        if (auto it = m_full.find(std::pair<std::string_view, std::string_view>{name, value}); it != m_full.end())
            return SearchResult(static_cast<std::size_t>(it->second), false, true);

        auto [lo, hi] = m_name.equal_range(name);
        std::uint64_t best = 0;
        for (auto it = lo; it != hi; ++it) {
            if (it->second > best)
                best = it->second;
        }

        if (best)
            return SearchResult(static_cast<std::size_t>(best), false, false);
        return SearchResult::none();
    }

    const HeaderField *at_pos(std::size_t pos) const noexcept {
        return (pos < m_deque.size()) ? &m_deque[pos] : nullptr;
    }
    const HeaderField *at_gen(std::uint64_t gen) const noexcept {
        const std::size_t pos = gen_to_pos(gen);
        return (pos != SearchResult::NPOS) ? &m_deque[pos] : nullptr;
    }

    std::size_t gen_to_pos(std::uint64_t gen) const noexcept {
        if (gen == 0 || gen > m_generation || m_deque.empty())
            return SearchResult::NPOS;
        const std::uint64_t oldest = m_generation - (m_deque.size() - 1);
        if (gen < oldest)
            return SearchResult::NPOS;
        return static_cast<std::size_t>(m_generation - gen);
    }

    void set_max_size(std::size_t new_max) {
        m_max_size = new_max;
        while (!m_deque.empty() && m_current_size > m_max_size)
            evict_oldest();
    }

    std::size_t size() const noexcept { return m_deque.size(); }
    std::size_t current_size() const noexcept { return m_current_size; }
    std::uint64_t insert_count() const noexcept { return m_generation; }
    std::size_t max_size() const noexcept { return m_max_size; }

  private:
    void evict_oldest() {
        if (m_deque.empty())
            return;
        const auto &e = m_deque.back();
        const std::uint64_t tail_gen = m_generation - (m_deque.size() - 1);

        // Manual cleanup since we can't store iterators in the shared HeaderField
        if (auto it = m_full.find({e.name(), e.value()}); it != m_full.end() && it->second == tail_gen) {
            m_full.erase(it);
        }

        auto [lo, hi] = m_name.equal_range(e.name());
        for (auto it = lo; it != hi; ++it) {
            if (it->second == tail_gen) {
                m_name.erase(it);
                break;
            }
        }

        m_current_size -= e.size();
        m_deque.pop_back();
    }

    void evict_all() {
        m_full.clear();
        m_name.clear();
        m_deque.clear();
        m_current_size = 0;
    }

    std::size_t m_max_size;
    std::size_t m_current_size{0};
    std::uint64_t m_generation{0};
    std::deque<HeaderField> m_deque;
    FullMap m_full;
    NameMap m_name;
};

} // namespace codec::shared::table
