export module io_codec_shared:table;

import std;
import hashmap;
import io_shared;
import :types;
import :consts;

namespace io::shared_codec::table {

template <typename... Ts>
struct overloaded : Ts... {
    using Ts::operator()...;
};

enum class HeaderKeyType : bool { NameOnly = 0, FullMatch = 1 };

class HeaderKey {
  public:
    HeaderKey(std::string_view name, std::string_view value = {}, HeaderKeyType type = HeaderKeyType::NameOnly)
        : m_name(name), m_value(value), m_type(type) {
        if (std::holds_alternative<std::string_view>(m_name) && std::get<std::string_view>(m_name).empty())
            throw std::runtime_error("Header name cannot be empty");
    }
    HeaderKey(shared::http::Token token, std::string_view value = {}, HeaderKeyType type = HeaderKeyType::NameOnly)
        : m_name(token), m_value(value), m_type(type) {}

    bool operator==(const HeaderKey &other) const noexcept {
        return m_name == other.m_name && m_type == other.m_type &&
               (m_type == HeaderKeyType::NameOnly || m_value == other.m_value);
    }

    bool is_equal(std::string_view name, std::string_view value, HeaderKeyType type) const noexcept {
        if (m_type != type)
            return false;

        bool name_match = std::visit(
            [&](auto &&arg) -> bool {
                using T = std::decay_t<decltype(arg)>;

                if constexpr (std::is_same_v<T, shared::http::Token>) {
                    return shared::http::token_to_string(arg) == name;
                } else if constexpr (std::is_same_v<T, std::string_view>) {
                    return arg == name;
                }
            },
            m_name);

        if (!name_match)
            return false;
        return (m_type == HeaderKeyType::NameOnly) || (m_value == value);
    }

    bool is_equal(shared::http::Token token, std::string_view value, HeaderKeyType type) const noexcept {
        if (m_type != type)
            return false;

        bool name_match = std::visit(
            [&](auto &&arg) -> bool {
                using T = std::decay_t<decltype(arg)>;

                if constexpr (std::is_same_v<T, shared::http::Token>) {
                    return arg == token;
                } else if constexpr (std::is_same_v<T, std::string_view>) {
                    return arg == shared::http::token_to_string(token);
                }
            },
            m_name);

        if (!name_match)
            return false;

        return (m_type == HeaderKeyType::NameOnly) || (m_value == value);
    }

    constexpr std::variant<shared::http::Token, std::string_view> get_name() const noexcept { return m_name; }
    constexpr std::string_view get_value() const noexcept { return m_value; }
    constexpr HeaderKeyType get_type() const noexcept { return m_type; }

  private:
    std::variant<shared::http::Token, std::string_view> m_name;
    std::string_view m_value;
    HeaderKeyType m_type;
};

struct HeaderEqual {
    using is_transparent = void;

    bool operator()(const HeaderKey &lhs, const HeaderKey &rhs) const noexcept { return lhs == rhs; }

    bool operator()(const HeaderKey &k, std::string_view n, std::string_view v, HeaderKeyType t) const noexcept {
        return k.is_equal(n, v, t);
    }

    bool operator()(const HeaderKey &k, shared::http::Token token, std::string_view v, HeaderKeyType t) const noexcept {
        return k.is_equal(token, v, t);
    }
};

struct HeaderHasher {
    using is_transparent = void;

    std::size_t operator()(const HeaderKey &k) const noexcept {
        return std::visit([&](auto &&arg) -> std::size_t { return hash_impl(arg, k.get_value(), k.get_type()); },
                          k.get_name());
    }

    std::size_t operator()(std::string_view name, std::string_view value, HeaderKeyType type) const noexcept {
        return hash_impl(name, value, type);
    }

    std::size_t operator()(shared::http::Token token, std::string_view value, HeaderKeyType type) const noexcept {
        return hash_impl(token, value, type);
    }

  private:
    // Helper: Combines bits using the Golden Ratio to prevent collisions
    static void hash_combine(std::size_t &seed, std::size_t v) noexcept {
        seed ^= v + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }

    std::size_t hash_impl(std::string_view name, std::string_view value, HeaderKeyType type) const noexcept {
        std::size_t h = std::hash<std::string_view>{}(name);
        if (type == HeaderKeyType::FullMatch)
            hash_combine(h, std::hash<std::string_view>{}(value));
        hash_combine(h, static_cast<std::size_t>(type));
        return h;
    }

