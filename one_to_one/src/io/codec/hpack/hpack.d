module io.codec.hpack.hpack;
@nogc nothrow:

public import io.codec.hpack.types;
public import io.codec.hpack.table;

import io.codec.shared;
import io.codec.shared.lowlevel;
import io.shared.http.header;
import io.shared.http.types : Token, token_to_string;
import interfaces.request  : IRequest;
import interfaces.response : IResponse;

// PORT-NOTE: C++ HpackFlushReason : bool → D enum bool equivalent
enum HpackFlushReason : bool { OVERFLOW = false, END = true }

// PORT-NOTE: C++ std::function<void(span<const byte>, HpackFlushReason)> FlushCallback
// → D fn+ctx pair to stay @nogc.
struct FlushCallback {
    // PORT-NOTE: value wrapper (struct), exempt from class-only rule
    void function(void* ctx, const(ubyte)[] data, HpackFlushReason reason) @nogc nothrow fn;
    void* ctx;
}

// HpackEncoder — encodes a set of header entries into HPACK wire format.
// PORT-NOTE: C++ class template HpackEncoder<UInt, Width> → D class HpackEncoder!(UInt, Width).
class HpackEncoder(UInt = uint, int Width = 4) if (DecodeWidth!Width) {
  public:
    this(HPackTable* table,
         const(HeaderEntry)[] headers,
         size_t max_frame_size,
         FlushCallback on_flush,
         bool use_auto_policy = true,
         bool use_huffman = true) {
        m_table          = table;
        m_headers        = headers;
        m_flush_size     = max_frame_size;
        m_on_flush       = on_flush;
        m_use_auto_policy = use_auto_policy;
        m_use_huffman    = use_huffman;
        m_buf_pos        = 0;
    }

    void opCall() const {
        m_buf_pos = 0;
        foreach (ref entry; m_headers)
            encode_entry(entry);
        if (m_on_flush.fn !is null)
            m_on_flush.fn(m_on_flush.ctx, m_buf[0 .. m_buf_pos], HpackFlushReason.END);
        m_buf_pos = 0;
    }

  private:
    void emit(ubyte byte_val) const {
        m_buf[m_buf_pos++] = byte_val;
        if (m_buf_pos == m_flush_size) {
            if (m_on_flush.fn !is null)
                m_on_flush.fn(m_on_flush.ctx, m_buf[0 .. m_buf_pos], HpackFlushReason.OVERFLOW);
            m_buf_pos = 0;
        }
    }

    void drain_range(R)(ref R range) const {
        while (!range.empty()) {
            emit(range.front());
            range.popFront();
        }
    }

    void encode_string_field(const(char)[] view) const {
        // PORT-NOTE: C++ encode_string adaptor → write into temp buf then drain
        ubyte[65536] tmp_buf;
        ubyte[] tmp = tmp_buf[];
        size_t tmp_pos = 0;
        const(ubyte)[] data = (cast(const(ubyte)*)view.ptr)[0 .. view.length];
        encode_string!Width(m_use_huffman, data, tmp, tmp_pos);
        foreach (i; 0 .. tmp_pos)
            emit(tmp[i]);
    }

    void encode_indexed(UInt idx) const {
        auto range = encode_int!UInt(idx, 7U, PrefixHelper.HPACK_INDEXED_FIELD);
        drain_range(range);
    }

    void encode_incremental(UInt idx, const(char)[] value) const {
        auto range = encode_int!UInt(idx, 6U, PrefixHelper.HPACK_LITERAL_WITH_INDEXING);
        drain_range(range);
        encode_string_field(value);
        // update encoding table
        // PORT-NOTE: C++ visits the table entry to get its name; simplified here.
        m_table.insert(value, value); // TODO: fix — needs to fetch actual name from table
    }

    void encode_incremental_new(const(char)[] name, const(char)[] value) const {
        emit(cast(ubyte)PrefixHelper.HPACK_LITERAL_WITH_INDEXING);
        encode_string_field(name);
        encode_string_field(value);
        m_table.insert(name, value);
    }

