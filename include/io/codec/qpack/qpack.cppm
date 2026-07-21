export module io_codec_qpack;

export import :types;
export import :table;

import io_codec_shared;
import io_error;
import io_shared;

static constexpr std::string COOKIE_SEPARATOR = "; ";

export namespace io::codec::qpack {

template <std::unsigned_integral UInt = std::uint32_t, int Width = 4>
    requires shared_codec::DecodeWidth<Width>
class QPack {
  public:
    /**
     * @brief Spins up a QPACK codec with fresh, independently-sized encoding and decoding
     * dynamic tables — each direction gets its own table since QPACK, like HPACK, tracks
     * send/receive state separately.
     * @param table_size the byte budget for both the encoding and decoding dynamic tables,
     * defaults to 4096.
     */
    explicit QPack(std::size_t table_size = 4096)
        : m_decoding_table{std::make_shared<QPackTable>(table_size)},
          m_encoding_table{std::make_shared<QPackTable>(table_size)}, m_huffman{}, m_known_received_count{0},
          m_cookie_index{nullptr} {}

    /**
     * @brief Spins up a QPACK codec over pre-built dynamic tables — useful when the tables need
     * to be shared or pre-configured outside this codec's construction.
     * @param decoding_table the dynamic table to use for decoding inbound headers, moved in.
     * @param encoding_table the dynamic table to use for encoding outbound headers, moved in.
     */
    explicit QPack(std::shared_ptr<QPackTable> decoding_table, std::shared_ptr<QPackTable> encoding_table)
        : m_decoding_table(std::move(decoding_table)), m_encoding_table(std::move(encoding_table)), m_huffman{},
          m_known_received_count{0}, m_cookie_index{nullptr} {}


    // Encode a complete field section (request/response stream).
    /**
     * @brief Encodes `m_table`'s headers as a complete QPACK field section (RFC 9204 §4.5) —
     * two passes over the fields: first to compute the Required Insert Count from whatever
     * dynamic-table entries got referenced, then to actually emit each field against that fixed
     * Base. Two-pass is load-bearing here — the Base has to be known before any field line gets
     * written, since relative/post-base indices are all computed against it.
     * @tparam Out an output iterator accepting std::uint8_t.
     * @param out the output iterator encoded bytes get written to.
     */
    template <std::output_iterator<std::uint8_t> Out>
    void encode(Out out) {
        UInt required_insert_count = 0;

        // 1. First pass: Determine RIC based on the newest dynamic entry referenced
        for (const auto &field : m_table) {
            shared_codec::SearchResult result = m_encoding_table->search(field->get_name(), field->get_value());

            if (result.found() && !result.is_static()) {
                // RIC must be at least the absolute index of the largest used entry
                required_insert_count = std::max(required_insert_count, static_cast<UInt>(result.index()));
            }
        }

        // Base gets fixed to the RIC from the first pass — every relative/post-base
        // index below is computed against this exact value.
        UInt base = required_insert_count;

        encode_field_section_prefix(required_insert_count, out);

        // 4. Second pass: Encode the actual instructions using the Base
        for (const auto &field : m_table) {
            shared_codec::SearchResult result = m_encoding_table->search(field->get_name(), field->get_value());

            // Same three-way decision as HPACK: full match goes indexed, name-only match
            // goes literal-with-indexed-name, no match goes full fresh literal.
            if (result.is_full_match()) {
                encode_indexed_field(static_cast<UInt>(result.index()), result.is_static(), base, out);
            } else if (result.found()) {
                encode_indexed_name(static_cast<UInt>(result.index()), field->get_value(), result.is_static(), false,
                                    base, out);
            } else {
                encode_new_field(field->get_name(), field->get_value(), false, out);
            }
        }
    }

    // Decode a complete field section (request/response stream).
    /**
     * @brief Decodes a complete QPACK field section — reads the Required Insert Count and Base
     * from the prefix, blocks (throws) if the decoder hasn't caught up to the dynamic table
     * state the encoder assumed, then walks each field line dispatching by representation type.
     * Getting the blocking check wrong means reading dynamic-table slots that don't exist yet —
     * straight UB territory, not just a wrong answer.
     * @param data the encoded field section bytes.
     * @throws error::http::CompressionError if the Required Insert Count hasn't been reached
     * yet (stream should be treated as blocked, not decoded) or if a field line has an unknown
     * representation type.
     */
    void decode(std::span<const std::uint8_t> data) {
        std::size_t pos = 0;
        // Prefix first — need RIC and Base before any field line can be interpreted.
        auto [ric, base] = decode_field_section_prefix(data, pos);

        // Blocking check — can't safely resolve dynamic-table references the encoder
        // assumed if this side hasn't caught up to that many inserts yet.
        if (ric > m_decoding_table->insert_count()) {
            throw error::http::CompressionError{"Stream blocked: Required Insert Count not yet reached"};
        }

        // Walk field lines one at a time, dispatching on each line's representation type.
        while (pos < data.size()) {
            auto rep_type = shared_codec::detect_representation_qpack_stream(data[pos]);  // FIXME(clang-tidy): unchecked operator[], consider .at()

            switch (rep_type) {
            case shared_codec::PrefixHelper::HPACK_INDEXED_FIELD:
                decode_indexed_field(data, pos, base);
                break;
            case shared_codec::PrefixHelper::QPACK_INDEXED_NAME:
                decode_indexed_name(data, pos, base);
                break;
            case shared_codec::PrefixHelper::QPACK_NEW_FIELD:
                decode_new_field(data, pos);
                break;
            case shared_codec::PrefixHelper::QPACK_POST_BASE_INDEXED_FIELD:
                decode_post_base_indexed_field(data, pos, base);
                break;
            case shared_codec::PrefixHelper::QPACK_POST_BASE_INDEXED_NAME:
                decode_post_base_indexed_name(data, pos, base);
                break;
            default:
                // Line's high bits didn't match any known field line type.
                throw error::http::CompressionError{"Unknown QPACK field line representation"};
            }
        }
    }

