export module io_codec_shared:table;

import std;
import hashmap;
import interfaces;
import :types;
import :consts;

namespace io::shared_codec::table {

template <typename... Ts>
struct Overloaded : Ts... {
    using Ts::operator()...;
};

enum class HeaderKeyType : bool { NAME_ONLY = false, FULL_MATCH = true };

class HeaderKey {
  public:
    HeaderKey(std::string_view name, std::string_view value = {},
              HeaderKeyType type = HeaderKeyType::NAME_ONLY)
        : m_name(name), m_value(value), m_type(type) {
        if (std::holds_alternative<std::string_view>(m_name) &&
            std::get<std::string_view>(m_name).empty()) {
            throw std::runtime_error("Header name cannot be empty");
        }
    }
    HeaderKey(interfaces::io::types::Token token, std::string_view value = {},
              HeaderKeyType type = HeaderKeyType::NAME_ONLY)
        : m_name(token), m_value(value), m_type(type) {}

    bool operator==(const HeaderKey &other) const noexcept {
        return m_name == other.m_name && m_type == other.m_type &&
               (m_type == HeaderKeyType::NAME_ONLY || m_value == other.m_value);
    }

    [[nodiscard]] bool is_equal(std::string_view name, std::string_view value,
                                HeaderKeyType type) const noexcept {
        if (m_type != type) {
            return false;
        }

        bool name_match = std::visit(
            [&](auto &&arg) -> bool {
                using T = std::decay_t<decltype(arg)>;

                if constexpr (std::is_same_v<T, interfaces::io::types::Token>) {
                    return interfaces::io::types::token_to_string(arg) == name;
                } else if constexpr (std::is_same_v<T, std::string_view>) {
                    return arg == name;
                }
            },
            m_name);

        if (!name_match) {
            return false;
        }
        return (m_type == HeaderKeyType::NAME_ONLY) || (m_value == value);
    }

    [[nodiscard]] bool is_equal(interfaces::io::types::Token token, std::string_view value,
                                HeaderKeyType type) const noexcept {
        if (m_type != type) {
            return false;
        }

        bool name_match = std::visit(
            [&](auto &&arg) -> bool {
                using T = std::decay_t<decltype(arg)>;

                if constexpr (std::is_same_v<T, interfaces::io::types::Token>) {
                    return arg == token;
                } else if constexpr (std::is_same_v<T, std::string_view>) {
                    return arg == interfaces::io::types::token_to_string(token);
                }
            },
            m_name);

        if (!name_match) {
            return false;
        }

        return (m_type == HeaderKeyType::NAME_ONLY) || (m_value == value);
    }

    [[nodiscard]] constexpr std::variant<interfaces::io::types::Token, std::string_view>
    get_name() const noexcept {
        return m_name;
    }
    [[nodiscard]] constexpr std::string_view get_value() const noexcept { return m_value; }
    [[nodiscard]] constexpr HeaderKeyType get_type() const noexcept { return m_type; }

  private:
    std::variant<interfaces::io::types::Token, std::string_view> m_name;
    std::string_view m_value;
    HeaderKeyType m_type;
};

struct HeaderEqual {
    using is_transparent = void;

    bool operator()(const HeaderKey &lhs, const HeaderKey &rhs) const noexcept {
        return lhs == rhs;
    }

    bool operator()(const HeaderKey &key, std::string_view name, std::string_view value,
                    HeaderKeyType type) const noexcept {
        return key.is_equal(name, value, type);
    }

    bool operator()(const HeaderKey &key, interfaces::io::types::Token token,
                    std::string_view value, HeaderKeyType type) const noexcept {
        return key.is_equal(token, value, type);
    }
};

struct HeaderHasher {
    using is_transparent = void;

    std::size_t operator()(const HeaderKey &key) const noexcept {
        return std::visit(
            [&](auto &&arg) -> std::size_t {
                return hash_impl(arg, key.get_value(), key.get_type());
            },
            key.get_name());
    }

    std::size_t operator()(std::string_view name, std::string_view value,
                           HeaderKeyType type) const noexcept {
        return hash_impl(name, value, type);
    }

    std::size_t operator()(interfaces::io::types::Token token, std::string_view value,
                           HeaderKeyType type) const noexcept {
        return hash_impl(token, value, type);
    }

  private:
    // Helper: Combines bits using the Golden Ratio to prevent collisions
    static void hash_combine(std::size_t &seed, std::size_t value) noexcept {
        seed ^= value + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }

    static std::size_t hash_impl(std::string_view name, std::string_view value,
                                 HeaderKeyType type) noexcept {
        std::size_t hash = std::hash<std::string_view>{}(name);
        if (type == HeaderKeyType::FULL_MATCH) {
            hash_combine(hash, std::hash<std::string_view>{}(value));
        }
        hash_combine(hash, static_cast<std::size_t>(type));
        return hash;
    }