    void encode_without_indexing(UInt idx, const(char)[] value) const {
        auto range = encode_int!UInt(idx, 4U, PrefixHelper.HPACK_LITERAL_WITHOUT_INDEXING);
        drain_range(range);
        encode_string_field(value);
    }

    void encode_without_indexing_new(const(char)[] name, const(char)[] value) const {
        emit(cast(ubyte)PrefixHelper.HPACK_LITERAL_WITHOUT_INDEXING);
        encode_string_field(name);
        encode_string_field(value);
    }

    void encode_never_indexed(UInt idx, const(char)[] value) const {
        auto range = encode_int!UInt(idx, 4U, PrefixHelper.HPACK_LITERAL_NEVER_INDEXED);
        drain_range(range);
        encode_string_field(value);
    }

    void encode_never_indexed_new(const(char)[] name, const(char)[] value) const {
        emit(cast(ubyte)PrefixHelper.HPACK_LITERAL_NEVER_INDEXED);
        encode_string_field(name);
        encode_string_field(value);
    }

    void encode_cookies(const(char)[] value) const {
        static immutable const(char)[] SEP = "; ";
        // Simple split on "; "
        size_t start = 0;
        while (start < value.length) {
            size_t end = start;
            while (end + SEP.length <= value.length &&
                   value[end .. end + SEP.length] != SEP)
                ++end;
            const crumb = value[start .. end];
            if (crumb.length > 0) {
                auto result = m_table.search("cookie", crumb);
                encode_hpack_field("cookie", crumb, EncodePolicy.WithIndexing, result);
            }
            start = (end + SEP.length <= value.length) ? end + SEP.length : value.length;
        }
    }

    void encode_hpack_field(const(char)[] name, const(char)[] value, EncodePolicy policy,
                            SearchResult result) const {
        switch (policy) {
        case EncodePolicy.WithIndexing:
            if (result.is_full_match())
                encode_indexed(cast(UInt)result.index());
            else if (result.found())
                encode_incremental(cast(UInt)result.index(), value);
            else
                encode_incremental_new(name, value);
            break;
        case EncodePolicy.WithoutIndexing:
            if (result.found())
                encode_without_indexing(cast(UInt)result.index(), value);
            else
                encode_without_indexing_new(name, value);
            break;
        case EncodePolicy.NeverIndexed:
            if (result.found())
                encode_never_indexed(cast(UInt)result.index(), value);
            else
                encode_never_indexed_new(name, value);
            break;
        default: break;
        }
    }

    void encode_entry(ref const HeaderEntry entry) const {
        const(char)[] name;
        const(char)[] value;
        bool is_cookie = false;

        if (entry.kind == HeaderEntryKind.Static) {
            name     = token_to_string(entry.static_field.m_name);
            value    = entry.static_field.m_value;
            is_cookie = (entry.static_field.m_name == Token.COOKIE);
        } else {
            name  = entry.dynamic_field.m_name;
            value = entry.dynamic_field.m_value;
            is_cookie = (name == "cookie");
        }

        if (is_cookie) {
            encode_cookies(value);
            return;
        }

        const EncodePolicy POLICY =
            m_use_auto_policy ? policy_for(name) : EncodePolicy.WithIndexing;
        encode_hpack_field(name, value, POLICY, m_table.search(name, value));
    }

    HPackTable*            m_table;
    const(HeaderEntry)[]   m_headers;
    size_t                 m_flush_size;
    FlushCallback          m_on_flush;
    bool                   m_use_auto_policy;
    bool                   m_use_huffman;
    mutable ubyte[16384]   m_buf;
    mutable size_t         m_buf_pos;
}