    // Encode an encoder stream instruction sequence.
    /**
     * @brief Encodes `m_table`'s fields as encoder stream instructions (RFC 9204 §4.3) — these
     * populate the peer's dynamic table ahead of (or independent from) any field section, via
     * duplicate/insert-with-indexed-name/insert-with-literal-name instructions depending on
     * what's already indexed.
     * @tparam Out an output iterator accepting std::uint8_t.
     * @param out the output iterator encoded instruction bytes get written to.
     */
    template <std::output_iterator<std::uint8_t> Out>
    void encode_encoder_stream(Out out) {
        // Every field gets its own instruction, picked by how much of it's already
        // sitting in the table — full match just duplicates the existing entry, a
        // name-only match reuses the name and literals the value, and a total miss
        // ships both as fresh literals.
        for (const auto &field : m_table) {
            shared_codec::SearchResult result = m_encoding_table->search(field->get_name(), field->get_value());

            if (result.is_full_match()) {
                encode_duplicate(static_cast<UInt>(result.index()), result.is_static(), 0, out);
            } else if (result.found()) {
                encode_insert_with_indexed_name(static_cast<UInt>(result.index()), result.is_static(),
                                                field->get_value(), out);
            } else {
                encode_insert_with_literal_name(field->get_name(), field->get_value(), out);
            }
        }
    }

    // Decode an encoder stream instruction sequence.
    /**
     * @brief Decodes a sequence of encoder stream instructions, mutating the decoding table as
     * inserts/duplicates/size-updates come in — this is how the decoder's dynamic table stays
     * in sync with what the encoder has been indexing.
     * @param data the encoded instruction bytes.
     * @throws error::http::CompressionError if an instruction byte doesn't match any known
     * encoder stream representation type.
     */
    void decode_encoder_stream(std::span<const std::uint8_t> data) {
        std::size_t pos = 0;

        // Instructions keep coming until the buffer's drained, each one mutating the
        // decoding table so it tracks whatever the encoder's been indexing.
        while (pos < data.size()) {
            auto rep_type = shared_codec::detect_representation_qpack_encoder(data[pos]);  // FIXME(clang-tidy): unchecked operator[], consider .at()

            switch (rep_type) {
            case shared_codec::PrefixHelper::QPACK_INSERT_INDEXED_NAME:
                decode_inserted_with_indexed_name(data, pos);
                break;
            case shared_codec::PrefixHelper::QPACK_INSERT_LITERAL_NAME:
                decode_inserted_with_literal_name(data, pos);
                break;
            case shared_codec::PrefixHelper::QPACK_DYNAMIC_TABLE_SIZE_UPDATE: {
                // Capacity change — resize right away so later instructions in this
                // same batch see the updated budget.
                UInt new_size = decode_table_size_update(data, pos);
                m_decoding_table->set_max_size(new_size);
                break;
            }
            case shared_codec::PrefixHelper::QPACK_DUPLICATE:
                decode_duplicate(data, pos);
                break;
            default:
                throw error::http::CompressionError{"Unknown QPACK encoder stream instruction"};
            }
        }
    }

    // Decode a decoder stream instruction sequence.
    /**
     * @brief Decodes a sequence of decoder stream instructions coming back from the peer —
     * Section Acknowledgment and Stream Cancellation are currently no-ops (TODOs, tracked
     * inline), but Insert Count Increment does drive `m_known_received_count`, which is what
     * downstream Required Insert Count math depends on.
     * @param data the encoded instruction bytes.
     * @throws error::http::CompressionError if an instruction byte doesn't match any known
     * decoder stream representation type.
     */
    void decode_decoder_stream(std::span<const std::uint8_t> data) {
        std::size_t pos = 0;

        // Same drain-until-empty shape as the encoder stream, but for instructions
        // flowing back from the peer about what it's received/processed.
        while (pos < data.size()) {
            auto rep_type = shared_codec::detect_representation_qpack_decoder(data[pos]);  // FIXME(clang-tidy): unchecked operator[], consider .at()

            switch (rep_type) {
            case shared_codec::PrefixHelper::QPACK_DEC_ACK: {
                UInt stream_id = decode_section_acknowledgment(data, pos);
                (void)stream_id; // TODO: track known received count per stream
                break;
            }
            case shared_codec::PrefixHelper::QPACK_DEC_STREAM_CANCELLATION: {
                UInt stream_id = decode_stream_cancellation(data, pos);
                (void)stream_id; // TODO: free blocked stream state
                break;
            }
            case shared_codec::PrefixHelper::QPACK_DEC_INSERT_COUNT_INCREMENT:
                m_known_received_count += decode_insert_count_increment(data, pos);
                break;
            default:
                throw error::http::CompressionError{"Unknown QPACK decoder stream instruction"};
            }
        }
    }
    /**
     * @brief Public-facing wrapper for emitting a Section Acknowledgment decoder stream
     * instruction — thin forward to encode_section_acknowledgment().
     * @tparam Out an output iterator accepting std::uint8_t.
     * @param stream_id the ID of the stream whose field section is being acknowledged.
     * @param out the output iterator the instruction bytes get written to.
     */
    template <std::output_iterator<std::uint8_t> Out>
    void encode_section_ack(UInt stream_id, Out out) {
        encode_section_acknowledgment(stream_id, out);
    }

    /**
     * @brief Public-facing wrapper for emitting a Stream Cancellation decoder stream
     * instruction — thin forward to encode_stream_cancellation().
     * @tparam Out an output iterator accepting std::uint8_t.
     * @param stream_id the ID of the stream being cancelled.
     * @param out the output iterator the instruction bytes get written to.
     */
    template <std::output_iterator<std::uint8_t> Out>
    void encode_stream_cancel(UInt stream_id, Out out) {
        encode_stream_cancellation(stream_id, out);
    }

    /**
     * @brief Public-facing wrapper for emitting an Insert Count Increment decoder stream
     * instruction — thin forward to encode_insert_count_increment().
     * @tparam Out an output iterator accepting std::uint8_t.
     * @param increment how much to increment the peer's known-received-count by.
     * @param out the output iterator the instruction bytes get written to.
     */
    template <std::output_iterator<std::uint8_t> Out>
    void encode_insert_count_inc(UInt increment, Out out) {
        encode_insert_count_increment(increment, out);
    }

    /**
     * @brief Appends a single field to the pending field-section header list — lowkey the
     * simplest setter in this whole class.
     * @param field the header field to append.
     */
    void add_field(const std::shared_ptr<interfaces::io::HeaderField<>> &field) { m_table.push_back(field); }

    /**
     * @brief Replaces the entire pending field-section header list wholesale.
     * @param table the header fields to encode on the next encode() call, copied in.
     */
    void set_table(const std::vector<std::shared_ptr<interfaces::io::HeaderField<>>> &table) { m_table = table; }

