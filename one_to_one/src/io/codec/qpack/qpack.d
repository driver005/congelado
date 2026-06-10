module io.codec.qpack.qpack;
@nogc nothrow:

public import io.codec.qpack.types;
public import io.codec.qpack.table;

import io.codec.shared;
import io.codec.shared.atom : Atom;
import io.shared.http.header;
import io.shared.http.types : Token;

private static immutable const(char)[] COOKIE_SEPARATOR = "; ";

// QPack — RFC 9204 QPACK encoder/decoder.
// PORT-NOTE: C++ class template QPack<UInt, Width> → D class QPack!(UInt, Width).
class QPack(UInt = uint, int Width = 4) if (DecodeWidth!Width) {
  public:
    this(size_t table_size = 4096) {
        m_decoding_table        = new QPackTable(table_size);
        m_encoding_table        = new QPackTable(table_size);
        m_known_received_count  = 0;
        m_cookie_index          = null;
    }

    this(QPackTable* decoding_table, QPackTable* encoding_table) {
        m_decoding_table       = decoding_table;
        m_encoding_table       = encoding_table;
        m_known_received_count = 0;
        m_cookie_index         = null;
    }

    // Encode a complete field section (request/response stream).
    size_t encode(ref ubyte[] out_buf, ref size_t out_pos) {
        UInt required_insert_count = 0;

        // 1. First pass: Determine RIC based on the newest dynamic entry referenced
        foreach (ref field; m_table) {
            const auto result = m_encoding_table.search(field.m_name, field.m_value);
            if (result.found() && !result.is_static()) {
                // RIC must be at least the absolute index of the largest used entry
                const UInt idx = cast(UInt)result.index();
                if (idx > required_insert_count)
                    required_insert_count = idx;
            }
        }

        UInt base = required_insert_count;
        encode_field_section_prefix(required_insert_count, out_buf, out_pos);

        // 4. Second pass: Encode the actual instructions using the Base
        foreach (ref field; m_table) {
            const auto result = m_encoding_table.search(field.m_name, field.m_value);
            if (result.is_full_match())
                encode_indexed_field(cast(UInt)result.index(), result.is_static(), base, out_buf, out_pos);
            else if (result.found())
                encode_indexed_name(cast(UInt)result.index(), field.m_value, result.is_static(), false, base, out_buf, out_pos);
            else
                encode_new_field(field.m_name, field.m_value, false, out_buf, out_pos);
        }

        return out_pos;
    }

    // Decode a complete field section (request/response stream).
    bool decode(const(ubyte)[] data) {
        size_t pos = 0;
        UInt ric, base;
        if (!decode_field_section_prefix(data, pos, ric, base))
            return false;

        if (ric > m_decoding_table.insert_count())
            return false; // Stream blocked: Required Insert Count not yet reached

        while (pos < data.length) {
            auto rep_type = detect_representation_qpack_stream(data[pos]);
            switch (rep_type) {
            case PrefixHelper.HPACK_INDEXED_FIELD:
                if (!decode_indexed_field(data, pos, base)) return false;
                break;
            case PrefixHelper.QPACK_INDEXED_NAME:
                if (!decode_indexed_name(data, pos, base)) return false;
                break;
            case PrefixHelper.QPACK_NEW_FIELD:
                if (!decode_new_field(data, pos)) return false;
                break;
            case PrefixHelper.QPACK_POST_BASE_INDEXED_FIELD:
                if (!decode_post_base_indexed_field(data, pos, base)) return false;
                break;
            case PrefixHelper.QPACK_POST_BASE_INDEXED_NAME:
                if (!decode_post_base_indexed_name(data, pos, base)) return false;
                break;
            default:
                return false; // Unknown QPACK field line representation
            }
        }
        return true;
    }

    // Encode an encoder stream instruction sequence.
    size_t encode_encoder_stream(ref ubyte[] out_buf, ref size_t out_pos) {
        foreach (ref field; m_table) {
            const auto result = m_encoding_table.search(field.m_name, field.m_value);
            if (result.is_full_match())
                encode_duplicate(cast(UInt)result.index(), result.is_static(), 0, out_buf, out_pos);
            else if (result.found())
                encode_insert_with_indexed_name(cast(UInt)result.index(), result.is_static(),
                                                field.m_value, out_buf, out_pos);
            else
                encode_insert_with_literal_name(field.m_name, field.m_value, out_buf, out_pos);
        }
        return out_pos;
    }

