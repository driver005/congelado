module io.codec.qpack.table;
@nogc nothrow:

import util.alloc : make, dispose;

import io.codec.shared;
import io.shared.http.header;
import io.shared.http.types : Token;
import io.codec.qpack.types;

// PORT-NOTE: C++ inline const std::array<shared_ptr<HeaderField<true>>, 99> STATIC_TABLE
// → D static array of HeaderFieldStatic values (no heap allocation).
// Improvement idea: generate as CTFE.
private __gshared HeaderFieldStatic[99] _STATIC_FIELDS;
private __gshared const(HeaderFieldStatic)*[99] STATIC_TABLE_PTRS;

private shared static this() {
    _STATIC_FIELDS[ 0] = HeaderFieldStatic(Token.AUTHORITY, "");
    _STATIC_FIELDS[ 1] = HeaderFieldStatic(Token.PATH, "/");
    _STATIC_FIELDS[ 2] = HeaderFieldStatic(Token.AGE, "0");
    _STATIC_FIELDS[ 3] = HeaderFieldStatic(Token.CONTENT_DISPOSITION, "");
    _STATIC_FIELDS[ 4] = HeaderFieldStatic(Token.CONTENT_LENGTH, "0");
    _STATIC_FIELDS[ 5] = HeaderFieldStatic(Token.COOKIE, "");
    _STATIC_FIELDS[ 6] = HeaderFieldStatic(Token.DATE, "");
    _STATIC_FIELDS[ 7] = HeaderFieldStatic(Token.E_TAG, "");
    _STATIC_FIELDS[ 8] = HeaderFieldStatic(Token.IF_MODIFIED_SINCE, "");
    _STATIC_FIELDS[ 9] = HeaderFieldStatic(Token.IF_NONE_MATCH, "");
    _STATIC_FIELDS[10] = HeaderFieldStatic(Token.LAST_MODIFIED, "");
    _STATIC_FIELDS[11] = HeaderFieldStatic(Token.LINK, "");
    _STATIC_FIELDS[12] = HeaderFieldStatic(Token.LOCATION, "");
    _STATIC_FIELDS[13] = HeaderFieldStatic(Token.REFERER, "");
    _STATIC_FIELDS[14] = HeaderFieldStatic(Token.SET_COOKIE, "");
    _STATIC_FIELDS[15] = HeaderFieldStatic(Token.METHOD, "CONNECT");
    _STATIC_FIELDS[16] = HeaderFieldStatic(Token.METHOD, "DELETE");
    _STATIC_FIELDS[17] = HeaderFieldStatic(Token.METHOD, "GET");
    _STATIC_FIELDS[18] = HeaderFieldStatic(Token.METHOD, "HEAD");
    _STATIC_FIELDS[19] = HeaderFieldStatic(Token.METHOD, "OPTIONS");
    _STATIC_FIELDS[20] = HeaderFieldStatic(Token.METHOD, "POST");
    _STATIC_FIELDS[21] = HeaderFieldStatic(Token.METHOD, "PUT");
    _STATIC_FIELDS[22] = HeaderFieldStatic(Token.SCHEME, "http");
    _STATIC_FIELDS[23] = HeaderFieldStatic(Token.SCHEME, "https");
    _STATIC_FIELDS[24] = HeaderFieldStatic(Token.STATUS, "103");
    _STATIC_FIELDS[25] = HeaderFieldStatic(Token.STATUS, "200");
    _STATIC_FIELDS[26] = HeaderFieldStatic(Token.STATUS, "304");
    _STATIC_FIELDS[27] = HeaderFieldStatic(Token.STATUS, "404");
    _STATIC_FIELDS[28] = HeaderFieldStatic(Token.STATUS, "503");
    _STATIC_FIELDS[29] = HeaderFieldStatic(Token.ACCEPT, "*/*");
    _STATIC_FIELDS[30] = HeaderFieldStatic(Token.ACCEPT, "application/dns-message");
    _STATIC_FIELDS[31] = HeaderFieldStatic(Token.ACCEPT_ENCODING, "gzip, deflate, br");
    _STATIC_FIELDS[32] = HeaderFieldStatic(Token.ACCEPT_RANGES, "bytes");
    _STATIC_FIELDS[33] = HeaderFieldStatic(Token.ACCESS_CONTROL_ALLOW_HEADERS, "cache-control");
    _STATIC_FIELDS[34] = HeaderFieldStatic(Token.ACCESS_CONTROL_ALLOW_HEADERS, "content-type");
    _STATIC_FIELDS[35] = HeaderFieldStatic(Token.ACCESS_CONTROL_ALLOW_ORIGIN, "*");
    _STATIC_FIELDS[36] = HeaderFieldStatic(Token.CACHE_CONTROL, "max-age=0");
    _STATIC_FIELDS[37] = HeaderFieldStatic(Token.CACHE_CONTROL, "max-age=2592000");
    _STATIC_FIELDS[38] = HeaderFieldStatic(Token.CACHE_CONTROL, "max-age=604800");
    _STATIC_FIELDS[39] = HeaderFieldStatic(Token.CACHE_CONTROL, "no-cache");
    _STATIC_FIELDS[40] = HeaderFieldStatic(Token.CACHE_CONTROL, "no-store");
    _STATIC_FIELDS[41] = HeaderFieldStatic(Token.CACHE_CONTROL, "public, max-age=31536000");
    _STATIC_FIELDS[42] = HeaderFieldStatic(Token.CONTENT_ENCODING, "br");
    _STATIC_FIELDS[43] = HeaderFieldStatic(Token.CONTENT_ENCODING, "gzip");
    _STATIC_FIELDS[44] = HeaderFieldStatic(Token.CONTENT_TYPE, "application/dns-message");
    _STATIC_FIELDS[45] = HeaderFieldStatic(Token.CONTENT_TYPE, "application/javascript");
    _STATIC_FIELDS[46] = HeaderFieldStatic(Token.CONTENT_TYPE, "application/json");
    _STATIC_FIELDS[47] = HeaderFieldStatic(Token.CONTENT_TYPE, "application/x-www-form-urlencoded");
    _STATIC_FIELDS[48] = HeaderFieldStatic(Token.CONTENT_TYPE, "image/gif");
    _STATIC_FIELDS[49] = HeaderFieldStatic(Token.CONTENT_TYPE, "image/jpeg");
    _STATIC_FIELDS[50] = HeaderFieldStatic(Token.CONTENT_TYPE, "image/png");
    _STATIC_FIELDS[51] = HeaderFieldStatic(Token.CONTENT_TYPE, "text/css");
    _STATIC_FIELDS[52] = HeaderFieldStatic(Token.CONTENT_TYPE, "text/html; charset=utf-8");
    _STATIC_FIELDS[53] = HeaderFieldStatic(Token.CONTENT_TYPE, "text/plain");
    _STATIC_FIELDS[54] = HeaderFieldStatic(Token.CONTENT_TYPE, "text/plain;charset=utf-8");
    _STATIC_FIELDS[55] = HeaderFieldStatic(Token.RANGE, "bytes=0-");
    _STATIC_FIELDS[56] = HeaderFieldStatic(Token.STRICT_TRANSPORT_SECURITY, "max-age=31536000");
    _STATIC_FIELDS[57] = HeaderFieldStatic(Token.STRICT_TRANSPORT_SECURITY, "max-age=31536000; includesubdomains");
    _STATIC_FIELDS[58] = HeaderFieldStatic(Token.STRICT_TRANSPORT_SECURITY, "max-age=31536000; includesubdomains; preload");
    _STATIC_FIELDS[59] = HeaderFieldStatic(Token.VARY, "accept-encoding");
    _STATIC_FIELDS[60] = HeaderFieldStatic(Token.VARY, "origin");
    _STATIC_FIELDS[61] = HeaderFieldStatic(Token.X_CONTENT_TYPE_OPTIONS, "nosniff");
    _STATIC_FIELDS[62] = HeaderFieldStatic(Token.X_XSS_PROTECTION, "1; mode=block");
    _STATIC_FIELDS[63] = HeaderFieldStatic(Token.STATUS, "100");
    _STATIC_FIELDS[64] = HeaderFieldStatic(Token.STATUS, "204");
    _STATIC_FIELDS[65] = HeaderFieldStatic(Token.STATUS, "206");
    _STATIC_FIELDS[66] = HeaderFieldStatic(Token.STATUS, "302");
    _STATIC_FIELDS[67] = HeaderFieldStatic(Token.STATUS, "400");
    _STATIC_FIELDS[68] = HeaderFieldStatic(Token.STATUS, "403");
    _STATIC_FIELDS[69] = HeaderFieldStatic(Token.STATUS, "421");
    _STATIC_FIELDS[70] = HeaderFieldStatic(Token.STATUS, "425");
    _STATIC_FIELDS[71] = HeaderFieldStatic(Token.STATUS, "500");
    _STATIC_FIELDS[72] = HeaderFieldStatic(Token.ACCEPT_LANGUAGE, "");
    _STATIC_FIELDS[73] = HeaderFieldStatic(Token.ACCESS_CONTROL_ALLOW_CREDENTIALS, "FALSE");
    _STATIC_FIELDS[74] = HeaderFieldStatic(Token.ACCESS_CONTROL_ALLOW_CREDENTIALS, "TRUE");
    _STATIC_FIELDS[75] = HeaderFieldStatic(Token.ACCESS_CONTROL_ALLOW_HEADERS, "*");
    _STATIC_FIELDS[76] = HeaderFieldStatic(Token.ACCESS_CONTROL_ALLOW_METHODS, "get");
    _STATIC_FIELDS[77] = HeaderFieldStatic(Token.ACCESS_CONTROL_ALLOW_METHODS, "get, post, options");
    _STATIC_FIELDS[78] = HeaderFieldStatic(Token.ACCESS_CONTROL_ALLOW_METHODS, "options");
    _STATIC_FIELDS[79] = HeaderFieldStatic(Token.ACCESS_CONTROL_EXPOSE_HEADERS, "content-length");
    _STATIC_FIELDS[80] = HeaderFieldStatic(Token.ACCESS_CONTROL_REQUEST_HEADERS, "content-type");
    _STATIC_FIELDS[81] = HeaderFieldStatic(Token.ACCESS_CONTROL_REQUEST_METHOD, "get");
    _STATIC_FIELDS[82] = HeaderFieldStatic(Token.ACCESS_CONTROL_REQUEST_METHOD, "post");
    _STATIC_FIELDS[83] = HeaderFieldStatic(Token.ALT_SVC, "clear");
    _STATIC_FIELDS[84] = HeaderFieldStatic(Token.AUTHORIZATION, "");
    _STATIC_FIELDS[85] = HeaderFieldStatic(Token.CONTENT_SECURITY_POLICY,
                                           "script-src 'none'; object-src 'none'; base-uri 'none'");
    _STATIC_FIELDS[86] = HeaderFieldStatic(Token.EARLY_DATA, "1");
    _STATIC_FIELDS[87] = HeaderFieldStatic(Token.EXPECT_CT, "");
    _STATIC_FIELDS[88] = HeaderFieldStatic(Token.FORWARDED, "");
    _STATIC_FIELDS[89] = HeaderFieldStatic(Token.IF_RANGE, "");
    _STATIC_FIELDS[90] = HeaderFieldStatic(Token.ORIGIN, "");
    _STATIC_FIELDS[91] = HeaderFieldStatic(Token.PURPOSE, "prefetch");
    _STATIC_FIELDS[92] = HeaderFieldStatic(Token.SERVER, "");
    _STATIC_FIELDS[93] = HeaderFieldStatic(Token.TIMING_ALLOW_ORIGIN, "*");
    _STATIC_FIELDS[94] = HeaderFieldStatic(Token.UPGRADE_INSECURE_REQUESTS, "1");
    _STATIC_FIELDS[95] = HeaderFieldStatic(Token.USER_AGENT, "");
    _STATIC_FIELDS[96] = HeaderFieldStatic(Token.X_FORWARDED_FOR, "");
    _STATIC_FIELDS[97] = HeaderFieldStatic(Token.X_FRAME_OPTIONS, "deny");
    _STATIC_FIELDS[98] = HeaderFieldStatic(Token.X_FRAME_OPTIONS, "sameorigin");

    foreach (i; 0 .. 99)
        STATIC_TABLE_PTRS[i] = &_STATIC_FIELDS[i];

    _qpack_static.init(STATIC_TABLE_PTRS[]);
}