  private:
    // Helper

    // The ToBeIndexed template parameter indicates whether the field being pushed should be added to the decoding table
    // (true for encoder stream instructions, false for request/response stream).
    /**
     * @brief Resolves a name-indexed field to its literal value, either inserting it into the
     * relevant dynamic table (encoder stream path) or appending it to `m_table` as a decoded
     * header — with a special case that folds repeated `cookie` fields into one entry joined by
     * `"; "` instead of pushing duplicates, mirroring HPACK's cookie-splitting in reverse.
     * @tparam IsDecoder when true, operates on `m_decoding_table`; when false,
     * `m_encoding_table`.
     * @tparam IsIndexable when true, the resolved pair gets inserted into the dynamic table
     * instead of appended to `m_table` — this is the encoder-stream-instruction path.
     * @tparam IsIndexPostBase forwarded to the table lookup to select post-base vs. relative
     * index interpretation.
     * @param idx the index to resolve; 0 is never valid.
     * @param value the decoded header value.
     * @param is_static whether `idx` addresses the static or dynamic table.
     * @param base the field section's Base value, used for dynamic lookups.
     * @throws error::http::InvalidIndexError if `idx` is 0.
     * @warning The non-indexable/cookie branch dereferences `m_cookie_index` (`->is_empty()`)
     * before ever checking it against nullptr. `m_cookie_index` starts life null and only gets
     * assigned once a cookie has actually been pushed — so the very first `cookie` field seen on
     * this instance is straight cooked: null pointer deref, no safety net. Not something this
     * doc pass fixes, just flagging it so nobody gets caught slipping.
     */
    template <bool IsDecoder = true, bool IsIndexable = false, bool IsIndexPostBase = false>
    void push_helper(UInt idx, std::string_view value, bool is_static, std::size_t base) {
        // 0 is never a valid wire index — bail before touching either table.
        if (idx == 0) {
            throw error::http::InvalidIndexError{idx};
        }

        // Resolve the name behind `idx`, on whichever side (decoder/encoder) this
        // instantiation is wired for.
        auto field = [&] {
            if constexpr (IsDecoder) {
                return m_decoding_table->at<IsIndexPostBase>(idx, is_static, base);
            } else {
                return m_encoding_table->at<IsIndexPostBase>(idx, is_static, base);
            }
        }();

        std::visit(
            [&](auto &&field_ptr) {
                if constexpr (IsIndexable) {
                    // Encoder-stream-instruction path — cache the resolved pair into
                    // the dynamic table instead of appending it to `m_table`.
                    if constexpr (IsDecoder) {
                        m_decoding_table->insert(std::string{field_ptr->get_name()}, std::string{value});
                    } else {
                        m_encoding_table->insert(std::string{field_ptr->get_name()}, std::string{value});
                    }
                } else {
                    // Request/response stream path — a repeated `cookie` field folds
                    // into the existing entry (joined by "; ") instead of pushing a
                    // duplicate, mirroring HPACK's crumb-splitting in reverse.
                    const auto NAME = field_ptr->get_name();
                    if (NAME == "cookie") {
                        if (!m_cookie_index->is_empty()) {
                            m_cookie_index->set_value(m_cookie_index->get_value() + COOKIE_SEPARATOR +
                                                      std::string{value});
                        } else {
                            auto cookie_field = std::make_shared<interfaces::io::HeaderField<>>(NAME, std::string{value});
                            m_table.push_back(cookie_field);
                            m_cookie_index = cookie_field;
                        }
                    } else {
                        m_table.push_back(std::make_shared<interfaces::io::HeaderField<>>(NAME, std::string{value}));
                    }
                }
            },
            field);
    }

    /**
     * @brief Same fork as push_helper(), but for the new-name variants where both name and
     * value arrived as fresh literals rather than a table lookup — same cookie-folding special
     * case applies.
     * @tparam IsDecoder when true, operates on `m_decoding_table`; when false,
     * `m_encoding_table`.
     * @tparam IsIndexable when true, the pair gets inserted into the dynamic table instead of
     * appended to `m_table`.
     * @param name the decoded header name.
     * @param value the decoded header value.
     * @throws error::http::EmptyNameError if `name` is empty.
     * @warning Same nullptr footgun as push_helper() — the first `cookie` field decoded on a
     * fresh instance dereferences a null `m_cookie_index` before the empty check ever runs.
     */
    template <bool IsDecoder = true, bool IsIndexable = false>
    void push_helper_new_entry(std::string_view name, std::string_view value) {
        // RFC 9204 forbids an empty header name, same as HPACK.
        if (name.empty()) {
            throw error::http::EmptyNameError{};
        }

        if constexpr (IsIndexable) {
            // Encoder-stream-instruction path — cache the fresh pair.
            if constexpr (IsDecoder) {
                m_decoding_table->insert(name, value);
            } else {
                m_encoding_table->insert(name, value);
            }
        } else {
            // Request/response stream path — same cookie-folding special case as
            // push_helper() above.
            if (name == "cookie") {
                if (!m_cookie_index->is_empty()) {
                    m_cookie_index->set_value(m_cookie_index->get_value() + COOKIE_SEPARATOR + std::string{value});
                } else {
                    auto field = std::make_shared<interfaces::io::HeaderField<>>(std::string{name}, std::string{value});
                    m_table.push_back(field);
                    m_cookie_index = field;
                }
            } else {
                m_table.push_back(std::make_shared<interfaces::io::HeaderField<>>(std::string{name}, std::string{value}));
            }
        }
    }

    /* Field section prefix */

