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
    /**
     * @brief Builds a key from a plain string name — for headers that don't have a static-table
     * token (custom/non-standard header names).
     * @param name the header name; borrowed as a string_view, so it must outlive this key.
     * @param value optional header value; only matters when `type` is FULL_MATCH.
     * @param type NAME_ONLY (default) ignores `value` for equality/hashing, FULL_MATCH includes it.
     * @throws std::runtime_error if `name` is empty — an empty header name is never valid, no
     * exceptions carved out.
     */
    HeaderKey(std::string_view name, std::string_view value = {},
              HeaderKeyType type = HeaderKeyType::NAME_ONLY)
        : m_name(name), m_value(value), m_type(type) {
        // Guard clause — an empty string-name key is never valid, straight throw.
        if (std::holds_alternative<std::string_view>(m_name) &&
            std::get<std::string_view>(m_name).empty()) {
            throw std::runtime_error("Header name cannot be empty");
        }
    }
    /**
     * @brief Builds a key from a well-known header Token — the fast path for static-table
     * headers, no string comparison needed for the name half.
     * @param token the header's interned Token.
     * @param value optional header value; only matters when `type` is FULL_MATCH.
     * @param type NAME_ONLY (default) ignores `value` for equality/hashing, FULL_MATCH includes it.
     */
    HeaderKey(interfaces::io::types::Token token, std::string_view value = {},
              HeaderKeyType type = HeaderKeyType::NAME_ONLY)
        : m_name(token), m_value(value), m_type(type) {}

    /**
     * @brief Full equality: same name variant, same type, and (for FULL_MATCH) same value.
     * @param other the key to compare against.
     * @return true if both keys represent the same header entry.
     */
    // Compares m_name via std::get_if instead of std::variant::operator== — libstdc++'s
    // variant::operator== goes through std::get<>() internally, which carries an unconditional
    // (defensive, unreachable-here) valueless_by_exception() check that throws; std::get_if has
    // no throwing path at all, so dispatching manually on index() + get_if sidesteps the escape
    // entirely instead of just asserting it away.
    bool operator==(const HeaderKey &other) const noexcept {
        if (m_name.index() != other.m_name.index()) {
            return false;
        }

        bool name_equal = false;
        if (const auto *lhs = std::get_if<interfaces::io::types::Token>(&m_name)) {
            name_equal = (*lhs == *std::get_if<interfaces::io::types::Token>(&other.m_name));
        } else if (const auto *lhs_view = std::get_if<std::string_view>(&m_name)) {
            name_equal = (*lhs_view == *std::get_if<std::string_view>(&other.m_name));
        }

        return name_equal && m_type == other.m_type &&
               (m_type == HeaderKeyType::NAME_ONLY || m_value == other.m_value);
    }

    /**
     * @brief Transparent-lookup equality against a raw string name/value/type triple — lets the
     * hash map compare without materializing a whole HeaderKey first.
     * @param name the header name to compare against this key's name.
     * @param value the header value to compare against this key's value (only checked for
     * FULL_MATCH).
     * @param type must match this key's type or it's an instant false.
     * @return true if this key matches the given name/value/type.
     */
    // Dispatches on m_name via std::get_if rather than std::visit — libstdc++'s std::visit
    // lowers to a jump table with an unconditional (defensive, unreachable-here)
    // __throw_bad_variant_access fallback for the "no alternative matched" case; std::get_if
    // never throws, so manual index()-driven dispatch removes the throwing path outright.
    [[nodiscard]] bool is_equal(std::string_view name, std::string_view value,
                                HeaderKeyType type) const noexcept {
        // Type mismatch is an instant false — no point comparing name/value at all.
        if (m_type != type) {
            return false;
        }

        // Dispatch on whichever variant alternative this key's name actually holds —
        // Token gets stringified for comparison, string_view compares directly.
        bool name_match = false;
        if (const auto *token = std::get_if<interfaces::io::types::Token>(&m_name)) {
            name_match = interfaces::io::types::token_to_string(*token) == name;
        } else if (const auto *view = std::get_if<std::string_view>(&m_name)) {
            name_match = (*view == name);
        }

        if (!name_match) {
            return false;
        }
        // Name matched — for NAME_ONLY that's the whole answer, FULL_MATCH also needs the value.
        return (m_type == HeaderKeyType::NAME_ONLY) || (m_value == value);
    }

    /**
     * @brief Transparent-lookup equality against a Token/value/type triple — the Token-keyed
     * sibling of the string_view overload above.
     * @param token the header Token to compare against this key's name.
     * @param value the header value to compare against this key's value (only checked for
     * FULL_MATCH).
     * @param type must match this key's type or it's an instant false.
     * @return true if this key matches the given token/value/type.
     */
    // Same std::get_if-based dispatch as the string_view overload of is_equal() above — see
    // that overload's comment for why this avoids std::visit's internal throwing fallback.
    [[nodiscard]] bool is_equal(interfaces::io::types::Token token, std::string_view value,
                                HeaderKeyType type) const noexcept {
        // Same shape as the string_view overload — type mismatch bails immediately.
        if (m_type != type) {
            return false;
        }

        // Token-keyed sibling of the dispatch above: Token compares directly, string_view gets
        // the token stringified for comparison.
        bool name_match = false;
        if (const auto *stored_token = std::get_if<interfaces::io::types::Token>(&m_name)) {
            name_match = (*stored_token == token);
        } else if (const auto *view = std::get_if<std::string_view>(&m_name)) {
            name_match = (*view == interfaces::io::types::token_to_string(token));
        }

        if (!name_match) {
            return false;
        }

        return (m_type == HeaderKeyType::NAME_ONLY) || (m_value == value);
    }

    /**
     * @brief Gets the raw name variant — either a Token or a string_view, caller's job to check
     * which.
     * @return the stored name variant.
     */
    [[nodiscard]] constexpr std::variant<interfaces::io::types::Token, std::string_view>
    get_name() const noexcept {
        return m_name;
    }
    /**
     * @brief Gets the stored value.
     * @return the header value (meaningless unless getType() is FULL_MATCH).
     */
    [[nodiscard]] constexpr std::string_view get_value() const noexcept { return m_value; }
    /**
     * @brief Gets whether this key does name-only or full name+value matching.
     * @return the key's HeaderKeyType.
     */
    [[nodiscard]] constexpr HeaderKeyType get_type() const noexcept { return m_type; }

  private:
    std::variant<interfaces::io::types::Token, std::string_view> m_name;
    std::string_view m_value;
    HeaderKeyType m_type;
};