    static std::size_t hash_impl(interfaces::io::types::Token token, std::string_view value,
                                 HeaderKeyType type) noexcept {
        std::size_t hash = std::hash<std::uint32_t>{}(std::to_underlying(token));
        if (type == HeaderKeyType::FULL_MATCH) {
            hash_combine(hash, std::hash<std::string_view>{}(value));
        }
        hash_combine(hash, static_cast<std::size_t>(type));
        return hash;
    }
};


template <typename T>
concept StaticHeaderTable = requires(T table) {
    { std::size(table) } -> std::convertible_to<std::size_t>;
    requires std::same_as<std::decay_t<decltype(table[0])>,
                          std::shared_ptr<interfaces::io::HeaderField<true>>>;
};

using QpackMap = hashmap::swiss::SwissHashMap<HeaderKey, std::size_t, HeaderHasher, HeaderEqual>;

} // namespace io::shared_codec::table

export namespace io::shared_codec::table {

template <const auto &Table>
    requires StaticHeaderTable<decltype(Table)>
class StaticTable {
  public:
    static constexpr std::size_t STATIC_SIZE = std::size(Table);

    static std::optional<std::shared_ptr<interfaces::io::HeaderField<true>>>
    at(std::size_t idx) noexcept {
        if (idx >= STATIC_SIZE) {
            return std::nullopt;
        }
        // Returns the shared_ptr from the static array
        return Table[idx];
    }

    template <IndexCalculation Calc = IndexCalculation::Q_PACK>
    static SearchResult search(std::string_view name, std::string_view value) noexcept {
        if (auto result = search_full_match<Calc>(name, value); result.found()) {
            return result;
        }

        if (auto result = search_name_only<Calc>(name); result.found()) {
            return result;
        }

        return SearchResult::none();
    }

    template <IndexCalculation Calc = IndexCalculation::Q_PACK>
    static SearchResult search_full_match(std::string_view name, std::string_view value) noexcept {
        if (auto positon = MAP.find(name, value, HeaderKeyType::FULL_MATCH)) {
            return SearchResult{*positon + (Calc == IndexCalculation::H_PACK), true, true};
        }

        return SearchResult::none();
    }

    template <IndexCalculation Calc = IndexCalculation::Q_PACK>
    static SearchResult search_name_only(std::string_view name) noexcept {
        if (auto positon = MAP.find(name, "", HeaderKeyType::NAME_ONLY)) {
            return SearchResult{*positon + (Calc == IndexCalculation::H_PACK), true, false};
        }

        return SearchResult::none();
    }

    static constexpr std::size_t size() noexcept { return STATIC_SIZE; }

  private:
    static inline const QpackMap MAP = [] {
        QpackMap map;

        // TODO: We can optimize by adding reserve support to out our map
        // m.reserve(STATIC_SIZE * 2);

        for (std::size_t i = 0; i < STATIC_SIZE; ++i) {
            auto field = Table[i];

            map.upsert(HeaderKey{field->get_name(), field->get_value(), HeaderKeyType::FULL_MATCH},
                       i);
            map.upsert(HeaderKey{field->get_name(), "", HeaderKeyType::NAME_ONLY}, i);
        }
        return map;
    }();
};

class DynamicTable {
  public:
    explicit DynamicTable(std::size_t max_size = 4096)
        : m_max_size{max_size}, m_current_size{0}, m_generation{0} {
        // TODO: add reserve support to our map and set an initial capacity based on max_size and
        // average entry size m_map.reserve(128); // Initial capacity to reduce early collisions
    }


    template <IndexCalculation Calc = IndexCalculation::Q_PACK>
    std::size_t insert(std::string_view name, std::string_view value) {
        auto field = std::make_shared<interfaces::io::HeaderField<false>>(name, value);
        return insert<Calc>(std::move(field));
    }

    template <IndexCalculation Calc = IndexCalculation::Q_PACK>
    std::size_t insert(interfaces::io::types::Token token, std::string_view value) {
        auto field = std::make_shared<interfaces::io::HeaderField<true>>(token, value);
        return insert<Calc>(std::move(field));
    }