    /**
     * @brief Emits the two-integer field section prefix (RFC 9204 §4.5.1) — encoded RIC first,
     * then a sign-and-magnitude Delta Base that lets the decoder reconstruct Base without
     * needing it to always be >= RIC.
     * @tparam Out an output iterator accepting std::uint8_t.
     * @param ric the field section's Required Insert Count (unencoded).
     * @param base the field section's Base value, which every relative/post-base index in the
     * section gets computed against.
     * @param out the output iterator the prefix bytes get written to.
     */
    template <std::output_iterator<std::uint8_t> Out>
    void encode_field_section_prefix(UInt ric, UInt base, Out out) {
        // Wrap RIC into its wire-compressed form first, then emit it.
        UInt enc_ric = m_encoding_table->encode_ric(ric);
        shared_codec::raw::Atom<UInt, Width>::encode_int(enc_ric, 8U, 0x00, out);

        // RFC 9204: Base = RIC + DeltaBase (Sign 0) OR Base = RIC - DeltaBase - 1 (Sign 1)
        if (base >= ric) {
            // Sign bit 0 (Positive or Zero Delta)
            shared_codec::raw::Atom<UInt, Width>::encode_int(base - ric, 7U, 0x00, out);
        } else {
            // Sign bit 1 (Negative Delta)
            shared_codec::raw::Atom<UInt, Width>::encode_int(ric - base - 1, 7U, 0x80, out);
        }
    }

    /**
     * @brief Decodes the field section prefix, reversing the encoded RIC via QPackTable's
     * decode_ric() and applying the sign-and-magnitude Delta Base to recover Base.
     * @param data the field section bytes, positioned at the start of the prefix.
     * @param pos the read cursor into `data`; advanced past the prefix on return.
     * @return the decoded {Required Insert Count, Base} pair.
     * @throws error::http::TruncatedDataError if `data` runs out before the sign bit / delta
     * base can be read.
     * @throws error::http::CompressionError if the negative-delta branch underflows (delta
     * base >= required insert count with a zero required insert count) — a peer sending this is
     * lying about its own state, no cap.
     */
    std::pair<UInt, UInt> decode_field_section_prefix(std::span<const std::uint8_t> data, std::size_t &pos) {
        // Read the wire-encoded RIC and reverse the compression back to the true count.
        UInt enc_ric = shared_codec::raw::Atom<UInt, Width>::decode_int(data, pos, 8U).value();

        UInt req_insert_count = m_decoding_table->decode_ric(enc_ric);

        // Need at least one more byte for the sign bit / delta base.
        if (pos >= data.size()) {
            throw error::http::TruncatedDataError{};
        }

        // Sign bit lives in the delta base byte's top bit; read it before consuming
        // the integer itself.
        bool sign_bit = (data[pos] & 0x80) != 0;  // FIXME(clang-tidy): unchecked operator[], consider .at()
        UInt delta_base = shared_codec::raw::Atom<UInt, Width>::decode_int(data, pos, 7U).value();

        UInt base = 0;
        if (!sign_bit) {
            base = req_insert_count + delta_base;
        } else {
            // Negative delta logic: Base = RIC - Delta - 1
            if (req_insert_count == 0 || delta_base >= req_insert_count) {
                throw error::http::CompressionError{"Negative Base calculation underflow"};
            }
            base = req_insert_count - delta_base - 1;
        }

        return {req_insert_count, base};
    }

    /* Encoder / Decoder stream operations */

    // Encode

    // The pattern is 1Txxxxxx
    /**
     * @brief Emits an "Insert with Name Reference" encoder stream instruction (RFC 9204 §4.3.1,
     * pattern `1Txxxxxx`) — the name comes from an existing table entry (static or dynamic per
     * the T-bit), the value is a fresh literal, and the pair gets inserted into the encoding
     * table so a later field section can reference it by index.
     * @tparam Out an output iterator accepting std::uint8_t.
     * @param idx the index the name is found at, in whichever table `is_static` selects.
     * @param is_static when true, `idx` addresses the static table (sets the T-bit); when
     * false, the dynamic table.
     * @param value the header value to encode as a literal and index.
     * @param out the output iterator the instruction bytes get written to.
     */
    template <std::output_iterator<std::uint8_t> Out>
    void encode_insert_with_indexed_name(UInt idx, bool is_static, std::string_view value, Out out) {
        // Prefix is 1 (0x80), and T is the 7th bit (0x40)
        auto prefix = shared_codec::PrefixHelper::QPACK_INSERT_INDEXED_NAME;
        // Set the T-bit if it's a static entry.
        if (is_static) {
            prefix |= 0x40;
        }

        // We use a 5-bit prefix length because bits 3-7 are used for the integer value.
        shared_codec::raw::Atom<UInt, Width>::encode_int(idx, 6U, prefix, out);
        // Value gets emitted as a literal string right after the index.
        shared_codec::raw::Atom<UInt, Width>::encode_string(m_huffman, value, out);


        // Insert the resolved pair into the encoding table so later field sections
        // can reference it by index instead of re-sending it.
        push_helper<false, true>(idx, value, is_static, 0);
    }

    // The pattern is 01Hxxxxx
    /**
     * @brief Emits an "Insert with Literal Name" encoder stream instruction (RFC 9204 §4.3.2,
     * pattern `01Hxxxxx`) — both name and value are fresh literals (H-bit flags whether the
     * name got Huffman-coded), inserted into the encoding table for future reuse.
     * @tparam Out an output iterator accepting std::uint8_t.
     * @param name the header name to encode as a literal and index.
     * @param value the header value to encode as a literal and index.
     * @param out the output iterator the instruction bytes get written to.
     */
    template <std::output_iterator<std::uint8_t> Out>
    void encode_insert_with_literal_name(std::string_view name, std::string_view value, Out out) {
        // Both name and value go out as literal strings — no table entry to reuse.
        shared_codec::raw::Atom<UInt, Width>::encode_string(m_huffman, name, out, 6U);
        shared_codec::raw::Atom<UInt, Width>::encode_string(m_huffman, value, out);

        // Index the fresh pair for future reuse.
        push_helper_new_entry<false, true>(std::string{name}, std::string{value});
    }

    // The pattern is 000xxxxx
    /**
     * @brief Emits a "Duplicate" encoder stream instruction (RFC 9204 §4.3.4, pattern
     * `000xxxxx`) — re-inserts an existing dynamic table entry at the front, refreshing its
     * age so it survives eviction longer without re-sending its literal name/value.
     * @tparam Out an output iterator accepting std::uint8_t.
     * @param idx the dynamic table index of the entry to duplicate.
     * @param out the output iterator the instruction bytes get written to.
     */
    template <std::output_iterator<std::uint8_t> Out>
    void encode_duplicate(UInt idx, Out out) {
        // We use a 5-bit prefix length because bits 3-7 are used for the integer value.
        shared_codec::raw::Atom<UInt, Width>::encode_int(idx, 5U, shared_codec::PrefixHelper::QPACK_DUPLICATE, out);

        // Re-insert the referenced entry at the front — refreshes its age against
        // eviction without re-sending the literal name/value.
        m_encoding_table->insert(m_encoding_table->at(idx, false));
    }