struct HeaderEqual {
    using is_transparent = void;

    /**
     * @brief Standard `HeaderKey`-to-`HeaderKey` comparator, for the plain map lookups — no
     * motion beyond delegating straight to HeaderKey::operator==.
     * @param lhs left-hand key.
     * @param rhs right-hand key.
     * @return true if the two keys are equal.
     */
    bool operator()(const HeaderKey &lhs, const HeaderKey &rhs) const noexcept {
        return lhs == rhs;
    }

    /**
     * @brief Transparent comparator overload — lets a heterogeneous lookup (name/value/type) skip
     * building a HeaderKey just to compare it.
     * @param key the map's stored key.
     * @param name the raw name to compare.
     * @param value the raw value to compare.
     * @param type the type to compare.
     * @return true if `key` matches the given name/value/type.
     */
    bool operator()(const HeaderKey &key, std::string_view name, std::string_view value,
                    HeaderKeyType type) const noexcept {
        return key.is_equal(name, value, type);
    }

    /**
     * @brief Transparent comparator overload for the Token-keyed heterogeneous lookup.
     * @param key the map's stored key.
     * @param token the raw token to compare.
     * @param value the raw value to compare.
     * @param type the type to compare.
     * @return true if `key` matches the given token/value/type.
     */
    bool operator()(const HeaderKey &key, interfaces::io::types::Token token,
                    std::string_view value, HeaderKeyType type) const noexcept {
        return key.is_equal(token, value, type);
    }
};

struct HeaderHasher {
    using is_transparent = void;