    // Decode an encoder stream instruction sequence.
    bool decode_encoder_stream(const(ubyte)[] data) {
        size_t pos = 0;
        while (pos < data.length) {
            auto rep_type = detect_representation_qpack_encoder(data[pos]);
            switch (rep_type) {
            case PrefixHelper.QPACK_INSERT_INDEXED_NAME:
                if (!decode_inserted_with_indexed_name(data, pos)) return false;
                break;
            case PrefixHelper.QPACK_INSERT_LITERAL_NAME:
                if (!decode_inserted_with_literal_name(data, pos)) return false;
                break;
            case PrefixHelper.QPACK_DYNAMIC_TABLE_SIZE_UPDATE: {
                UInt new_size;
                if (!decode_table_size_update(data, pos, new_size)) return false;
                m_decoding_table.set_max_size(new_size);
                break;
            }
            case PrefixHelper.QPACK_DUPLICATE:
                if (!decode_duplicate(data, pos)) return false;
                break;
            default:
                return false; // Unknown QPACK encoder stream instruction
            }
        }
        return true;
    }

    // Decode a decoder stream instruction sequence.
    bool decode_decoder_stream(const(ubyte)[] data) {
        size_t pos = 0;
        while (pos < data.length) {
            auto rep_type = detect_representation_qpack_decoder(data[pos]);
            switch (rep_type) {
            case PrefixHelper.QPACK_DEC_ACK: {
                UInt stream_id;
                if (!decode_section_acknowledgment(data, pos, stream_id)) return false;
                (cast(void)stream_id); // TODO: track known received count per stream
                break;
            }
            case PrefixHelper.QPACK_DEC_STREAM_CANCELLATION: {
                UInt stream_id;
                if (!decode_stream_cancellation(data, pos, stream_id)) return false;
                (cast(void)stream_id); // TODO: free blocked stream state
                break;
            }
            case PrefixHelper.QPACK_DEC_INSERT_COUNT_INCREMENT: {
                UInt inc;
                if (!decode_insert_count_increment(data, pos, inc)) return false;
                m_known_received_count += inc;
                break;
            }
            default:
                return false; // Unknown QPACK decoder stream instruction
            }
        }
        return true;
    }

    bool encode_section_ack(UInt stream_id, ref ubyte[] out_buf, ref size_t out_pos) {
        return encode_section_acknowledgment(stream_id, out_buf, out_pos);
    }

    bool encode_stream_cancel(UInt stream_id, ref ubyte[] out_buf, ref size_t out_pos) {
        return encode_stream_cancellation(stream_id, out_buf, out_pos);
    }

    bool encode_insert_count_inc(UInt increment, ref ubyte[] out_buf, ref size_t out_pos) {
        return encode_insert_count_increment(increment, out_buf, out_pos);
    }

    void add_field(HeaderField* field) {
        m_table = m_table ~ DynEntry(false, field);
    }

    void set_table(DynEntry[] table) { m_table = table; }

  private:
    // PORT-NOTE: C++ vector<shared_ptr<HeaderField<>>> m_table → D array of DynEntry.
    struct DynEntry {
        // PORT-NOTE: value wrapper (struct), exempt from class-only rule
        bool          is_static;
        union {
            HeaderField*       dynamic_field;
            HeaderFieldStatic* static_field;
        }
        const(char)[] get_name_str() const pure {
            return is_static ? io.shared.http.types.token_to_string(static_field.m_name)
                             : dynamic_field.m_name;
        }
        const(char)[] get_value() const pure {
            return is_static ? static_field.m_value : dynamic_field.m_value;
        }
    }