    // The pattern is 001xxxxx
    /**
     * @brief Emits a "Set Dynamic Table Capacity" encoder stream instruction (RFC 9204 §4.3.3,
     * pattern `001xxxxx`) and resizes the encoding table to match — table and wire state stay
     * in lockstep since the resize happens right here.
     * @tparam Out an output iterator accepting std::uint8_t.
     * @param size the new dynamic table max size in bytes.
     * @param out the output iterator the instruction bytes get written to.
     */
    template <std::output_iterator<std::uint8_t> Out>
    void encode_table_size_update(UInt size, Out out) {
        // Resize first so table and wire state never get to disagree, same pattern as
        // HPACK's size-update adaptor.
        m_encoding_table->set_max_size(size);

        shared_codec::raw::Atom<UInt, Width>::encode_int(
            size, 5U, shared_codec::PrefixHelper::QPACK_DYNAMIC_TABLE_SIZE_UPDATE, out);
    }

    // Decode

    // The pattern is 1Txxxxxx
    /**
     * @brief Decodes an "Insert with Name Reference" encoder stream instruction — table-indexed
     * name plus a literal value, inserted into the decoding table via push_helper<true, true>().
     * @param data the instruction bytes.
     * @param pos the read cursor into `data`; advanced past the instruction on return.
     */
    void decode_inserted_with_indexed_name(std::span<const std::uint8_t> data, std::size_t &pos) {
        // Table-indexed name (T-bit carried on IDX), literal value right after it.
        const auto IDX = shared_codec::raw::Atom<UInt, Width>::template decode_int<1>(data, pos, 7U);
        const auto VALUE = shared_codec::raw::Atom<UInt, Width>::decode_string(m_huffman, data, pos);

        // Indexable — the resolved pair gets cached into the decoding table.
        push_helper<true, true>(IDX.value(), VALUE, IDX.is_static(), 0);
    }

    // The pattern is 01Hxxxxx
    /**
     * @brief Decodes an "Insert with Literal Name" encoder stream instruction — fresh name and
     * value, inserted into the decoding table via push_helper_new_entry<true, true>().
     * @param data the instruction bytes.
     * @param pos the read cursor into `data`; advanced past the instruction on return.
     */
    void decode_inserted_with_literal_name(std::span<const std::uint8_t> data, std::size_t &pos) {
        // Both name and value arrive as literals.
        const auto NAME = shared_codec::raw::Atom<UInt, Width>::decode_string(m_huffman, data, pos, 5U);
        const auto VALUE = shared_codec::raw::Atom<UInt, Width>::decode_string(m_huffman, data, pos);

        // Index the fresh pair into the decoding table.
        push_helper_new_entry<true, true>(NAME, VALUE);
    }

    // The pattern is 000xxxxx
    /**
     * @brief Decodes a "Duplicate" encoder stream instruction and re-inserts the referenced
     * dynamic table entry at the front, refreshing its age against eviction.
     * @param data the instruction bytes.
     * @param pos the read cursor into `data`; advanced past the instruction on return.
     * @throws error::http::InvalidIndexError if the decoded index is 0.
     */
    void decode_duplicate(std::span<const std::uint8_t> data, std::size_t &pos) {
        // Decode the referenced index, reject the never-valid 0.
        const auto IDX = shared_codec::raw::Atom<UInt, Width>::decode_int(data, pos, 5U);
        if (IDX.value() == 0) {
            throw error::http::InvalidIndexError{IDX.value()};
        }

        // Re-insert the referenced entry at the front, same age-refresh as the encode side.
        m_decoding_table->insert(m_decoding_table->at<>(IDX, false));
    }

    // The pattern is 001xxxxx
    /**
     * @brief Decodes a "Set Dynamic Table Capacity" encoder stream instruction and applies it
     * to the decoding table.
     * @param data the instruction bytes.
     * @param pos the read cursor into `data`; advanced past the instruction on return.
     * @note The RFC 9204 §4.3.3 cap against the negotiated maximum table capacity is a TODO
     * here (see the commented-out bounds check right below) — for now any size the peer sends
     * gets applied as-is.
     */
    void decode_table_size_update(std::span<const std::uint8_t> data, std::size_t &pos) {
        // Pull the new capacity off the wire.
        const auto NEW_SIZE = shared_codec::raw::Atom<UInt, Width>::decode_int(data, pos, 5U);
        // TODO: check the specs
        // if (new_size > m_decoding_table->max_size())
        //     throw error::http::TableSizeError{new_size, m_decoding_table->max_size()};

        m_decoding_table->set_max_size(NEW_SIZE);
    }

    /* Decoder Helper */

    // The pattern is  1xxxxxxx
    /**
     * @brief Emits a "Section Acknowledgment" decoder stream instruction (RFC 9204 §4.4.1,
     * pattern `1xxxxxxx`) — tells the encoder a specific stream's field section has been fully
     * processed, so it can safely evict entries that section depended on.
     * @tparam Out an output iterator accepting std::uint8_t.
     * @param size the stream ID being acknowledged.
     * @param out the output iterator the instruction bytes get written to.
     */
    template <std::output_iterator<std::uint8_t> Out>
    void encode_section_acknowledgment(UInt size, Out out) {
        // We use a 7-bit prefix length because the first bit is used for the prefix.
        shared_codec::raw::Atom<UInt, Width>::encode_int(size, 7U, shared_codec::PrefixHelper::QPACK_DEC_ACK, out);
    }

    // The pattern is  01xxxxxx
    /**
     * @brief Emits a "Stream Cancellation" decoder stream instruction (RFC 9204 §4.4.2, pattern
     * `01xxxxxx`) — tells the encoder a stream got cancelled without its field section ever
     * being fully processed, so any entries it would've depended on can be released.
     * @tparam Out an output iterator accepting std::uint8_t.
     * @param size the stream ID being cancelled.
     * @param out the output iterator the instruction bytes get written to.
     */
    template <std::output_iterator<std::uint8_t> Out>
    void encode_stream_cancellation(UInt size, Out out) {
        // We use a 6-bit prefix length because the first bit is used for the prefix.
        shared_codec::raw::Atom<UInt, Width>::encode_int(
            size, 6U, shared_codec::PrefixHelper::QPACK_DEC_STREAM_CANCELLATION, out);
    }