    /**
     * @brief Hashes a HeaderKey by dispatching on whichever variant alternative (Token or
     * string_view) it's actually holding.
     * @param key the key to hash.
     * @return the combined hash of `key`'s name, value (if FULL_MATCH), and type.
     */
    // Same std::get_if-based dispatch rationale as HeaderKey::is_equal() above — avoids
    // std::visit's internal throwing fallback rather than just asserting it unreachable.
    std::size_t operator()(const HeaderKey &key) const noexcept {
        const auto NAME = key.get_name();
        if (const auto *token = std::get_if<interfaces::io::types::Token>(&NAME)) {
            return hash_impl(*token, key.get_value(), key.get_type());
        }
        return hash_impl(*std::get_if<std::string_view>(&NAME), key.get_value(), key.get_type());
    }

    /**
     * @brief Transparent hash overload for the string-name heterogeneous lookup — must produce
     * the same hash a HeaderKey built from these same args would.
     * @param name the header name.
     * @param value the header value.
     * @param type the match type.
     * @return the combined hash.
     */
    std::size_t operator()(std::string_view name, std::string_view value,
                           HeaderKeyType type) const noexcept {
        return hash_impl(name, value, type);
    }

    /**
     * @brief Transparent hash overload for the Token-keyed heterogeneous lookup.
     * @param token the header token.
     * @param value the header value.
     * @param type the match type.
     * @return the combined hash.
     */
    std::size_t operator()(interfaces::io::types::Token token, std::string_view value,
                           HeaderKeyType type) const noexcept {
        return hash_impl(token, value, type);
    }

  private:
    // Helper: Combines bits using the Golden Ratio to prevent collisions
    /**
     * @brief Folds `value` into `seed` using the classic golden-ratio mix — keeps combined hashes
     * from clustering.
     * @param seed the running hash, updated in place.
     * @param value the next component's hash to fold in.
     */
    static void hash_combine(std::size_t &seed, std::size_t value) noexcept {
        seed ^= value + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }

    /**
     * @brief Hash implementation for the string-name path — name hash always counts, value hash
     * only folds in for FULL_MATCH, and the type itself gets folded in too so NAME_ONLY and
     * FULL_MATCH never collide with each other.
     * @param name the header name.
     * @param value the header value.
     * @param type the match type.
     * @return the combined hash.
     */
    static std::size_t hash_impl(std::string_view name, std::string_view value,
                                 HeaderKeyType type) noexcept {
        // Name always contributes; value only folds in for FULL_MATCH so NAME_ONLY keys don't
        // pay for a value hash they don't use.
        std::size_t hash = std::hash<std::string_view>{}(name);
        if (type == HeaderKeyType::FULL_MATCH) {
            hash_combine(hash, std::hash<std::string_view>{}(value));
        }
        // Fold the type in too, so a NAME_ONLY and FULL_MATCH key with the same name never
        // collide with each other.
        hash_combine(hash, static_cast<std::size_t>(type));
        return hash;
    }