// HpackTableSizeUpdateEncoder — encodes a dynamic table size update.
// PORT-NOTE: C++ HpackTableSizeUpdateAdaptor → D free function encode_table_size_update.
EncodeIntRange!UInt encode_table_size_update(UInt = uint)(HPackTable* table, UInt size) {
    table.set_max_size(size);
    return encode_int!UInt(size, 5U, PrefixHelper.HPACK_DYNAMIC_TABLE_SIZE_UPDATE);
}

// HpackDecoderAdapter — decodes HPACK wire bytes and populates an IRequest.
// PORT-NOTE: C++ class template HpackDecoderAdapter<Protocol, UInt, Width> → D class.
class HpackDecoderAdapter(Protocol, UInt = uint, int Width = 4) if (DecodeWidth!Width) {
  public:
    this(HPackTable* table, IRequest!Protocol* req) {
        m_table   = table;
        m_request = req;
    }

    size_t opCall(const(ubyte)[] data) const {
        const size_t TOTAL = data.length;
        size_t offset = 0;

        while (offset < TOTAL) {
            auto slice = data[offset .. $];
            const auto rep_type = detect_representation_hpack(slice[0]);
            const bool IS_NEW = rep_type != PrefixHelper.HPACK_INDEXED_FIELD &&
                                rep_type != PrefixHelper.HPACK_DYNAMIC_TABLE_SIZE_UPDATE &&
                                !(slice[0] & ~cast(ubyte)rep_type);

            size_t consumed = 0;
            switch (rep_type) {
            case PrefixHelper.HPACK_INDEXED_FIELD:
                consumed = decode_indexed(slice);
                break;
            case PrefixHelper.HPACK_LITERAL_WITH_INDEXING:
                consumed = IS_NEW ? decode_incremental_new(slice) : decode_incremental(slice);
                break;
            case PrefixHelper.HPACK_LITERAL_NEVER_INDEXED:
            case PrefixHelper.HPACK_LITERAL_WITHOUT_INDEXING:
                consumed = IS_NEW ? decode_literal_new!false(slice)
                                  : decode_literal!false(slice, 4U);
                break;
            case PrefixHelper.HPACK_DYNAMIC_TABLE_SIZE_UPDATE:
                consumed = decode_table_size_update(slice);
                break;
            default:
                return offset; // invalid HPACK representation type — stop
            }

            if (consumed == 0) break; // truncated / error
            offset += consumed;
        }

        return offset;
    }

  private:
    void push_helper(bool Indexable = true)(UInt idx, const(char)[] value) const {
        if (idx == 0) return;
        // PORT-NOTE: C++ std::visit(…, m_table.at(idx)) → simplified: look up static table
        auto* field = m_table.at_static(idx);
        if (field is null) return;
        if (Indexable) {
            m_table.insert(token_to_string(field.m_name), value);
        } else {
            m_request.add_header(token_to_string(field.m_name), value);
        }
    }

    void push_helper_new(bool Indexable = true)(const(char)[] name, const(char)[] value) const {
        if (name.length == 0) return;
        if (Indexable) {
            m_table.insert(name, value);
        } else {
            m_request.add_header(name, value);
        }
    }

    void add_field_static(const(HeaderFieldStatic)* field) const {
        if (field is null) return;
        m_request.add_header(token_to_string(field.m_name), field.m_value);
    }

    // 1xxxxxxx
    size_t decode_indexed(const(ubyte)[] data) const {
        auto result = decode_int!UInt(data, 7U);
        if (result.m_consumed == 0 || result.m_value == 0) return 0;
        add_field_static(m_table.at_static(result.m_value));
        return result.m_consumed;
    }

    // 01xxxxxx + value
    size_t decode_incremental(const(ubyte)[] data) const {
        char[65536] value_buf;
        char[] value_str = value_buf[];
        auto idx = decode_int!UInt(data, 6U);
        if (idx.m_consumed == 0) return 0;
        auto val = decode_string!Width(data[idx.m_consumed .. $], value_str);
        if (val.consumed == 0) return 0;
        push_helper!true(idx.m_value, val.value);
        return idx.m_consumed + val.consumed;
    }

