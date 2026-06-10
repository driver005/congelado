module io.codec.shared.table;
@nogc nothrow:

import io.codec.shared.types;
import io.codec.shared.consts;
import io.shared.http.header;
import io.shared.http.types : Token, token_to_string;
import utils.hashmap.swiss : SwissHashMap;
import util.alloc : make, dispose;

// PORT-NOTE: C++ Overloaded<Ts...> visitor helper → not needed in D; we use
// switch/if chains on HeaderEntryKind instead of std::visit.

enum HeaderKeyType : bool { NAME_ONLY = false, FULL_MATCH = true }

// PORT-NOTE: C++ std::variant<Token, string_view> m_name → D tagged union in HeaderKey.
enum HeaderNameKind : ubyte { Token, String }

// HeaderKey — lookup key for the HPACK/QPACK table maps.
// PORT-NOTE: C++ class HeaderKey with std::variant member → D struct HeaderKey.
// PORT-NOTE: ABI POD value wrapper, exempt from class-only rule.
struct HeaderKey {
    // PORT-NOTE: value wrapper (struct), exempt from class-only rule
    HeaderNameKind name_kind;
    union {
        Token          name_token;
        const(char)[]  name_string;
    }
    const(char)[]  m_value;
    HeaderKeyType  m_type;

    static HeaderKey from_string(const(char)[] name, const(char)[] value = null,
                                 HeaderKeyType type = HeaderKeyType.NAME_ONLY) {
        // PORT-NOTE: C++ throws on empty name → D returns invalid key with name_kind.String
        HeaderKey k;
        k.name_kind   = HeaderNameKind.String;
        k.name_string = name;
        k.m_value     = value;
        k.m_type      = type;
        return k;
    }

    static HeaderKey from_token(Token token, const(char)[] value = null,
                                HeaderKeyType type = HeaderKeyType.NAME_ONLY) {
        HeaderKey k;
        k.name_kind  = HeaderNameKind.Token;
        k.name_token = token;
        k.m_value    = value;
        k.m_type     = type;
        return k;
    }

    bool opEquals(ref const HeaderKey other) const pure {
        if (m_type != other.m_type) return false;
        bool name_match = names_equal(other);
        if (!name_match) return false;
        return (m_type == HeaderKeyType.NAME_ONLY) || (m_value == other.m_value);
    }

    bool is_equal(const(char)[] name, const(char)[] value, HeaderKeyType type) const pure {
        if (m_type != type) return false;
        bool nm = (name_kind == HeaderNameKind.Token)
                  ? (token_to_string(name_token) == name)
                  : (name_string == name);
        if (!nm) return false;
        return (m_type == HeaderKeyType.NAME_ONLY) || (m_value == value);
    }

    bool is_equal(Token token, const(char)[] value, HeaderKeyType type) const pure {
        if (m_type != type) return false;
        bool nm = (name_kind == HeaderNameKind.Token)
                  ? (name_token == token)
                  : (name_string == token_to_string(token));
        if (!nm) return false;
        return (m_type == HeaderKeyType.NAME_ONLY) || (m_value == value);
    }

    const(char)[] get_name_str() const pure {
        return (name_kind == HeaderNameKind.Token) ? token_to_string(name_token) : name_string;
    }
    const(char)[] get_value() const pure { return m_value; }
    HeaderKeyType get_type()  const pure { return m_type;  }

private:
    bool names_equal(ref const HeaderKey other) const pure {
        if (name_kind == HeaderNameKind.Token && other.name_kind == HeaderNameKind.Token)
            return name_token == other.name_token;
        const(char)[] a = (name_kind == HeaderNameKind.Token) ? token_to_string(name_token) : name_string;
        const(char)[] b = (other.name_kind == HeaderNameKind.Token) ? token_to_string(other.name_token) : other.name_string;
        return a == b;
    }
}