    /**
     * @brief Hash implementation for the Token-keyed path — same shape as the string overload,
     * just hashes the Token's underlying integer instead of a string.
     * @param token the header token.
     * @param value the header value.
     * @param type the match type.
     * @return the combined hash.
     */
    static std::size_t hash_impl(interfaces::io::types::Token token, std::string_view value,
                                 HeaderKeyType type) noexcept {
        // Same shape as the string overload, just hashing the Token's underlying integer.
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

    /**
     * @brief Looks up a static-table entry by raw index.
     * @param idx the static-table index.
     * @return the entry at `idx`, or `std::nullopt` if `idx` is out of range.
     */
    static std::optional<std::shared_ptr<interfaces::io::HeaderField<true>>>
    at(std::size_t idx) noexcept {
        if (idx >= STATIC_SIZE) {
            return std::nullopt;
        }
        // Returns the shared_ptr from the static array
        return Table[idx];  // FIXME(clang-tidy): unchecked operator[], consider .at(); non-constant array index
    }

    /**
     * @brief Searches the static table, trying a full name+value match first and falling back to
     * name-only.
     * @tparam Calc HPACK vs. QPACK index numbering — HPACK indices are 1-based, QPACK 0-based, so
     * the offset math differs.
     * @param name the header name to search for.
     * @param value the header value to search for.
     * @return a found SearchResult (full or name-only match), or SearchResult::none() if neither
     * hit.
     */
    template <IndexCalculation Calc = IndexCalculation::Q_PACK>
    static SearchResult search(std::string_view name, std::string_view value) noexcept {
        // Try the tighter full name+value match first, only fall back to name-only if that
        // whiffs.
        if (auto result = search_full_match<Calc>(name, value); result.found()) {
            return result;
        }

        if (auto result = search_name_only<Calc>(name); result.found()) {
            return result;
        }

        return SearchResult::none();
    }

    /**
     * @brief Searches the static table for an exact name+value match only — no name-only
     * fallback.
     * @tparam Calc HPACK vs. QPACK index numbering.
     * @param name the header name to search for.
     * @param value the header value to search for.
     * @return a found SearchResult if both matched, SearchResult::none() otherwise.
     */
    template <IndexCalculation Calc = IndexCalculation::Q_PACK>
    static SearchResult search_full_match(std::string_view name, std::string_view value) noexcept {
        // HPACK indices are 1-based, QPACK 0-based — that +1 is the whole difference.
        if (auto positon = get_map().find(name, value, HeaderKeyType::FULL_MATCH)) {
            return SearchResult{*positon + (Calc == IndexCalculation::H_PACK), true, true};
        }

        return SearchResult::none();
    }

    /**
     * @brief Searches the static table for a name-only match, ignoring value entirely.
     * @tparam Calc HPACK vs. QPACK index numbering.
     * @param name the header name to search for.
     * @return a found SearchResult if the name matched, SearchResult::none() otherwise.
     */
    template <IndexCalculation Calc = IndexCalculation::Q_PACK>
    static SearchResult search_name_only(std::string_view name) noexcept {
        // Empty value passed to find() since NAME_ONLY entries never look at it anyway.
        if (auto positon = get_map().find(name, "", HeaderKeyType::NAME_ONLY)) {
            return SearchResult{*positon + (Calc == IndexCalculation::H_PACK), true, false};
        }

        return SearchResult::none();
    }

    /** @brief Gets the static table's entry count. @return `STATIC_SIZE`, fixed at compile time. */
    static constexpr std::size_t size() noexcept { return STATIC_SIZE; }

  private:
    // Lazily built on first use (C++11 magic-statics, thread-safe) rather than a static-duration
    // member initialized at load time — the build below calls upsert(), which can throw (e.g.
    // std::bad_alloc); this way that throw happens on first real use instead of during static
    // initialization, where it can't be caught. Private implementation detail of this class only,
    // so unlike STATIC_TABLE above it's never used as a non-type template argument anywhere.
    // FIXME(clang-tidy): bugprone-exception-escape — noexcept: search_full_match()/
    // search_name_only() above are noexcept and call this on every lookup, so this can't have a
    // narrower contract than they do. In practice only the very first call can throw (building
    // the map); same "no safe fallback for a table this codebase needs to work at all" risk
    // tolerance already accepted for STATIC_TABLE above — a failed build here is as fatal as a
    // failed static init would have been, just deferred to first use.
    static const QpackMap &get_map() noexcept {
        static const QpackMap MAP = [] {
            QpackMap built;

            // TODO: We can optimize by adding reserve support to out our map
            // m.reserve(STATIC_SIZE * 2);

            // Every static-table entry gets indexed twice — once under a full name+value key for
            // exact matches, once under a name-only key for the fallback lookup.
            for (std::size_t i = 0; i < STATIC_SIZE; ++i) {
                const auto &field = Table[i];  // FIXME(clang-tidy): unchecked operator[], consider .at(); non-constant array index

                built.upsert(
                    HeaderKey{field->get_name(), field->get_value(), HeaderKeyType::FULL_MATCH}, i);
                built.upsert(HeaderKey{field->get_name(), "", HeaderKeyType::NAME_ONLY}, i);
            }
            return built;
        }();
        return MAP;
    }
};

class DynamicTable {
  public:
    /**
     * @brief Builds an empty dynamic table capped at `max_size` bytes.
     * @param max_size the eviction budget, in bytes (RFC 7541 §4.1 accounting: entry size = name
     * + value + `ENTRY_OVERHEAD`); defaults to 4096.
     */
    explicit DynamicTable(std::size_t max_size = 4096)
        : m_max_size{max_size} {
        // TODO: add reserve support to our map and set an initial capacity based on max_size and
        // average entry size m_map.reserve(128); // Initial capacity to reduce early collisions
    }