    // Helper
    // The ToBeIndexed template parameter indicates whether the field being pushed should be added to the decoding table
    // (true for encoder stream instructions, false for request/response stream).
    bool push_helper(bool IsDecoder = true, bool IsIndexable = false, bool IsIndexPostBase = false)(
            UInt idx, const(char)[] value, bool is_static, size_t base) {
        if (idx == 0) return false;

        const(HeaderFieldStatic)* field = null;
        if (is_static) {
            static if (IsDecoder)
                field = m_decoding_table.at_static(idx);
            else
                field = m_encoding_table.at_static(idx);
        } else {
            static if (IsDecoder)
                field = m_decoding_table.at_dynamic!IsIndexPostBase(idx, base);
            else
                field = m_encoding_table.at_dynamic!IsIndexPostBase(idx, base);
        }

        if (field is null) return false;

        const(char)[] NAME = io.shared.http.types.token_to_string(field.m_name);

        static if (IsIndexable) {
            static if (IsDecoder)
                m_decoding_table.insert(NAME, value);
            else
                m_encoding_table.insert(NAME, value);
        } else {
            if (NAME == "cookie") {
                if (m_cookie_index !is null && !m_cookie_index.is_empty()) {
                    // append to existing cookie field
                    // PORT-NOTE: GC concatenation needed here in improvement pass
                } else {
                    auto cookie_field = new HeaderField();
                    cookie_field.m_name  = NAME;
                    cookie_field.m_value = value;
                    m_table = m_table ~ DynEntry(false, cookie_field);
                    m_cookie_index = cookie_field;
                }
            } else {
                auto f = new HeaderField();
                f.m_name  = NAME;
                f.m_value = value;
                m_table = m_table ~ DynEntry(false, f);
            }
        }
        return true;
    }

    bool push_helper_new_entry(bool IsDecoder = true, bool IsIndexable = false)(
            const(char)[] name, const(char)[] value) {
        if (name.length == 0) return false;

        static if (IsIndexable) {
            static if (IsDecoder)
                m_decoding_table.insert(name, value);
            else
                m_encoding_table.insert(name, value);
        } else {
            if (name == "cookie") {
                // PORT-NOTE: see push_helper cookie path
            } else {
                auto field = new HeaderField();
                field.m_name  = name;
                field.m_value = value;
                m_table = m_table ~ DynEntry(false, field);
            }
        }
        return true;
    }

    /* Field section prefix */

    void encode_field_section_prefix(UInt ric, ref ubyte[] out_buf, ref size_t out_pos) {
        UInt enc_ric = cast(UInt)m_encoding_table.encode_ric(ric);
        Atom!(UInt, Width).encode_int(enc_ric, 8U, cast(ubyte)0x00, out_buf, out_pos);

        // RFC 9204: Base = RIC + DeltaBase (Sign 0) OR Base = RIC - DeltaBase - 1 (Sign 1)
        UInt base = ric;
        if (base >= ric) {
            // Sign bit 0 (Positive or Zero Delta)
            Atom!(UInt, Width).encode_int(base - ric, 7U, cast(ubyte)0x00, out_buf, out_pos);
        } else {
            // Sign bit 1 (Negative Delta)
            Atom!(UInt, Width).encode_int(ric - base - 1, 7U, cast(ubyte)0x80, out_buf, out_pos);
        }
    }

    bool decode_field_section_prefix(const(ubyte)[] data, ref size_t pos, out UInt ric, out UInt base) {
        auto enc_ric_result = Atom!(UInt, Width).decode_int!0(data, pos, 8U);
        if (enc_ric_result.m_consumed == 0) return false;

        UInt req_insert_count = cast(UInt)m_decoding_table.decode_ric(enc_ric_result.m_value);

        if (pos >= data.length) return false;

        bool sign_bit = (data[pos] & 0x80) != 0;
        auto delta_result = Atom!(UInt, Width).decode_int!0(data, pos, 7U);
        if (delta_result.m_consumed == 0) return false;

        UInt delta_base = delta_result.m_value;

        if (!sign_bit) {
            base = req_insert_count + delta_base;
        } else {
            // Negative delta logic: Base = RIC - Delta - 1
            if (req_insert_count == 0 || delta_base >= req_insert_count)
                return false; // Negative Base calculation underflow
            base = req_insert_count - delta_base - 1;
        }

        ric = req_insert_count;
        return true;
    }

    /* Encoder / Decoder stream operations */

    // Encode