    // The pattern is  00xxxxxx
    /**
     * @brief Emits an "Insert Count Increment" decoder stream instruction (RFC 9204 §4.4.3,
     * pattern `00xxxxxx`) — tells the encoder how many additional dynamic table inserts the
     * decoder now knows about, letting it unblock field sections waiting on them.
     * @tparam Out an output iterator accepting std::uint8_t.
     * @param size the increment amount.
     * @param out the output iterator the instruction bytes get written to.
     */
    template <std::output_iterator<std::uint8_t> Out>
    void encode_insert_count_increment(UInt size, Out out) {
        // We use a 6-bit prefix length because the first bit is used for the prefix.
        shared_codec::raw::Atom<UInt, Width>::encode_int(
            size, 6U, shared_codec::PrefixHelper::QPACK_DEC_INSERT_COUNT_INCREMENT, out);
    }

    // The pattern is 1xxxxxxx
    /**
     * @brief Decodes a "Section Acknowledgment" decoder stream instruction — reads and consumes
     * the acknowledged stream ID, currently just logging it.
     * @param data the instruction bytes.
     * @param pos the read cursor into `data`; advanced past the instruction on return.
     * @warning Declared `void`, but decode_decoder_stream() calls this expecting a `UInt`
     * stream ID back (`UInt stream_id = decode_section_acknowledgment(...)`). That's a
     * return-type mismatch as written — straight L, won't build until one side gets fixed. Also
     * the actual bookkeeping against `m_blocked_streams`/`m_known_received_count` is commented
     * out below (TODO), so even once it compiles this doesn't yet act on the acknowledgment.
     */
    void decode_section_acknowledgment(std::span<const std::uint8_t> data, std::size_t &pos) {
        // Consume the stream ID off the wire — actually acting on it is still a TODO
        // below, so for now this just logs it.
        const auto STREAM_ID = shared_codec::raw::Atom<UInt, Width>::decode_int(data, pos, 7U).value();
        std::print("Decoded Section Acknowledgment for stream ID: {}\n", STREAM_ID);

        // if (!m_blocked_streams.contains(stream_id))
        //     throw error::http::QpackDecoderStreamError{
        //         std::format("Section acknowledgment for unknown or unblocked stream {}", stream_id)};
        //
        // UInt ric = m_blocked_streams.at(stream_id);
        //
        // if (m_known_received_count < ric)
        //     throw error::http::QpackDecoderStreamError{std::format("Section acknowledgment for stream {} premature: "
        //                                                            "known_received_count={} <
        //                                                            required_insert_count={}", stream_id,
        //                                                            m_known_received_count, ric)};
        //
        // m_blocked_streams.erase(stream_id);
    }

    // The pattern is  01xxxxxx
    /**
     * @brief Decodes a "Stream Cancellation" decoder stream instruction — reads and consumes
     * the cancelled stream ID.
     * @param data the instruction bytes.
     * @param pos the read cursor into `data`; advanced past the instruction on return.
     * @warning Same return-type mismatch as decode_section_acknowledgment(): declared `void`
     * but called as `UInt stream_id = decode_stream_cancellation(...)`. Freeing the blocked
     * stream state is also a TODO — right now this just consumes the bytes and logs (with a
     * copy-pasted "Section Acknowledgment" message, no less).
     */
    void decode_stream_cancellation(std::span<const std::uint8_t> data, std::size_t &pos) {
        // Same "consume and log" stub shape as decode_section_acknowledgment() — freeing
        // blocked stream state is still a TODO.
        const auto STREAM_ID = shared_codec::raw::Atom<UInt, Width>::decode_int(data, pos, 6U).value();
        std::print("Decoded Section Acknowledgment for stream ID: {}\n", STREAM_ID);

        // TODO: implement logic to free blocked stream state associated with this stream ID
        // m_blocked_streams.erase(stream_id);
    }

    // The pattern is  00xxxxxx
    /**
     * @brief Decodes an "Insert Count Increment" decoder stream instruction, validates it, and
     * applies it directly to `m_known_received_count` — a peer can't increment past however
     * many inserts this side's dynamic table has actually made.
     * @param data the instruction bytes.
     * @param pos the read cursor into `data`; advanced past the instruction on return.
     * @throws error::http::InvalidIndexError if the decoded increment is 0 (RFC 9204 §4.4.3
     * forbids a zero increment).
     * @throws error::http::CompressionError if the increment would push the known-received
     * count past the dynamic table's actual insert count.
     * @warning Declared `void` and already mutates `m_known_received_count` internally, but
     * decode_decoder_stream() calls it as `m_known_received_count +=
     * decode_insert_count_increment(...)` — that's a type error (can't add `void`) on top of
     * double-applying the increment if it were ever coerced into compiling. Same return-type
     * mismatch pattern as decode_section_acknowledgment()/decode_stream_cancellation() above.
     */
    void decode_insert_count_increment(std::span<const std::uint8_t> data, std::size_t &pos) {
        // RFC 9204 §4.4.3 forbids a zero increment — reject before it does anything.
        const auto SIZE = shared_codec::raw::Atom<UInt, Width>::decode_int(data, pos, 6U);
        if (SIZE.value() == 0) {
            throw error::http::InvalidIndexError{SIZE.value()};
        }

        // A peer can't claim credit for more inserts than this side's table has
        // actually made — bound-check before applying.
        UInt max_increment = m_decoding_table->insert_count() - m_known_received_count;
        if (SIZE.value() > max_increment) {
            throw error::http::CompressionError{"Insert Count Increment exceeds unacknowledged insertion count"};
        }

        m_known_received_count += SIZE.value();
    }

    /* Request / Response stream! */

    // Encode