private __gshared StaticTableBase _qpack_static;

ref StaticTableBase QPackStatic() { return _qpack_static; }

// HeaderTable — RFC 9204 separate index spaces
// PORT-NOTE: C++ class QPackTable → D class QPackTable (has behavior).
class QPackTable {
  public:
    this(size_t max_capacity = 0) {
        m_dyn = make!DynamicTable(max_capacity);
    }

    ~this() {
        dispose(m_dyn);
    }

    const(HeaderFieldStatic)* at_static(size_t idx) const {
        return QPackStatic.at(idx);
    }

    // PORT-NOTE: C++ template<bool IsIndexPostBase, bool IsStatic> operator[] → D two methods.
    const(HeaderFieldStatic)* at_dynamic(bool is_post_base)(size_t idx, size_t base = 0) const {
        size_t abs_idx = 0;
        static if (is_post_base) {
            // Absolute = Base + 1 + Post-Base Index
            abs_idx = base + 1 + idx;
        } else {
            // Absolute = Base - Relative Index
            if (idx > base) return null;
            abs_idx = base - idx;
        }

        if (abs_idx >= m_dyn.get_size()) return null;

        // Your DynamicTable uses 1-based "generations"
        auto entry = m_dyn.at_generation(abs_idx + 1);
        if (entry is null) return null;
        if (entry.kind == HeaderEntryKind.Static) return entry.static_field;
        return null; // dynamic entries not returned as HeaderFieldStatic
    }