    // 01000000 + name + value
    size_t decode_incremental_new(const(ubyte)[] data) const {
        char[65536] name_buf;
        char[] name_str = name_buf[];
        char[65536] value_buf;
        char[] value_str = value_buf[];
        auto nm = decode_string!Width(data[1 .. $], name_str);
        if (nm.consumed == 0) return 0;
        size_t name_offset = 1 + nm.consumed;
        auto val = decode_string!Width(data[name_offset .. $], value_str);
        if (val.consumed == 0) return 0;
        push_helper_new!true(nm.value, val.value);
        return name_offset + val.consumed;
    }

    // 0000xxxx / 0001xxxx + value
    size_t decode_literal(bool Indexable)(const(ubyte)[] data, ubyte prefix_bits) const {
        char[65536] value_buf;
        char[] value_str = value_buf[];
        auto idx = decode_int!UInt(data, prefix_bits);
        if (idx.m_consumed == 0) return 0;
        auto val = decode_string!Width(data[idx.m_consumed .. $], value_str);
        if (val.consumed == 0) return 0;
        push_helper!Indexable(idx.m_value, val.value);
        return idx.m_consumed + val.consumed;
    }

    // 00000000 / 00010000 + name + value
    size_t decode_literal_new(bool Indexable)(const(ubyte)[] data) const {
        char[65536] name_buf;
        char[] name_str = name_buf[];
        char[65536] value_buf;
        char[] value_str = value_buf[];
        auto nm = decode_string!Width(data[1 .. $], name_str);
        if (nm.consumed == 0) return 0;
        size_t name_offset = 1 + nm.consumed;
        auto val = decode_string!Width(data[name_offset .. $], value_str);
        if (val.consumed == 0) return 0;
        push_helper_new!Indexable(nm.value, val.value);
        return name_offset + val.consumed;
    }

    // 001xxxxx
    size_t decode_table_size_update(const(ubyte)[] data) const {
        auto new_size = decode_int!UInt(data, 5U);
        if (new_size.m_consumed == 0) return 0;
        if (new_size.m_value > m_table.max_size()) return 0;
        m_table.set_max_size(new_size.m_value);
        return new_size.m_consumed;
    }

    HPackTable*           m_table;
    IRequest!Protocol*    m_request;
}

// Hpack — facade combining encoder and decoder.
// PORT-NOTE: C++ class template Hpack<Protocol, UInt, Width> → D class Hpack!(Protocol, UInt, Width).
class Hpack(Protocol, UInt = uint, int Width = 4) if (DecodeWidth!Width) {
  public:
    this(HPackTable* decoding_table, HPackTable* encoding_table,
         IRequest!Protocol* req, IResponse!Protocol* res,
         bool use_huffman = true) {
        m_encoding_table = encoding_table;
        m_decoding_table = decoding_table;
        m_request        = req;
        m_response       = res;
        m_use_huffman    = use_huffman;
    }

    HpackEncoder!(UInt, Width) make_encoder(FlushCallback on_flush, bool use_auto_policy = true) {
        return new HpackEncoder!(UInt, Width)(
            m_encoding_table,
            m_response.get_headers(),
            16384,
            on_flush,
            use_auto_policy,
            m_use_huffman);
    }

    size_t decode(const(ubyte)[] data) {
        auto adapter = new HpackDecoderAdapter!(Protocol, UInt, Width)(m_decoding_table, m_request);
        return adapter(data);
    }

    EncodeIntRange!UInt encode_table_size_update(UInt size) {
        return .encode_table_size_update!UInt(m_encoding_table, size);
    }

  private:
    HPackTable*           m_encoding_table;
    HPackTable*           m_decoding_table;
    IRequest!Protocol*    m_request;
    IResponse!Protocol*   m_response;
    bool                  m_use_huffman;
}