    // The pattern is 1Txxxxxx
    /**
     * @brief Emits an Indexed Field Line for the request/response stream (RFC 9204 §4.5.2,
     * pattern `1Txxxxxx`) — both name and value already live in a table (T-bit selects static
     * vs. dynamic), so it's just an index on the wire.
     * @tparam Out an output iterator accepting std::uint8_t.
     * @param idx the index the field is fully found at, in whichever table `is_static` selects.
     * @param is_static when true, `idx` addresses the static table (sets the T-bit); when
     * false, the dynamic table.
     * @param out the output iterator the field line bytes get written to.
     * @warning Two loose ends here: the body calls `idx.value()` even though `idx` is a plain
     * `UInt`, not a DecodeIntResult — that's not valid on an integer type. And every call site
     * (in encode()) passes a `base` argument too, but this signature only takes three params.
     * Both read like unfinished refactor residue, not intentional design — flagging so it
     * doesn't get mistaken for load-bearing behavior.
     */
    template <std::output_iterator<std::uint8_t> Out>
    void encode_indexed_field(UInt idx, bool is_static, Out out) {
        // Prefix is 1 (0x80), and T is the 7th bit (0x40)
        auto prefix = shared_codec::PrefixHelper::HPACK_INDEXED_FIELD;
        // Set the T-bit if it's a static entry.
        if (is_static) {
            prefix |= 0x40;
        }

        // We use a 5-bit prefix length because bits 3-7 are used for the integer value.
        shared_codec::raw::Atom<UInt, Width>::encode_int(idx, 6U, prefix, out);

        // Resolve the fully-indexed field and add it to the outgoing field list.
        add_field(m_encoding_table->at<>(idx.value()));
    }

    // The pattern is 0001xxxx
    /**
     * @brief Emits a Post-Base Indexed Field Line (RFC 9204 §4.5.3, pattern `0001xxxx`) — an
     * indexed field whose dynamic table entry was inserted after this section's Base was fixed,
     * so no cap, it's addressed forward from Base instead of backward.
     * @tparam Out an output iterator accepting std::uint8_t.
     * @param idx the post-base index the field is fully found at.
     * @param out the output iterator the field line bytes get written to.
     */
    template <std::output_iterator<std::uint8_t> Out>
    void encode_post_base_indexed_field(UInt idx, Out out) {
        // We use a 4-bit prefix length because bits 4-7 are used for the integer value.
        shared_codec::raw::Atom<UInt, Width>::encode_int(
            idx, 4U, shared_codec::PrefixHelper::QPACK_POST_BASE_INDEXED_FIELD, out);

        // Resolve using post-base interpretation and add to the outgoing field list.
        add_field(m_encoding_table->at<true>(idx.value()));
    }

    // The pattern is 01NTxxxx
    /**
     * @brief Emits a Literal Field Line with Name Reference (RFC 9204 §4.5.4, pattern
     * `01NTxxxx`) — table-indexed name plus a literal value; N flags never-indexed (for
     * sensitive headers like `authorization`/`cookie`) and T flags static vs. dynamic. Also
     * inserts the resolved pair into the encoding table, mirroring HPACK's incremental-indexing
     * behavior for this line type.
     * @tparam Out an output iterator accepting std::uint8_t.
     * @param idx the index the name is found at, in whichever table `is_static` selects.
     * @param value the header value to encode as a literal.
     * @param is_static when true, `idx` addresses the static table (sets the T-bit).
     * @param is_never_indexed when true, sets the N-bit so intermediaries must forward this
     * line unindexed too.
     * @param out the output iterator the field line bytes get written to.
     * @warning Same `idx.value()`/`idx.is_static()`-on-a-plain-`UInt` issue as
     * encode_indexed_field() above — `idx` here is a raw integer parameter, not a
     * DecodeIntResult, so those member calls don't exist on it as written.
     */
    template <std::output_iterator<std::uint8_t> Out>
    void encode_indexed_name(UInt idx, std::string_view value, bool is_static, bool is_never_indexed, Out out) {
        // Prefix is 1 (0x40), and N is the 6th bit (0x20), and T is the 5th bit (0x10)
        auto prefix = shared_codec::PrefixHelper::QPACK_INDEXED_NAME;
        // Set the N-bit if the filed can never be indexd.
        if (is_never_indexed) {
            prefix |= 0x20;
        }
        // Set the T-bit if it's a static entry.
        if (is_static) {
            prefix |= 0x10;
        }

        // We use a 4-bit prefix length because bits 4-7 are used for the integer value.
        shared_codec::raw::Atom<UInt, Width>::encode_int(idx, 4U, prefix, out);
        // Value goes out as a literal string right after the index.
        shared_codec::raw::Atom<UInt, Width>::encode_string(m_huffman, value, out);


        // Mirrors HPACK's incremental-indexing behavior — cache the resolved pair too.
        push_helper<false, true>(idx.value(), value, idx.is_static(), 0);
    }

    // The pattern is 0000Nxxx
    /**
     * @brief Emits a Literal Field Line with Post-Base Name Reference (RFC 9204 §4.5.5, pattern
     * `0000Nxxx`) — like encode_indexed_name(), but the referenced name lives in a dynamic
     * table entry inserted after this section's Base, so it's addressed forward instead of
     * backward.
     * @tparam Out an output iterator accepting std::uint8_t.
     * @param idx the post-base index the name is found at.
     * @param value the header value to encode as a literal.
     * @param is_never_indexed when true, sets the N-bit so intermediaries must forward this
     * line unindexed too.
     * @param out the output iterator the field line bytes get written to.
     */
    template <std::output_iterator<std::uint8_t> Out>
    void encode_post_base_indexed_name(UInt idx, std::string_view value, bool is_never_indexed, Out out) {
        // Prefix is 4 bits (0x00), and N is the 4th bit (0x08)
        auto prefix = shared_codec::PrefixHelper::QPACK_POST_BASE_INDEXED_NAME;
        // Set the N-bit if the filed can never be indexd.
        if (is_never_indexed) {
            prefix |= 0x08;
        }

        // We use a 5-bit prefix length because bits 5-7 are used for the integer value.
        shared_codec::raw::Atom<UInt, Width>::encode_int(idx, 3U, prefix, out);
        // Value goes out as a literal string right after the index.
        shared_codec::raw::Atom<UInt, Width>::encode_string(m_huffman, value, out);


        // Post-base interpretation forwarded through so the cache stays consistent
        // with how this field actually got addressed on the wire.
        push_helper<false, true, true>(idx.value(), value, idx.is_static(), 0);
    }

