module io.codec.hpack.table;
@nogc nothrow:

import util.alloc : make, dispose;

import io.codec.shared;
import io.shared.http.header;
import io.shared.http.types : Token;
import io.codec.hpack.consts;

// PORT-NOTE: C++ inline const std::array<shared_ptr<HeaderField<true>>, 61> STATIC_TABLE
// → D static immutable array of HeaderFieldStatic values (no heap allocation).
// Improvement idea: generate STATIC_TABLE as CTFE to avoid the static this() runtime init.
private __gshared HeaderFieldStatic[61] _STATIC_FIELDS;
private __gshared const(HeaderFieldStatic)*[61] STATIC_TABLE_PTRS;

private shared static this() {
    _STATIC_FIELDS[ 0] = HeaderFieldStatic(Token.AUTHORITY, "");
    _STATIC_FIELDS[ 1] = HeaderFieldStatic(Token.METHOD, "GET");
    _STATIC_FIELDS[ 2] = HeaderFieldStatic(Token.METHOD, "POST");
    _STATIC_FIELDS[ 3] = HeaderFieldStatic(Token.PATH, "/");
    _STATIC_FIELDS[ 4] = HeaderFieldStatic(Token.PATH, "/index.html");
    _STATIC_FIELDS[ 5] = HeaderFieldStatic(Token.SCHEME, "http");
    _STATIC_FIELDS[ 6] = HeaderFieldStatic(Token.SCHEME, "https");
    _STATIC_FIELDS[ 7] = HeaderFieldStatic(Token.STATUS, "200");
    _STATIC_FIELDS[ 8] = HeaderFieldStatic(Token.STATUS, "204");
    _STATIC_FIELDS[ 9] = HeaderFieldStatic(Token.STATUS, "206");
    _STATIC_FIELDS[10] = HeaderFieldStatic(Token.STATUS, "304");
    _STATIC_FIELDS[11] = HeaderFieldStatic(Token.STATUS, "400");
    _STATIC_FIELDS[12] = HeaderFieldStatic(Token.STATUS, "404");
    _STATIC_FIELDS[13] = HeaderFieldStatic(Token.STATUS, "500");
    _STATIC_FIELDS[14] = HeaderFieldStatic(Token.ACCEPT_CHARSET, "");
    _STATIC_FIELDS[15] = HeaderFieldStatic(Token.ACCEPT_ENCODING, "gzip, deflate");
    _STATIC_FIELDS[16] = HeaderFieldStatic(Token.ACCEPT_LANGUAGE, "");
    _STATIC_FIELDS[17] = HeaderFieldStatic(Token.ACCEPT_RANGES, "");
    _STATIC_FIELDS[18] = HeaderFieldStatic(Token.ACCEPT, "");
    _STATIC_FIELDS[19] = HeaderFieldStatic(Token.ACCESS_CONTROL_ALLOW_ORIGIN, "");
    _STATIC_FIELDS[20] = HeaderFieldStatic(Token.AGE, "");
    _STATIC_FIELDS[21] = HeaderFieldStatic(Token.ALLOW, "");
    _STATIC_FIELDS[22] = HeaderFieldStatic(Token.AUTHORIZATION, "");
    _STATIC_FIELDS[23] = HeaderFieldStatic(Token.CACHE_CONTROL, "");
    _STATIC_FIELDS[24] = HeaderFieldStatic(Token.CONTENT_DISPOSITION, "");
    _STATIC_FIELDS[25] = HeaderFieldStatic(Token.CONTENT_ENCODING, "");
    _STATIC_FIELDS[26] = HeaderFieldStatic(Token.CONTENT_LANGUAGE, "");
    _STATIC_FIELDS[27] = HeaderFieldStatic(Token.CONTENT_LENGTH, "");
    _STATIC_FIELDS[28] = HeaderFieldStatic(Token.CONTENT_LOCATION, "");
    _STATIC_FIELDS[29] = HeaderFieldStatic(Token.CONTENT_RANGE, "");
    _STATIC_FIELDS[30] = HeaderFieldStatic(Token.CONTENT_TYPE, "");
    _STATIC_FIELDS[31] = HeaderFieldStatic(Token.COOKIE, "");
    _STATIC_FIELDS[32] = HeaderFieldStatic(Token.DATE, "");
    _STATIC_FIELDS[33] = HeaderFieldStatic(Token.E_TAG, "");
    _STATIC_FIELDS[34] = HeaderFieldStatic(Token.EXPECT, "");
    _STATIC_FIELDS[35] = HeaderFieldStatic(Token.EXPIRES, "");
    _STATIC_FIELDS[36] = HeaderFieldStatic(Token.FROM, "");
    _STATIC_FIELDS[37] = HeaderFieldStatic(Token.HOST, "");
    _STATIC_FIELDS[38] = HeaderFieldStatic(Token.IF_MATCH, "");
    _STATIC_FIELDS[39] = HeaderFieldStatic(Token.IF_MODIFIED_SINCE, "");
    _STATIC_FIELDS[40] = HeaderFieldStatic(Token.IF_NONE_MATCH, "");
    _STATIC_FIELDS[41] = HeaderFieldStatic(Token.IF_RANGE, "");
    _STATIC_FIELDS[42] = HeaderFieldStatic(Token.IF_UNMODIFIED_SINCE, "");
    _STATIC_FIELDS[43] = HeaderFieldStatic(Token.LAST_MODIFIED, "");
    _STATIC_FIELDS[44] = HeaderFieldStatic(Token.LINK, "");
    _STATIC_FIELDS[45] = HeaderFieldStatic(Token.LOCATION, "");
    _STATIC_FIELDS[46] = HeaderFieldStatic(Token.MAX_FORWARDS, "");
    _STATIC_FIELDS[47] = HeaderFieldStatic(Token.PROXY_AUTHENTICATE, "");
    _STATIC_FIELDS[48] = HeaderFieldStatic(Token.PROXY_AUTHORIZATION, "");
    _STATIC_FIELDS[49] = HeaderFieldStatic(Token.RANGE, "");
    _STATIC_FIELDS[50] = HeaderFieldStatic(Token.REFERER, "");
    _STATIC_FIELDS[51] = HeaderFieldStatic(Token.REFRESH, "");
    _STATIC_FIELDS[52] = HeaderFieldStatic(Token.RETRY_AFTER, "");
    _STATIC_FIELDS[53] = HeaderFieldStatic(Token.SERVER, "");
    _STATIC_FIELDS[54] = HeaderFieldStatic(Token.SET_COOKIE, "");
    _STATIC_FIELDS[55] = HeaderFieldStatic(Token.STRICT_TRANSPORT_SECURITY, "");
    _STATIC_FIELDS[56] = HeaderFieldStatic(Token.TRANSFER_ENCODING, "");
    _STATIC_FIELDS[57] = HeaderFieldStatic(Token.USER_AGENT, "");
    _STATIC_FIELDS[58] = HeaderFieldStatic(Token.VARY, "");
    _STATIC_FIELDS[59] = HeaderFieldStatic(Token.VIA, "");
    _STATIC_FIELDS[60] = HeaderFieldStatic(Token.WWW_AUTHENTICATE, "");

    foreach (i; 0 .. 61)
        STATIC_TABLE_PTRS[i] = &_STATIC_FIELDS[i];

    _hpack_static.init(STATIC_TABLE_PTRS[]);
}