    // The pattern is 1Txxxxxx
    bool encode_insert_with_indexed_name(UInt idx, bool is_static, const(char)[] value,
                                         ref ubyte[] out_buf, ref size_t out_pos) {
        // Prefix is 1 (0x80), and T is the 7th bit (0x40)
        ubyte prefix = cast(ubyte)PrefixHelper.QPACK_INSERT_INDEXED_NAME;
        // Set the T-bit if it's a static entry.
        if (is_static) prefix |= 0x40;

        // We use a 5-bit prefix length because bits 3-7 are used for the integer value.
        Atom!(UInt, Width).encode_int(idx, 6U, prefix, out_buf, out_pos);
        // PORT-NOTE: encode_string for value
        // Atom!(UInt, Width).encode_string(null, value, out_buf, out_pos);

        push_helper!(false, true)(idx, value, is_static, 0);
        return true;
    }

    // The pattern is 01Hxxxxx
    bool encode_insert_with_literal_name(const(char)[] name, const(char)[] value,
                                         ref ubyte[] out_buf, ref size_t out_pos) {
        // Atom!(UInt, Width).encode_string(null, name, out_buf, out_pos, 6U);
        // Atom!(UInt, Width).encode_string(null, value, out_buf, out_pos);

        push_helper_new_entry!(false, true)(name, value);
        return true;
    }

    // The pattern is 000xxxxx
    bool encode_duplicate(UInt idx, bool is_static, size_t base, ref ubyte[] out_buf, ref size_t out_pos) {
        // We use a 5-bit prefix length because bits 3-7 are used for the integer value.
        Atom!(UInt, Width).encode_int(idx, 5U, cast(ubyte)PrefixHelper.QPACK_DUPLICATE, out_buf, out_pos);

        // m_encoding_table.insert(m_encoding_table.at(idx, false));
        return true;
    }

    // The pattern is 001xxxxx
    bool encode_table_size_update(UInt size, ref ubyte[] out_buf, ref size_t out_pos) {
        m_encoding_table.set_max_size(size);
        Atom!(UInt, Width).encode_int(
            size, 5U, cast(ubyte)PrefixHelper.QPACK_DYNAMIC_TABLE_SIZE_UPDATE, out_buf, out_pos);
        return true;
    }

    // Decode

    // The pattern is 1Txxxxxx
    bool decode_inserted_with_indexed_name(const(ubyte)[] data, ref size_t pos) {
        const auto IDX = Atom!(UInt, Width).decode_int!1(data, pos, 7U);
        if (IDX.m_consumed == 0) return false;
        // PORT-NOTE: VALUE decode omitted (Huffman path TBD)
        push_helper!(true, true)(IDX.m_value, "", IDX.is_static(), 0);
        return true;
    }

    // The pattern is 01Hxxxxx
    bool decode_inserted_with_literal_name(const(ubyte)[] data, ref size_t pos) {
        // PORT-NOTE: NAME/VALUE decode omitted (Huffman path TBD)
        push_helper_new_entry!(true, true)("", "");
        return true;
    }

    // The pattern is 000xxxxx
    bool decode_duplicate(const(ubyte)[] data, ref size_t pos) {
        const auto IDX = Atom!(UInt, Width).decode_int!0(data, pos, 5U);
        if (IDX.m_consumed == 0 || IDX.m_value == 0) return false;

        // m_decoding_table.insert(m_decoding_table.at(IDX, false));
        return true;
    }

    // The pattern is 001xxxxx
    bool decode_table_size_update(const(ubyte)[] data, ref size_t pos, out UInt new_size) {
        const auto NEW_SIZE = Atom!(UInt, Width).decode_int!0(data, pos, 5U);
        if (NEW_SIZE.m_consumed == 0) return false;
        // TODO: check the specs
        // if (new_size > m_decoding_table.max_size())
        //     return false; // TableSizeError

        m_decoding_table.set_max_size(NEW_SIZE.m_value);
        new_size = NEW_SIZE.m_value;
        return true;
    }

    /* Decoder Helper */

    // The pattern is  1xxxxxxx
    bool encode_section_acknowledgment(UInt size, ref ubyte[] out_buf, ref size_t out_pos) {
        // We use a 7-bit prefix length because the first bit is used for the prefix.
        Atom!(UInt, Width).encode_int(size, 7U, cast(ubyte)PrefixHelper.QPACK_DEC_ACK, out_buf, out_pos);
        return true;
    }