    std::size_t hash_impl(shared::http::Token token, std::string_view value, HeaderKeyType type) const noexcept {
        std::size_t h = std::hash<std::uint32_t>{}(std::to_underlying(token));
        if (type == HeaderKeyType::FullMatch)
            hash_combine(h, std::hash<std::string_view>{}(value));
        hash_combine(h, static_cast<std::size_t>(type));
        return h;
    }
};


template <typename T>
concept StaticHeaderTable = requires(T table) {
    { std::size(table) } -> std::convertible_to<std::size_t>;
    requires std::same_as<std::decay_t<decltype(table[0])>, std::shared_ptr<shared::http::HeaderField<true>>>;
};

using QpackMap = hashmap::swiss::SwissHashMap<HeaderKey, std::size_t, HeaderHasher, HeaderEqual>;

} // namespace io::shared_codec::table

export namespace io::shared_codec::table {

template <const auto &Table>
    requires StaticHeaderTable<decltype(Table)>
class StaticTable {
  public:
    static constexpr std::size_t STATIC_SIZE = std::size(Table);

    static std::optional<std::shared_ptr<shared::http::HeaderField<true>>> at(std::size_t idx) noexcept {
        if (idx >= STATIC_SIZE) {
            return std::nullopt;
        }
        // Returns the shared_ptr from the static array
        return Table[idx];
    }

    template <IndexCalculation Calc = IndexCalculation::QPack>
    static SearchResult search(std::string_view name, std::string_view value) noexcept {
        if (auto result = search_full_match<Calc>(name, value); result.found()) {
            return result;
        }

        if (auto result = search_name_only<Calc>(name); result.found()) {
            return result;
        }

        return SearchResult::none();
    }

    template <IndexCalculation Calc = IndexCalculation::QPack>
    static SearchResult search_full_match(std::string_view name, std::string_view value) noexcept {
        if (auto positon = MAP.find(name, value, HeaderKeyType::FullMatch)) {
            return SearchResult{*positon + (Calc == IndexCalculation::HPack), true, true};
        }

        return SearchResult::none();
    }

    template <IndexCalculation Calc = IndexCalculation::QPack>
    static SearchResult search_name_only(std::string_view name) noexcept {
        if (auto positon = MAP.find(name, "", HeaderKeyType::NameOnly)) {
            return SearchResult{*positon + (Calc == IndexCalculation::HPack), true, false};
        }

        return SearchResult::none();
    }

    static constexpr std::size_t size() noexcept { return STATIC_SIZE; }

  private:
    static inline const QpackMap MAP = [] {
        QpackMap m;

        // TODO: We can optimize by adding reserve support to out our map
        // m.reserve(STATIC_SIZE * 2);

        for (std::size_t i = 0; i < STATIC_SIZE; ++i) {
            auto field = Table[i];

            m.upsert(HeaderKey{field->get_name(), field->get_value(), HeaderKeyType::FullMatch}, i);
            m.upsert(HeaderKey{field->get_name(), "", HeaderKeyType::NameOnly}, i);
        }
        return m;
    }();
};

class DynamicTable {
  public:
    explicit DynamicTable(std::size_t max_size = 4096) : m_max_size{max_size}, m_current_size{0}, m_generation{0} {
        // TODO: add reserve support to our map and set an initial capacity based on max_size and average entry size
        // m_map.reserve(128); // Initial capacity to reduce early collisions
    }


    template <IndexCalculation Calc = IndexCalculation::QPack>
    std::size_t insert(std::string_view name, std::string_view value) {
        auto field = std::make_shared<shared::http::HeaderField<false>>(std::move(name), std::move(value));
        return insert<Calc>(std::move(field));
    }

    template <IndexCalculation Calc = IndexCalculation::QPack>
    std::size_t insert(shared::http::Token token, std::string_view value) {
        auto field = std::make_shared<shared::http::HeaderField<true>>(token, std::move(value));
        return insert<Calc>(std::move(field));
    }