// PORT-NOTE: C++ HeaderHasher → D toHash free function for use with SwissHashMap.
// Helper: Combines bits using the Golden Ratio to prevent collisions
void hash_combine(ref size_t seed, size_t value) pure {
    seed ^= value + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

size_t header_key_hash(ref const HeaderKey key) pure {
    const(char)[] name_str = key.get_name_str();
    size_t hash = fnv1a(name_str);
    if (key.get_type() == HeaderKeyType.FULL_MATCH)
        hash_combine(hash, fnv1a(key.get_value()));
    hash_combine(hash, cast(size_t)key.get_type());
    return hash;
}

// Simple FNV-1a hash for const(char)[] (replaces std::hash<string_view>)
size_t fnv1a(const(char)[] s) pure {
    size_t h = 14695981039346656037UL;
    foreach (c; s)
        h = (h ^ cast(ubyte)c) * 1099511628211UL;
    return h;
}

// PORT-NOTE: C++ hashmap::swiss::SwissHashMap<HeaderKey, size_t, HeaderHasher, HeaderEqual>
// → D SwissHashMap!(HeaderKey, size_t) with custom hash via free function.
// TODO: wire in custom hash/equal to SwissHashMap template in improvement pass.
alias QpackMap = SwissHashMap!(HeaderKey, size_t);

// StaticTable — wraps a static array of HeaderFieldStatic* pointers.
// PORT-NOTE: C++ template<const auto& Table> StaticTable → D template struct StaticTable!Table.
//
// Note: In D we cannot template on non-type slice parameters in exactly the same way.
// The table is passed as a static-array reference via a runtime parameter in constructors.
// To retain the C++ spirit, we use a wrapper struct that stores a pointer to the table
// and its length, set once at module-init time via a static helper function.
// PORT-NOTE: ABI class (has behavior) per rules → kept as struct because it is stateless
//            (all methods are static/const lookups). The state is a compile-time constant
//            table ref, which cannot be a D class template parameter.
struct StaticTableBase {
    // PORT-NOTE: runtime-init static table ref (set by HPACK/QPACK table modules)
    const(HeaderFieldStatic*)[] m_table; // slice of static HeaderFieldStatic entries
    QpackMap m_map;

    // Internal init — called by HPACK/QPACK modules after filling STATIC_TABLE arrays.
    void init(const(HeaderFieldStatic*)[] table) {
        m_table = table;
        // Build the lookup map
        for (size_t i = 0; i < table.length; ++i) {
            const auto* field = table[i];
            auto full_key  = HeaderKey.from_token(field.m_name, field.m_value, HeaderKeyType.FULL_MATCH);
            auto name_key  = HeaderKey.from_token(field.m_name, null, HeaderKeyType.NAME_ONLY);
            m_map.upsert(full_key, i);
            m_map.upsert(name_key, i);
        }
    }

    const(HeaderFieldStatic)* at(size_t idx) const pure {
        if (idx >= m_table.length) return null;
        return m_table[idx];
    }

    size_t size() const pure { return m_table.length; }

    SearchResult search(IndexCalculation calc = IndexCalculation.Q_PACK)(
            const(char)[] name, const(char)[] value) const {
        if (auto r = search_full_match!calc(name, value); r.found())
            return r;
        if (auto r = search_name_only!calc(name); r.found())
            return r;
        return SearchResult.none();
    }

    SearchResult search_full_match(IndexCalculation calc = IndexCalculation.Q_PACK)(
            const(char)[] name, const(char)[] value) const {
        auto key = HeaderKey.from_string(name, value, HeaderKeyType.FULL_MATCH);
        if (auto pos = m_map.find(key)) {
            return SearchResult.make(*pos + (calc == IndexCalculation.H_PACK ? 1 : 0), true, true);
        }
        return SearchResult.none();
    }

    SearchResult search_name_only(IndexCalculation calc = IndexCalculation.Q_PACK)(
            const(char)[] name) const {
        auto key = HeaderKey.from_string(name, null, HeaderKeyType.NAME_ONLY);
        if (auto pos = m_map.find(key)) {
            return SearchResult.make(*pos + (calc == IndexCalculation.H_PACK ? 1 : 0), true, false);
        }
        return SearchResult.none();
    }
}

// DynamicTable — HPACK/QPACK dynamic header table with eviction.
// PORT-NOTE: C++ class DynamicTable → D class DynamicTable (has behavior, GC-managed).
// Callers: use make!DynamicTable / dispose for @nogc heap management.
class DynamicTable {
  public:
    this(size_t max_size = 4096) {
        m_max_size     = max_size;
        m_current_size = 0;
        m_generation   = 0;
        // TODO: add reserve support to our map and set an initial capacity based on max_size and average entry size
        // m_map.reserve(128); // Initial capacity to reduce early collisions
    }

    size_t insert(IndexCalculation calc = IndexCalculation.Q_PACK)(
            const(char)[] name, const(char)[] value) {
        auto* field = make!HeaderField();
        field.m_name  = name;
        field.m_value = value;
        return insert_impl!calc(field, false);
    }

    size_t insert(IndexCalculation calc = IndexCalculation.Q_PACK)(
            Token token, const(char)[] value) {
        auto* field = make!HeaderFieldStatic();
        field.m_name  = token;
        field.m_value = value;
        return insert_static_impl!calc(field);
    }

    // PORT-NOTE: C++ deque<HeaderEntry> m_deque → D dynamic array m_deque[]
    // (GC-managed in this class; improvement pass can replace with a ring buffer).
    SearchResult search(IndexCalculation calc = IndexCalculation.Q_PACK)(
            const(char)[] name, const(char)[] value) const {
        if (auto r = search_full_match!calc(name, value); r.found()) return r;
        if (auto r = search_name_only!calc(name); r.found()) return r;
        return SearchResult.none();
    }

    SearchResult search_full_match(IndexCalculation calc = IndexCalculation.Q_PACK)(
            const(char)[] name, const(char)[] value) const {
        auto key = HeaderKey.from_string(name, value, HeaderKeyType.FULL_MATCH);
        if (auto gen = m_map.find(key)) {
            static if (calc == IndexCalculation.Q_PACK)
                return SearchResult.make(*gen, true, true);
            else
                return SearchResult.make(generation_to_position(*gen), true, true);
        }
        return SearchResult.none();
    }

    SearchResult search_name_only(IndexCalculation calc = IndexCalculation.Q_PACK)(
            const(char)[] name) const {
        auto key = HeaderKey.from_string(name, null, HeaderKeyType.NAME_ONLY);
        if (auto gen = m_map.find(key)) {
            static if (calc == IndexCalculation.Q_PACK)
                return SearchResult.make(*gen, false, false);
            else
                return SearchResult.make(generation_to_position(*gen), false, false);
        }
        return SearchResult.none();
    }

    HeaderEntry* at_position(size_t pos) const {
        if (pos >= m_deque.length) return null;
        return cast(HeaderEntry*)m_deque[pos]; // cast away immutability for return
    }

    HeaderEntry* at_generation(size_t gen) const {
        const size_t POS = generation_to_position(gen);
        if (POS == SearchResult.NPOS) return null;
        return cast(HeaderEntry*)m_deque[POS];
    }

    size_t generation_to_position(size_t gen) const pure {
        if (gen == 0 || gen > m_generation || m_deque.length == 0)
            return SearchResult.NPOS;
        const size_t OLDEST = m_generation - (m_deque.length - 1);
        if (gen < OLDEST)
            return SearchResult.NPOS;
        return m_generation - gen;
    }

    void set_max_size(size_t new_max) {
        m_max_size = new_max;
        while (m_deque.length > 0 && m_current_size > m_max_size)
            evict_oldest();
    }

    size_t get_size()         const pure { return m_deque.length; }
    size_t get_current_size() const pure { return m_current_size; }
    size_t get_insert_count() const pure { return m_generation;   }
    size_t get_max_size()     const pure { return m_max_size;     }

  private:
    // PORT-NOTE: HeaderEntry is a tagged union; we store it by value in the deque.
    struct DequeEntry {
        // PORT-NOTE: value wrapper (struct), exempt from class-only rule
        bool  is_static;
        union {
            HeaderFieldStatic* static_field;
            HeaderField*       dynamic_field;
        }
        const(char)[] get_name_str() const pure {
            if (is_static) return token_to_string(static_field.m_name);
            return dynamic_field.m_name;
        }
        const(char)[] get_value() const pure {
            if (is_static) return static_field.m_value;
            return dynamic_field.m_value;
        }
        size_t entry_size() const pure {
            if (is_static) return static_field.size();
            return dynamic_field.size();
        }
    }

    size_t insert_impl(IndexCalculation calc)(HeaderField* field, bool is_static_flag) {
        const size_t ENTRY_SIZE = field.size();
        if (ENTRY_SIZE > m_max_size) {
            evict_all();
            return 0;
        }

        while (m_deque.length > 0 && m_current_size + ENTRY_SIZE > m_max_size)
            evict_oldest();

        ++m_generation;
        m_current_size += ENTRY_SIZE;

        auto full_key = HeaderKey.from_string(field.m_name, field.m_value, HeaderKeyType.FULL_MATCH);
        auto name_key = HeaderKey.from_string(field.m_name, null, HeaderKeyType.NAME_ONLY);
        m_map.upsert(full_key, m_generation);
        m_map.upsert(name_key, m_generation);

        // prepend to deque (push_front)
        DequeEntry entry;
        entry.is_static      = false;
        entry.dynamic_field  = field;
        // PORT-NOTE: D dynamic array has no push_front; use a simple prepend.
        // TODO: improvement — use a ring buffer for O(1) push_front/pop_back.
        m_deque = [entry] ~ m_deque;

        static if (calc == IndexCalculation.Q_PACK)
            return m_generation;
        else
            return generation_to_position(m_generation);
    }

    size_t insert_static_impl(IndexCalculation calc)(HeaderFieldStatic* field) {
        const size_t ENTRY_SIZE = field.size();
        if (ENTRY_SIZE > m_max_size) {
            evict_all();
            return 0;
        }

        while (m_deque.length > 0 && m_current_size + ENTRY_SIZE > m_max_size)
            evict_oldest();

        ++m_generation;
        m_current_size += ENTRY_SIZE;

        const(char)[] name_str = token_to_string(field.m_name);
        auto full_key = HeaderKey.from_string(name_str, field.m_value, HeaderKeyType.FULL_MATCH);
        auto name_key = HeaderKey.from_string(name_str, null, HeaderKeyType.NAME_ONLY);
        m_map.upsert(full_key, m_generation);
        m_map.upsert(name_key, m_generation);

        DequeEntry entry;
        entry.is_static    = true;
        entry.static_field = field;
        m_deque = [entry] ~ m_deque;

        static if (calc == IndexCalculation.Q_PACK)
            return m_generation;
        else
            return generation_to_position(m_generation);
    }

    void evict_oldest() {
        if (m_deque.length == 0) return;
        const ref field = m_deque[$ - 1];
        const size_t OLDEST_GEN = m_generation - (m_deque.length - 1);

        const(char)[] name  = field.get_name_str();
        const(char)[] value = field.get_value();

        auto full_key = HeaderKey.from_string(name, value, HeaderKeyType.FULL_MATCH);
        auto name_key = HeaderKey.from_string(name, null, HeaderKeyType.NAME_ONLY);
        m_map.erase(full_key);
        if (auto current_name_match = m_map.find(name_key)) {
            if (*current_name_match == OLDEST_GEN)
                m_map.erase(name_key);
        }
        m_current_size -= (name.length + value.length + ENTRY_OVERHEAD);
        m_deque = m_deque[0 .. $ - 1];
    }

    void evict_all() {
        m_map.clear();
        m_deque.length = 0;
        m_current_size = 0;
    }

    size_t       m_max_size;
    size_t       m_current_size;
    size_t       m_generation;
    DequeEntry[] m_deque;  // front = newest, back = oldest
    QpackMap     m_map;
}