    // The pattern is  01xxxxxx
    bool encode_stream_cancellation(UInt size, ref ubyte[] out_buf, ref size_t out_pos) {
        // We use a 6-bit prefix length because the first bit is used for the prefix.
        Atom!(UInt, Width).encode_int(
            size, 6U, cast(ubyte)PrefixHelper.QPACK_DEC_STREAM_CANCELLATION, out_buf, out_pos);
        return true;
    }

    // The pattern is  00xxxxxx
    bool encode_insert_count_increment(UInt size, ref ubyte[] out_buf, ref size_t out_pos) {
        // We use a 6-bit prefix length because the first bit is used for the prefix.
        Atom!(UInt, Width).encode_int(
            size, 6U, cast(ubyte)PrefixHelper.QPACK_DEC_INSERT_COUNT_INCREMENT, out_buf, out_pos);
        return true;
    }

    // The pattern is 1xxxxxxx
    bool decode_section_acknowledgment(const(ubyte)[] data, ref size_t pos, out UInt stream_id) {
        const auto STREAM_ID = Atom!(UInt, Width).decode_int!0(data, pos, 7U).m_value;
        // PORT-NOTE: C++ std::print → @nogc; logging dropped here.
        // if (!m_blocked_streams.contains(stream_id))
        //     ...
        stream_id = STREAM_ID;
        return true;
    }

    // The pattern is  01xxxxxx
    bool decode_stream_cancellation(const(ubyte)[] data, ref size_t pos, out UInt stream_id) {
        const auto STREAM_ID = Atom!(UInt, Width).decode_int!0(data, pos, 6U).m_value;
        // PORT-NOTE: C++ std::print → @nogc; logging dropped here.
        // TODO: implement logic to free blocked stream state associated with this stream ID
        // m_blocked_streams.erase(stream_id);
        stream_id = STREAM_ID;
        return true;
    }

    // The pattern is  00xxxxxx
    bool decode_insert_count_increment(const(ubyte)[] data, ref size_t pos, out UInt inc) {
        const auto SIZE = Atom!(UInt, Width).decode_int!0(data, pos, 6U);
        if (SIZE.m_consumed == 0 || SIZE.m_value == 0) return false;

        UInt max_increment = cast(UInt)(m_decoding_table.insert_count() - m_known_received_count);
        if (SIZE.m_value > max_increment) return false; // Insert Count Increment exceeds unacknowledged insertion count

        m_known_received_count += SIZE.m_value;
        inc = SIZE.m_value;
        return true;
    }

    /* Request / Response stream! */

    // Encode

    // The pattern is 1Txxxxxx
    bool encode_indexed_field(UInt idx, bool is_static, size_t base,
                              ref ubyte[] out_buf, ref size_t out_pos) {
        // Prefix is 1 (0x80), and T is the 7th bit (0x40)
        ubyte prefix = cast(ubyte)PrefixHelper.HPACK_INDEXED_FIELD;
        // Set the T-bit if it's a static entry.
        if (is_static) prefix |= 0x40;

        // We use a 5-bit prefix length because bits 3-7 are used for the integer value.
        Atom!(UInt, Width).encode_int(idx, 6U, prefix, out_buf, out_pos);

        // add_field(m_encoding_table.at(idx.value()));
        return true;
    }

    // The pattern is 0001xxxx
    bool encode_post_base_indexed_field(UInt idx, ref ubyte[] out_buf, ref size_t out_pos) {
        // We use a 4-bit prefix length because bits 4-7 are used for the integer value.
        Atom!(UInt, Width).encode_int(
            idx, 4U, cast(ubyte)PrefixHelper.QPACK_POST_BASE_INDEXED_FIELD, out_buf, out_pos);

        // add_field(m_encoding_table.at!(true)(idx.value()));
        return true;
    }

    // The pattern is 01NTxxxx
    bool encode_indexed_name(UInt idx, const(char)[] value, bool is_static, bool is_never_indexed,
                             size_t base, ref ubyte[] out_buf, ref size_t out_pos) {
        // Prefix is 1 (0x40), and N is the 6th bit (0x20), and T is the 5th bit (0x10)
        ubyte prefix = cast(ubyte)PrefixHelper.QPACK_INDEXED_NAME;
        // Set the N-bit if the filed can never be indexd.
        if (is_never_indexed) prefix |= 0x20;
        // Set the T-bit if it's a static entry.
        if (is_static) prefix |= 0x10;

        // We use a 4-bit prefix length because bits 4-7 are used for the integer value.
        Atom!(UInt, Width).encode_int(idx, 4U, prefix, out_buf, out_pos);
        // Atom!(UInt, Width).encode_string(null, value, out_buf, out_pos);

        push_helper!(false, true)(idx, value, is_static, 0);
        return true;
    }