    template <IndexCalculation Calc = IndexCalculation::QPack, bool IsStatic>
    std::size_t insert(std::shared_ptr<shared::http::HeaderField<IsStatic>> field) {
        const std::size_t entry_size = field->size();
        if (entry_size > m_max_size) {
            evict_all();
            return 0;
        }

        while (!m_deque.empty() && m_current_size + entry_size > m_max_size) {
            evict_oldest();
        }

        ++m_generation;
        m_current_size += entry_size;

        m_map.upsert(HeaderKey{field->get_name(), field->get_value(), HeaderKeyType::FullMatch}, m_generation);
        m_map.upsert(HeaderKey{field->get_name(), "", HeaderKeyType::NameOnly}, m_generation);

        m_deque.push_front(std::move(field));

        if constexpr (Calc == IndexCalculation::QPack) {
            return m_generation;
        } else {
            return generation_to_position(m_generation);
        }
    }

    template <IndexCalculation Calc = IndexCalculation::QPack>
    SearchResult search(std::string_view name, std::string_view value) const noexcept {
        if (auto result = search_full_match<Calc>(name, value); result.found()) {
            return result;
        }

        if (auto result = search_name_only<Calc>(name); result.found()) {
            return result;
        }

        return SearchResult::none();
    }

    template <IndexCalculation Calc = IndexCalculation::QPack>
    SearchResult search_full_match(std::string_view name, std::string_view value) const noexcept {
        if (auto generation = m_map.find(name, value, HeaderKeyType::FullMatch)) {
            if constexpr (Calc == IndexCalculation::QPack) {
                return SearchResult{*generation, true, true};
            } else {
                return SearchResult{generation_to_position(*generation), true, true};
            }
        }

        return SearchResult::none();
    }

    template <IndexCalculation Calc = IndexCalculation::QPack>
    SearchResult search_name_only(std::string_view name) const noexcept {
        if (auto generation = m_map.find(name, "", HeaderKeyType::NameOnly)) {
            if constexpr (Calc == IndexCalculation::QPack) {
                return SearchResult{*generation, true, false};
            } else {
                return SearchResult{generation_to_position(*generation), true, false};
            }
        }

        return SearchResult::none();
    }

    std::optional<shared::http::HeaderEntry> at_positon(std::size_t pos) const noexcept {
        return (pos < m_deque.size()) ? std::optional{m_deque[pos]} : std::nullopt;
    }

    std::optional<shared::http::HeaderEntry> at_generation(std::size_t gen) const noexcept {
        const std::size_t pos = generation_to_position(gen);
        return (pos != SearchResult::NPOS) ? std::optional{m_deque[pos]} : std::nullopt;
    }

    std::size_t generation_to_position(std::size_t gen) const noexcept {
        if (gen == 0 || gen > m_generation || m_deque.empty())
            return SearchResult::NPOS;
        const std::size_t oldest = m_generation - (m_deque.size() - 1);
        if (gen < oldest)
            return SearchResult::NPOS;
        return m_generation - gen;
    }

    void set_max_size(std::size_t new_max) {
        m_max_size = new_max;
        while (!m_deque.empty() && m_current_size > m_max_size)
            evict_oldest();
    }

    std::size_t get_size() const noexcept { return m_deque.size(); }
    std::size_t get_current_size() const noexcept { return m_current_size; }
    std::size_t get_insert_count() const noexcept { return m_generation; }
    std::size_t get_max_size() const noexcept { return m_max_size; }

  private:
    void evict_oldest() {
        if (m_deque.empty())
            return;
        const auto &field = m_deque.back();
        const std::size_t oldest_gen = m_generation - (m_deque.size() - 1);

        const auto [name, value] = std::visit(
            [](const auto &f) -> std::pair<std::string, std::string> {
                if constexpr (std::is_same_v<std::decay_t<decltype(f)>,
                                             std::shared_ptr<shared::http::HeaderField<true>>>)
                    return {std::string(shared::http::token_to_string(f->get_name())), std::string(f->get_value())};
                else
                    return {std::string(f->get_name()), std::string(f->get_value())};
            },
            field);

        m_map.erase(name, value, HeaderKeyType::FullMatch);
        if (auto current_name_match = m_map.find(name, "", HeaderKeyType::NameOnly)) {
            if (*current_name_match == oldest_gen)
                m_map.erase(name, "", HeaderKeyType::NameOnly);
        }
        m_current_size -= (name.size() + value.size() + ENTRY_OVERHEAD);
        m_deque.pop_back();
    }


    void evict_all() {
        m_map.clear();
        m_deque.clear();
        m_current_size = 0;
    }

    std::size_t m_max_size;
    std::size_t m_current_size;
    std::size_t m_generation;
    std::deque<shared::http::HeaderEntry> m_deque;
    QpackMap m_map;
};

} // namespace io::shared_codec::table