    SearchResult search(const(char)[] name, const(char)[] value) const {
        if (auto result = QPackStatic.search_full_match!(IndexCalculation.Q_PACK)(name, value); result.found())
            return result;
        if (auto result = m_dyn.search_full_match!(IndexCalculation.Q_PACK)(name, value); result.found())
            return result;
        if (auto result = QPackStatic.search_name_only!(IndexCalculation.Q_PACK)(name); result.found())
            return result;
        if (auto result = m_dyn.search_name_only!(IndexCalculation.Q_PACK)(name); result.found())
            return result;
        return SearchResult.none();
    }

    size_t encode_ric(size_t ric) const pure {
        if (ric == 0) return 0;
        const size_t MAX_ENTRIES = m_dyn.get_max_size() / 32;
        return (ric % (2 * MAX_ENTRIES)) + 1;
    }

    size_t decode_ric(size_t enc_ric) const pure {
        if (enc_ric == 0) return 0;
        const size_t MAX_ENTRIES = m_dyn.get_max_size() / 32;
        const size_t FULL_RANGE  = 2 * MAX_ENTRIES;

        // Total number of dynamic table inserts known to the decoder
        const size_t TOTAL_INST = m_dyn.get_insert_count();

        // Use the RFC algorithm to find the closest RIC to our current count
        size_t max_ric = TOTAL_INST + MAX_ENTRIES;
        size_t ric = ((max_ric / FULL_RANGE) * FULL_RANGE) + (enc_ric - 1);

        if (ric > max_ric && ric >= FULL_RANGE)
            ric -= FULL_RANGE;

        return ric;
    }