    // The pattern is 0000Nxxx
    bool encode_post_base_indexed_name(UInt idx, const(char)[] value, bool is_never_indexed,
                                       ref ubyte[] out_buf, ref size_t out_pos) {
        // Prefix is 4 bits (0x00), and N is the 4th bit (0x08)
        ubyte prefix = cast(ubyte)PrefixHelper.QPACK_POST_BASE_INDEXED_NAME;
        // Set the N-bit if the filed can never be indexd.
        if (is_never_indexed) prefix |= 0x08;

        // We use a 5-bit prefix length because bits 5-7 are used for the integer value.
        Atom!(UInt, Width).encode_int(idx, 3U, prefix, out_buf, out_pos);
        // Atom!(UInt, Width).encode_string(null, value, out_buf, out_pos);

        push_helper!(false, true, true)(idx, value, false, 0);
        return true;
    }

    // The pattern is 001NHxxx
    bool encode_new_field(const(char)[] name, const(char)[] value, bool is_never_indexed,
                          ref ubyte[] out_buf, ref size_t out_pos) {
        // Prefix is 1 (0x20), and N is the 5th bit (0x10)
        // ubyte prefix = cast(ubyte)PrefixHelper.QPACK_NEW_FIELD;
        // Set the N-bit if the filed can never be indexd.
        // if (is_never_indexed) prefix |= 0x10;

        // We use a 5-bit prefix length because bits 5-7 are used for the integer value.
        // Atom!(UInt, Width).encode_string(null, name, out_buf, out_pos, 3U);
        // Atom!(UInt, Width).encode_string(null, value, out_buf, out_pos);

        push_helper_new_entry!(false, true)(name, value);
        return true;
    }

    // Decode

    // The pattern is 1Txxxxxx
    bool decode_indexed_field(const(ubyte)[] data, ref size_t pos, size_t base) {
        const auto IDX = Atom!(UInt, Width).decode_int!1(data, pos, 6U);
        if (IDX.m_consumed == 0 || IDX.m_value == 0) return false;

        // add_field(m_decoding_table.at(IDX.value(), IDX.is_static(), base));
        return true;
    }

    // The pattern is 0001xxxx
    bool decode_post_base_indexed_field(const(ubyte)[] data, ref size_t pos, size_t base) {
        const auto IDX = Atom!(UInt, Width).decode_int!1(data, pos, 4U);
        if (IDX.m_consumed == 0 || IDX.m_value == 0) return false;

        // add_field(m_decoding_table.at!(true)(IDX.value(), IDX.is_static(), base));
        return true;
    }

    // The pattern is 01NTxxxx
    bool decode_indexed_name(const(ubyte)[] data, ref size_t pos, size_t base) {
        const auto IDX = Atom!(UInt, Width).decode_int!2(data, pos, 4U);
        if (IDX.m_consumed == 0) return false;
        // PORT-NOTE: VALUE decode omitted (Huffman path TBD)
        push_helper!( )(IDX.m_value, "", IDX.is_static(), base);
        return true;
    }

    // The pattern is 0000Nxxx
    bool decode_post_base_indexed_name(const(ubyte)[] data, ref size_t pos, size_t base) {
        const auto IDX = Atom!(UInt, Width).decode_int!1(data, pos, 3U);
        if (IDX.m_consumed == 0) return false;
        // PORT-NOTE: VALUE decode omitted (Huffman path TBD)
        push_helper!(true, false, true)(IDX.m_value, "", IDX.is_static(), base);
        return true;
    }

    // The pattern is 001NHxxx
    bool decode_new_field(const(ubyte)[] data, ref size_t pos) {
        // PORT-NOTE: NAME/VALUE decode omitted (Huffman path TBD)
        push_helper_new_entry!()("", "");
        return true;
    }


    QPackTable*   m_decoding_table;
    QPackTable*   m_encoding_table;
    DynEntry[]    m_table;
    UInt          m_known_received_count;
    HeaderField*  m_cookie_index;
}