    template <IndexCalculation Calc = IndexCalculation::Q_PACK, bool IsStatic>
    std::size_t insert(std::shared_ptr<interfaces::io::HeaderField<IsStatic>> field) {
        const std::size_t ENTRY_SIZE = field->size();
        if (ENTRY_SIZE > m_max_size) {
            evict_all();
            return 0;
        }

        while (!m_deque.empty() && m_current_size + ENTRY_SIZE > m_max_size) {
            evict_oldest();
        }

        ++m_generation;
        m_current_size += ENTRY_SIZE;

        m_map.upsert(HeaderKey{field->get_name(), field->get_value(), HeaderKeyType::FULL_MATCH},
                     m_generation);
        m_map.upsert(HeaderKey{field->get_name(), "", HeaderKeyType::NAME_ONLY}, m_generation);

        m_deque.push_front(std::move(field));

        if constexpr (Calc == IndexCalculation::Q_PACK) {
            return m_generation;
        } else {
            return generation_to_position(m_generation);
        }
    }

    template <IndexCalculation Calc = IndexCalculation::Q_PACK>
    [[nodiscard]] SearchResult search(std::string_view name,
                                      std::string_view value) const noexcept {
        if (auto result = search_full_match<Calc>(name, value); result.found()) {
            return result;
        }

        if (auto result = search_name_only<Calc>(name); result.found()) {
            return result;
        }

        return SearchResult::none();
    }

    template <IndexCalculation Calc = IndexCalculation::Q_PACK>
    [[nodiscard]] SearchResult search_full_match(std::string_view name,
                                                 std::string_view value) const noexcept {
        if (auto generation = m_map.find(name, value, HeaderKeyType::FULL_MATCH)) {
            if constexpr (Calc == IndexCalculation::Q_PACK) {
                return SearchResult{*generation, true, true};
            } else {
                return SearchResult{generation_to_position(*generation), true, true};
            }
        }

        return SearchResult::none();
    }

    template <IndexCalculation Calc = IndexCalculation::Q_PACK>
    [[nodiscard]] SearchResult search_name_only(std::string_view name) const noexcept {
        if (auto generation = m_map.find(name, "", HeaderKeyType::NAME_ONLY)) {
            if constexpr (Calc == IndexCalculation::Q_PACK) {
                return SearchResult{*generation, true, false};
            } else {
                return SearchResult{generation_to_position(*generation), true, false};
            }
        }

        return SearchResult::none();
    }

    [[nodiscard]] std::optional<interfaces::io::HeaderEntry>
    at_positon(std::size_t pos) const noexcept {
        return (pos < m_deque.size()) ? std::optional{m_deque[pos]} : std::nullopt;
    }

    [[nodiscard]] std::optional<interfaces::io::HeaderEntry>
    at_generation(std::size_t gen) const noexcept {
        const std::size_t POS = generation_to_position(gen);
        return (POS != SearchResult::NPOS) ? std::optional{m_deque[POS]} : std::nullopt;
    }

    [[nodiscard]] std::size_t generation_to_position(std::size_t gen) const noexcept {
        if (gen == 0 || gen > m_generation || m_deque.empty()) {
            return SearchResult::NPOS;
        }
        const std::size_t OLDEST = m_generation - (m_deque.size() - 1);
        if (gen < OLDEST) {
            return SearchResult::NPOS;
        }
        return m_generation - gen;
    }

    void set_max_size(std::size_t new_max) {
        m_max_size = new_max;
        while (!m_deque.empty() && m_current_size > m_max_size) {
            evict_oldest();
        }
    }

    [[nodiscard]] std::size_t get_size() const noexcept { return m_deque.size(); }
    [[nodiscard]] std::size_t get_current_size() const noexcept { return m_current_size; }
    [[nodiscard]] std::size_t get_insert_count() const noexcept { return m_generation; }
    [[nodiscard]] std::size_t get_max_size() const noexcept { return m_max_size; }

  private:
    void evict_oldest() {
        if (m_deque.empty()) {
            return;
        }
        const auto &field = m_deque.back();
        const std::size_t OLDEST_GEN = m_generation - (m_deque.size() - 1);

        const auto [name, value] = std::visit(
            [](const auto &field) -> std::pair<std::string, std::string> {
                if constexpr (std::is_same_v<std::decay_t<decltype(field)>,
                                             std::shared_ptr<interfaces::io::HeaderField<true>>>) {
                    return {std::string(interfaces::io::types::token_to_string(field->get_name())),
                            std::string(field->get_value())};
                } else {
                    return {std::string(field->get_name()), std::string(field->get_value())};
                }
            },
            field);

        m_map.erase(name, value, HeaderKeyType::FULL_MATCH);
        if (auto current_name_match = m_map.find(name, "", HeaderKeyType::NAME_ONLY)) {
            if (*current_name_match == OLDEST_GEN) {
                m_map.erase(name, "", HeaderKeyType::NAME_ONLY);
            }
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
    std::deque<interfaces::io::HeaderEntry> m_deque;
    QpackMap m_map;
};

} // namespace io::shared_codec::table