// Shared StaticTableBase instance for HPACK (RFC 7541 table).
private __gshared StaticTableBase _hpack_static;

// Convenience accessor
ref StaticTableBase HPackStatic() { return _hpack_static; }

// HeaderTable — RFC 7541 unified index space
// PORT-NOTE: C++ class HPackTable → D class HPackTable (has behavior).
class HPackTable {
  public:
    this(size_t max_size = DEFAULT_MAX_TABLE_SIZE) {
        m_dyn = make!DynamicTable(max_size);
    }

    ~this() {
        dispose(m_dyn);
    }

    // PORT-NOTE: C++ operator[] returns optional<HeaderEntry> → D returns pointer (null = not found).
    const(HeaderFieldStatic)* at_static(size_t idx) const {
        if (idx == 0) return null;
        if (idx <= HPackStatic.size())
            return HPackStatic.at(idx - 1);
        return null;
    }

    // PORT-NOTE: Returns a DynamicTable deque entry; null if not found.
    const(io.codec.shared.table.DynamicTable.DequeEntry)* opIndex(size_t idx) const {
        if (idx == 0) return null;
        if (idx <= HPackStatic.size()) return null; // handled by at_static
        return null; // TODO: expose DynamicTable.at_position in improvement pass
    }

    // at: throws (C++) → returns null in D; callers must check.
    const(HeaderFieldStatic)* at(size_t idx) const {
        return at_static(idx);
    }

    SearchResult search(const(char)[] name, const(char)[] value) const {
        if (auto result = HPackStatic.search_full_match!(IndexCalculation.H_PACK)(name, value); result.found())
            return result;

        if (auto result = m_dyn.search_full_match!(IndexCalculation.H_PACK)(name, value); result.found())
            return SearchResult.make(result.index() + HPackStatic.size() + 1, true, true);

        if (auto result = HPackStatic.search_name_only!(IndexCalculation.H_PACK)(name); result.found())
            return result;

        if (auto result = m_dyn.search_name_only!(IndexCalculation.H_PACK)(name); result.found())
            return SearchResult.make(result.index() + HPackStatic.size() + 1, true, false);

        return SearchResult.none();
    }

    size_t insert(const(char)[] name, const(char)[] value) {
        return m_dyn.insert!(IndexCalculation.H_PACK)(name, value);
    }

    size_t insert(Token token, const(char)[] value) {
        return m_dyn.insert!(IndexCalculation.H_PACK)(token, value);
    }

    void set_max_size(size_t new_max) { m_dyn.set_max_size(new_max); }

    size_t max_size()      const pure { return m_dyn.get_max_size(); }
    size_t current_size()  const pure { return m_dyn.get_current_size(); }
    size_t dynamic_count() const pure { return m_dyn.get_size(); }
    size_t total_entries() const pure { return HPackStatic.size() + m_dyn.get_size(); }

  private:
    DynamicTable m_dyn;
}