    /**
     * @brief Builds a field from a raw name/value pair and inserts it — straightforward motion,
     * just wraps the field construction before handing off to the real insert() below.
     * @tparam Calc HPACK vs. QPACK index numbering for the returned index.
     * @param name the header name.
     * @param value the header value.
     * @return the new entry's generation (QPACK) or position (HPACK) — see the `insert` overload
     * below for the full eviction/indexing contract.
     */
    template <IndexCalculation Calc = IndexCalculation::Q_PACK>
    std::size_t insert(std::string_view name, std::string_view value) {
        auto field = std::make_shared<interfaces::io::HeaderField<false>>(name, value);
        return insert<Calc>(std::move(field));
    }

    /**
     * @brief Builds a field from a Token/value pair and inserts it.
     * @tparam Calc HPACK vs. QPACK index numbering for the returned index.
     * @param token the header's Token.
     * @param value the header value.
     * @return the new entry's generation (QPACK) or position (HPACK).
     */
    template <IndexCalculation Calc = IndexCalculation::Q_PACK>
    std::size_t insert(interfaces::io::types::Token token, std::string_view value) {
        auto field = std::make_shared<interfaces::io::HeaderField<true>>(token, value);
        return insert<Calc>(std::move(field));
    }

    /**
     * @brief Inserts an already-built field, evicting oldest entries first if it doesn't fit
     * within `m_max_size`.
     * @warning If `field` alone is bigger than `m_max_size`, this evicts the *entire* table
     * (evict_all()) and returns 0 without inserting anything — the entry never makes it in. Don't
     * assume a non-throwing insert() means the header actually landed in the table; check the
     * return.
     * @tparam Calc QPACK indices are generation numbers (monotonically increasing, never
     * reused); HPACK indices are positions (relative, shift as entries evict) — pick based on
     * which protocol's calling this.
     * @tparam IsStatic whether `field` targets the static or dynamic backing (deduced from
     * `field`'s type).
     * @param field the header field to insert; ownership moves in.
     * @return the new entry's generation (QPACK) or position (HPACK), or 0 if it got evicted
     * instead of inserted.
     */
    template <IndexCalculation Calc = IndexCalculation::Q_PACK, bool IsStatic>
    std::size_t insert(std::shared_ptr<interfaces::io::HeaderField<IsStatic>> field) {
        // Entry alone blows the whole budget — wipe the table instead of trying to make room,
        // and report it as a non-insert.
        const std::size_t ENTRY_SIZE = field->size();
        if (ENTRY_SIZE > m_max_size) {
            evict_all();
            return 0;
        }

        // Evict oldest-first until there's room for the new entry.
        while (!m_deque.empty() && m_current_size + ENTRY_SIZE > m_max_size) {
            evict_oldest();
        }

        // Generation is monotonic and never reused, even across evictions — that's what makes
        // QPACK's absolute indexing scheme work.
        ++m_generation;
        m_current_size += ENTRY_SIZE;

        // Same double-index motion as the static table: full match key plus name-only key,
        // both pointing at this generation.
        m_map.upsert(HeaderKey{field->get_name(), field->get_value(), HeaderKeyType::FULL_MATCH},
                     m_generation);
        m_map.upsert(HeaderKey{field->get_name(), "", HeaderKeyType::NAME_ONLY}, m_generation);

        m_deque.push_front(std::move(field));

        // QPACK callers want the raw generation; HPACK callers want it converted to a live
        // relative position.
        if constexpr (Calc == IndexCalculation::Q_PACK) {
            return m_generation;
        } else {
            return generation_to_position(m_generation);
        }
    }