    bool is_ready(size_t ric) const pure { return ric <= m_dyn.get_insert_count(); }

    size_t insert(const(HeaderFieldStatic)* field) {
        const size_t GEN = m_dyn.insert(field.m_name, field.m_value);
        return GEN == 0 ? io.codec.shared.consts.SIZE_MAX : GEN - 1;
    }

    size_t insert(const(char)[] name, const(char)[] value) {
        const size_t GEN = m_dyn.insert(name, value);
        return GEN == 0 ? io.codec.shared.consts.SIZE_MAX : GEN - 1;
    }

    void set_max_size(size_t cap) { m_dyn.set_max_size(cap); }

    size_t insert_count()  const pure { return m_dyn.get_insert_count(); }
    size_t used()          const pure { return m_dyn.get_current_size(); }
    size_t dynamic_count() const pure { return m_dyn.get_size(); }
    size_t max_size()      const pure { return m_dyn.get_max_size(); }

  private:
    size_t abs_to_rel(size_t abs) const pure {
        const size_t IC = m_dyn.get_insert_count();
        if (abs >= IC) return io.codec.shared.consts.SIZE_MAX;
        return IC - 1 - abs;
    }

    size_t rel_to_abs(size_t rel) const pure {
        const size_t INC = m_dyn.get_insert_count();
        if (INC == 0 || rel >= INC) return io.codec.shared.consts.SIZE_MAX;
        return INC - 1 - rel;
    }

    // Helper for post-base indexing:
    // post-base: abs = base + 1 + pb
    static size_t post_base_to_absolut_index(size_t base, size_t post_base) pure {
        return base + 1 + post_base;
    }

    // [[nodiscard]] static std::size_t abs_to_post_base(std::size_t base, std::size_t abs) noexcept {
    //     if (abs <= base)
    //         return shared_codec::SIZE_MAX;
    //     return abs - base - 1;
    // }

    DynamicTable m_dyn;
}