    // The pattern is 001NHxxx
    /**
     * @brief Emits a Literal Field Line with Literal Name (RFC 9204 §4.5.6, pattern
     * `001NHxxx`) — both name and value are fresh literals (H-bit flags whether the name got
     * Huffman-coded), inserted into the encoding table via push_helper_new_entry.
     * @tparam Out an output iterator accepting std::uint8_t.
     * @param name the header name to encode as a literal.
     * @param value the header value to encode as a literal.
     * @param is_never_indexed when true, sets the N-bit so intermediaries must forward this
     * line unindexed too.
     * @param out the output iterator the field line bytes get written to.
     */
    template <std::output_iterator<std::uint8_t> Out>
    void encode_new_field(std::string_view name, std::string_view value, bool is_never_indexed, Out out) {
        // Prefix is 1 (0x20), and N is the 5th bit (0x10)
        auto prefix = shared_codec::PrefixHelper::QPACK_NEW_FIELD;
        // Set the N-bit if the filed can never be indexd.
        if (is_never_indexed) {
            prefix |= 0x10;
        }

        // We use a 5-bit prefix length because bits 5-7 are used for the integer value.
        shared_codec::raw::Atom<UInt, Width>::encode_string(m_huffman, name, out, 3U);
        shared_codec::raw::Atom<UInt, Width>::encode_string(m_huffman, value, out);

        // Fresh pair — index it for reuse by later field sections.
        push_helper_new_entry<false, true>(name, value);
    }

    // Decode

    // The pattern is 1Txxxxxx
    /**
     * @brief Decodes an Indexed Field Line — resolves the table-indexed field and appends it
     * straight to `m_table` via add_field().
     * @param data the field line bytes.
     * @param pos the read cursor into `data`; advanced past the field line on return.
     * @param base the field section's Base value, used to resolve dynamic-table indices.
     * @throws error::http::InvalidIndexError if the decoded index is 0.
     */
    void decode_indexed_field(std::span<const std::uint8_t> data, std::size_t &pos, std::size_t base) {
        // Decode the index (T-bit carried on IDX) and reject the never-valid 0.
        const auto IDX = shared_codec::raw::Atom<UInt, Width>::template decode_int<1>(data, pos, 6U);
        if (IDX.value() == 0) {
            throw error::http::InvalidIndexError{IDX.value()};
        }

        // Resolve against Base and append straight to the output field list.
        add_field(m_decoding_table->at<>(IDX.value(), IDX.is_static(), base));
    }

    // The pattern is 0001xxxx
    /**
     * @brief Decodes a Post-Base Indexed Field Line — resolves the post-base-indexed field
     * (counted forward from Base) and appends it to `m_table` via add_field().
     * @param data the field line bytes.
     * @param pos the read cursor into `data`; advanced past the field line on return.
     * @param base the field section's Base value that the post-base index is counted from.
     * @throws error::http::InvalidIndexError if the decoded index is 0.
     */
    void decode_post_base_indexed_field(std::span<const std::uint8_t> data, std::size_t &pos, std::size_t base) {
        // Decode the post-base index and reject the never-valid 0.
        const auto IDX = shared_codec::raw::Atom<UInt, Width>::template decode_int<1>(data, pos, 4U);
        if (IDX.value() == 0) {
            throw error::http::InvalidIndexError{IDX.value()};
        }

        // Resolve counting forward from Base, then append to the output list.
        add_field(m_decoding_table->at<true>(IDX.value(), IDX.is_static(), base));
    }

    // The pattern is 01NTxxxx
    /**
     * @brief Decodes a Literal Field Line with Name Reference — table-indexed name plus a
     * literal value, pushed via push_helper (default template args: not indexed into the
     * table, decoder side).
     * @param data the field line bytes.
     * @param pos the read cursor into `data`; advanced past the field line on return.
     * @param base the field section's Base value, used to resolve dynamic-table name indices.
     */
    void decode_indexed_name(std::span<const std::uint8_t> data, std::size_t &pos, std::size_t base) {
        // Table-indexed name, literal value right after it.
        const auto IDX = shared_codec::raw::Atom<UInt, Width>::template decode_int<2>(data, pos, 4U);
        const auto VALUE = shared_codec::raw::Atom<UInt, Width>::decode_string(m_huffman, data, pos);

        // Default template args — decoder side, not cached into the table.
        push_helper<>(IDX, VALUE, IDX.is_static(), base);
    }

    // The pattern is 0000Nxxx
    /**
     * @brief Decodes a Literal Field Line with Post-Base Name Reference — lowkey the rarest
     * line type on the wire, post-base-indexed name plus a literal value, pushed via
     * push_helper<true, false, true> (decoder side, not table-indexed, post-base
     * interpretation).
     * @param data the field line bytes.
     * @param pos the read cursor into `data`; advanced past the field line on return.
     * @param base the field section's Base value that the post-base name index is counted from.
     */
    void decode_post_base_indexed_name(std::span<const std::uint8_t> data, std::size_t &pos, std::size_t base) {
        // Post-base-indexed name, literal value right after it.
        const auto IDX = shared_codec::raw::Atom<UInt, Width>::template decode_int<1>(data, pos, 3U);
        const auto VALUE = shared_codec::raw::Atom<UInt, Width>::decode_string(m_huffman, data, pos);

        // Post-base interpretation forwarded through to the table lookup.
        push_helper<true, false, true>(IDX, VALUE, IDX.is_static(), base);
    }

    // The pattern is 001NHxxx
    /**
     * @brief Decodes a Literal Field Line with Literal Name — fresh name and value, pushed via
     * push_helper_new_entry (default template args: decoder side, not indexed into the table).
     * @param data the field line bytes.
     * @param pos the read cursor into `data`; advanced past the field line on return.
     * @throws error::http::EmptyNameError if the decoded name is empty.
     */
    void decode_new_field(std::span<const std::uint8_t> data, std::size_t &pos) {
        // Both name and value arrive as fresh literals.
        const auto NAME = shared_codec::raw::Atom<UInt, Width>::decode_string(m_huffman, data, pos, 3U);
        const auto VALUE = shared_codec::raw::Atom<UInt, Width>::decode_string(m_huffman, data, pos);

        // Default template args — decoder side, not cached into the table.
        push_helper_new_entry<>(NAME, VALUE);
    }


    std::shared_ptr<QPackTable> m_decoding_table;
    std::shared_ptr<QPackTable> m_encoding_table;
    shared_codec::huffman::Huffman<> m_huffman;
    std::vector<std::shared_ptr<interfaces::io::HeaderField<>>> m_table;
    UInt m_known_received_count;
    std::shared_ptr<interfaces::io::HeaderField<>> m_cookie_index;
};
} // namespace io::codec::qpack