    /**
     * @brief Searches the dynamic table, trying a full name+value match first and falling back
     * to name-only.
     * @tparam Calc HPACK vs. QPACK index numbering.
     * @param name the header name to search for.
     * @param value the header value to search for.
     * @return a found SearchResult (full or name-only match), or SearchResult::none() if neither
     * hit.
     */
    template <IndexCalculation Calc = IndexCalculation::Q_PACK>
    [[nodiscard]] SearchResult search(std::string_view name,
                                      std::string_view value) const noexcept {
        // Same full-match-then-name-only fallback pattern as StaticTable::search().
        if (auto result = search_full_match<Calc>(name, value); result.found()) {
            return result;
        }

        if (auto result = search_name_only<Calc>(name); result.found()) {
            return result;
        }

        return SearchResult::none();
    }

    /**
     * @brief Searches the dynamic table for an exact name+value match only.
     * @tparam Calc HPACK vs. QPACK index numbering — QPACK returns the raw generation, HPACK
     * converts it to a live position via generation_to_position().
     * @param name the header name to search for.
     * @param value the header value to search for.
     * @return a found SearchResult if both matched, SearchResult::none() otherwise.
     */
    template <IndexCalculation Calc = IndexCalculation::Q_PACK>
    [[nodiscard]] SearchResult search_full_match(std::string_view name,
                                                 std::string_view value) const noexcept {
        // Map stores raw generations — QPACK wants that as-is, HPACK needs it converted to a
        // live relative position first.
        if (auto generation = m_map.find(name, value, HeaderKeyType::FULL_MATCH)) {
            if constexpr (Calc == IndexCalculation::Q_PACK) {
                return SearchResult{*generation, true, true};
            } else {
                return SearchResult{generation_to_position(*generation), true, true};
            }
        }

        return SearchResult::none();
    }

    /**
     * @brief Searches the dynamic table for a name-only match, ignoring value.
     * @tparam Calc HPACK vs. QPACK index numbering.
     * @param name the header name to search for.
     * @return a found SearchResult if the name matched, SearchResult::none() otherwise.
     */
    template <IndexCalculation Calc = IndexCalculation::Q_PACK>
    [[nodiscard]] SearchResult search_name_only(std::string_view name) const noexcept {
        // Same QPACK-vs-HPACK conversion split as search_full_match() above.
        if (auto generation = m_map.find(name, "", HeaderKeyType::NAME_ONLY)) {
            if constexpr (Calc == IndexCalculation::Q_PACK) {
                return SearchResult{*generation, true, false};
            } else {
                return SearchResult{generation_to_position(*generation), true, false};
            }
        }

        return SearchResult::none();
    }

    /**
     * @brief Looks up an entry by its live HPACK-style position (relative to the newest entry).
     * @param pos the position to look up.
     * @return the entry at `pos`, or `std::nullopt` if `pos` is out of range.
     */
    [[nodiscard]] std::optional<interfaces::io::HeaderEntry>
    at_positon(std::size_t pos) const noexcept {
        return (pos < m_deque.size()) ? std::optional{m_deque[pos]} : std::nullopt;  // FIXME(clang-tidy): unchecked operator[], consider .at()
    }

    /**
     * @brief Looks up an entry by its QPACK-style generation (absolute insertion order, never
     * reused even after eviction).
     * @param gen the generation to look up.
     * @return the entry at that generation, or `std::nullopt` if it's already evicted or was
     * never inserted.
     */
    [[nodiscard]] std::optional<interfaces::io::HeaderEntry>
    at_generation(std::size_t gen) const noexcept {
        const std::size_t POS = generation_to_position(gen);
        return (POS != SearchResult::NPOS) ? std::optional{m_deque[POS]} : std::nullopt;  // FIXME(clang-tidy): unchecked operator[], consider .at()
    }

    /**
     * @brief Converts an absolute QPACK generation number into a live relative position in
     * `m_deque` — the bridge between the two indexing schemes, lowkey the trickiest bit of math
     * in this whole class.
     * @param gen the generation to convert.
     * @return the live position, or `SearchResult::NPOS` if `gen` is 0, was never issued, or has
     * already aged out of the table via eviction.
     */
    [[nodiscard]] std::size_t generation_to_position(std::size_t gen) const noexcept {
        // Generation 0 was never issued, and anything past the current counter or with an
        // empty table can't possibly be live.
        if (gen == 0 || gen > m_generation || m_deque.empty()) {
            return SearchResult::NPOS;
        }
        // Work out the oldest generation still tracked in the deque — anything older than that
        // has already been evicted.
        const std::size_t OLDEST = m_generation - (m_deque.size() - 1);
        if (gen < OLDEST) {
            return SearchResult::NPOS;
        }
        // Newest entry is position 0, so distance from the current generation is the position.
        return m_generation - gen;
    }

    /**
     * @brief Shrinks (or grows) the eviction budget, immediately evicting oldest entries if the
     * new cap is smaller than what's currently stored.
     * @param new_max the new byte budget.
     */
    void set_max_size(std::size_t new_max) {
        m_max_size = new_max;
        // Shrinking below current usage means evicting oldest-first until it fits again; growing
        // is a no-op since the loop condition's already false.
        while (!m_deque.empty() && m_current_size > m_max_size) {
            evict_oldest();
        }
    }

    /** @brief Gets how many entries are currently in the table. @return live entry count. */
    [[nodiscard]] std::size_t get_size() const noexcept { return m_deque.size(); }
    /**
     * @brief Gets how many bytes of the budget are currently used.
     * @return current byte usage per RFC 7541 accounting.
     */
    [[nodiscard]] std::size_t get_current_size() const noexcept { return m_current_size; }
    /**
     * @brief Gets the total number of inserts ever done, evicted or not.
     * @return the current generation counter.
     */
    [[nodiscard]] std::size_t get_insert_count() const noexcept { return m_generation; }
    /** @brief Gets the configured eviction budget. @return max size in bytes. */
    [[nodiscard]] std::size_t get_max_size() const noexcept { return m_max_size; }

  private:
    /**
     * @brief Evicts the single oldest entry: removes it from `m_deque`, unwinds its byte cost
     * from `m_current_size`, and cleans up its map entries — including the tricky bit where the
     * NAME_ONLY map entry only gets erased if it still points at the generation being evicted (a
     * newer entry with the same name may have already claimed that slot).
     */
    void evict_oldest() {
        if (m_deque.empty()) {
            return;
        }
        // Deque's ordered newest-to-oldest (push_front on insert), so the tail is the eviction
        // target.
        const auto &field = m_deque.back();
        const std::size_t OLDEST_GEN = m_generation - (m_deque.size() - 1);

        // Materialize the name/value as owned strings — Token-backed fields need stringifying
        // first, string-backed ones just get copied.
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

        // FULL_MATCH entry always belonged solely to this generation — safe to erase outright.
        m_map.erase(name, value, HeaderKeyType::FULL_MATCH);
        // NAME_ONLY entry might've been reclaimed by a newer insert with the same name, so only
        // erase it if it still points at the generation being evicted.
        if (auto current_name_match = m_map.find(name, "", HeaderKeyType::NAME_ONLY)) {
            if (*current_name_match == OLDEST_GEN) {
                m_map.erase(name, "", HeaderKeyType::NAME_ONLY);
            }
        }
        m_current_size -= (name.size() + value.size() + ENTRY_OVERHEAD);
        m_deque.pop_back();
    }


    /**
     * @brief Nukes the whole table, no cap — clears the map, the deque, and resets the byte
     * counter. Used when a single incoming entry is too big to ever fit, so the rest of the
     * table gets wiped alongside it rather than left in a half-consistent state.
     */
    void evict_all() {
        m_map.clear();
        m_deque.clear();
        m_current_size = 0;
    }

    std::size_t m_max_size;
    std::size_t m_current_size = 0;
    std::size_t m_generation = 0;
    std::deque<interfaces::io::HeaderEntry> m_deque;
    QpackMap m_map;
};

} // namespace io::shared_codec::table
