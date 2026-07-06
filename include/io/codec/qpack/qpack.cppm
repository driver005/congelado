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
    explicit QPack(std::size_t table_size = 4096)
        : m_decoding_table{std::make_shared<QPackTable>(table_size)},
          m_encoding_table{std::make_shared<QPackTable>(table_size)}, m_huffman{}, m_known_received_count{0},
          m_cookie_index{nullptr} {}

    explicit QPack(std::shared_ptr<QPackTable> decoding_table, std::shared_ptr<QPackTable> encoding_table)
        : m_decoding_table(std::move(decoding_table)), m_encoding_table(std::move(encoding_table)), m_huffman{},
          m_known_received_count{0}, m_cookie_index{nullptr} {}


    // Encode a complete field section (request/response stream).
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

        UInt base = required_insert_count;

        encode_field_section_prefix(required_insert_count, out);

        // 4. Second pass: Encode the actual instructions using the Base
        for (const auto &field : m_table) {
            shared_codec::SearchResult result = m_encoding_table->search(field->get_name(), field->get_value());

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
    void decode(std::span<const std::uint8_t> data) {
        std::size_t pos = 0;
        auto [ric, base] = decode_field_section_prefix(data, pos);

        if (ric > m_decoding_table->insert_count()) {
            throw error::http::CompressionError{"Stream blocked: Required Insert Count not yet reached"};
        }

        while (pos < data.size()) {
            auto rep_type = shared_codec::detect_representation_qpack_stream(data[pos]);

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
                throw error::http::CompressionError{"Unknown QPACK field line representation"};
            }
        }
    }

    // Encode an encoder stream instruction sequence.
    template <std::output_iterator<std::uint8_t> Out>
    void encode_encoder_stream(Out out) {
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
    void decode_encoder_stream(std::span<const std::uint8_t> data) {
        std::size_t pos = 0;

        while (pos < data.size()) {
            auto rep_type = shared_codec::detect_representation_qpack_encoder(data[pos]);

            switch (rep_type) {
            case shared_codec::PrefixHelper::QPACK_INSERT_INDEXED_NAME:
                decode_inserted_with_indexed_name(data, pos);
                break;
            case shared_codec::PrefixHelper::QPACK_INSERT_LITERAL_NAME:
                decode_inserted_with_literal_name(data, pos);
                break;
            case shared_codec::PrefixHelper::QPACK_DYNAMIC_TABLE_SIZE_UPDATE: {
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
    void decode_decoder_stream(std::span<const std::uint8_t> data) {
        std::size_t pos = 0;

        while (pos < data.size()) {
            auto rep_type = shared_codec::detect_representation_qpack_decoder(data[pos]);

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
    template <std::output_iterator<std::uint8_t> Out>
    void encode_section_ack(UInt stream_id, Out out) {
        encode_section_acknowledgment(stream_id, out);
    }

    template <std::output_iterator<std::uint8_t> Out>
    void encode_stream_cancel(UInt stream_id, Out out) {
        encode_stream_cancellation(stream_id, out);
    }

    template <std::output_iterator<std::uint8_t> Out>
    void encode_insert_count_inc(UInt increment, Out out) {
        encode_insert_count_increment(increment, out);
    }

    void add_field(const std::shared_ptr<interfaces::io::HeaderField<>> &field) { m_table.push_back(field); }

    void set_table(const std::vector<std::shared_ptr<interfaces::io::HeaderField<>>> &table) { m_table = table; }

  private:
    // Helper

    // The ToBeIndexed template parameter indicates whether the field being pushed should be added to the decoding table
    // (true for encoder stream instructions, false for request/response stream).
    template <bool IsDecoder = true, bool IsIndexable = false, bool IsIndexPostBase = false>
    void push_helper(UInt idx, std::string_view value, bool is_static, std::size_t base) {
        if (idx == 0) {
            throw error::http::InvalidIndexError{idx};
        }

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
                    if constexpr (IsDecoder) {
                        m_decoding_table->insert(std::string{field_ptr->get_name()}, std::string{value});
                    } else {
                        m_encoding_table->insert(std::string{field_ptr->get_name()}, std::string{value});
                    }
                } else {
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

    template <bool IsDecoder = true, bool IsIndexable = false>
    void push_helper_new_entry(std::string_view name, std::string_view value) {
        if (name.empty()) {
            throw error::http::EmptyNameError{};
        }

        if constexpr (IsIndexable) {
            if constexpr (IsDecoder) {
                m_decoding_table->insert(name, value);
            } else {
                m_encoding_table->insert(name, value);
            }
        } else {
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

    template <std::output_iterator<std::uint8_t> Out>
    void encode_field_section_prefix(UInt ric, UInt base, Out out) {
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

    std::pair<UInt, UInt> decode_field_section_prefix(std::span<const std::uint8_t> data, std::size_t &pos) {
        UInt enc_ric = shared_codec::raw::Atom<UInt, Width>::decode_int(data, pos, 8U).value();

        UInt req_insert_count = m_decoding_table->decode_ric(enc_ric);

        if (pos >= data.size()) {
            throw error::http::TruncatedDataError{};
        }

        bool sign_bit = (data[pos] & 0x80) != 0;
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
        shared_codec::raw::Atom<UInt, Width>::encode_string(m_huffman, value, out);


        push_helper<false, true>(idx, value, is_static, 0);
    }

    // The pattern is 01Hxxxxx
    template <std::output_iterator<std::uint8_t> Out>
    void encode_insert_with_literal_name(std::string_view name, std::string_view value, Out out) {
        shared_codec::raw::Atom<UInt, Width>::encode_string(m_huffman, name, out, 6U);
        shared_codec::raw::Atom<UInt, Width>::encode_string(m_huffman, value, out);

        push_helper_new_entry<false, true>(std::string{name}, std::string{value});
    }

    // The pattern is 000xxxxx
    template <std::output_iterator<std::uint8_t> Out>
    void encode_duplicate(UInt idx, Out out) {
        // We use a 5-bit prefix length because bits 3-7 are used for the integer value.
        shared_codec::raw::Atom<UInt, Width>::encode_int(idx, 5U, shared_codec::PrefixHelper::QPACK_DUPLICATE, out);

        m_encoding_table->insert(m_encoding_table->at(idx, false));
    }


    // The pattern is 001xxxxx
    template <std::output_iterator<std::uint8_t> Out>
    void encode_table_size_update(UInt size, Out out) {
        m_encoding_table->set_max_size(size);

        shared_codec::raw::Atom<UInt, Width>::encode_int(
            size, 5U, shared_codec::PrefixHelper::QPACK_DYNAMIC_TABLE_SIZE_UPDATE, out);
    }

    // Decode

    // The pattern is 1Txxxxxx
    void decode_inserted_with_indexed_name(std::span<const std::uint8_t> data, std::size_t &pos) {
        const auto IDX = shared_codec::raw::Atom<UInt, Width>::template decode_int<1>(data, pos, 7U);
        const auto VALUE = shared_codec::raw::Atom<UInt, Width>::decode_string(m_huffman, data, pos);

        push_helper<true, true>(IDX.value(), VALUE, IDX.is_static(), 0);
    }

    // The pattern is 01Hxxxxx
    void decode_inserted_with_literal_name(std::span<const std::uint8_t> data, std::size_t &pos) {
        const auto NAME = shared_codec::raw::Atom<UInt, Width>::decode_string(m_huffman, data, pos, 5U);
        const auto VALUE = shared_codec::raw::Atom<UInt, Width>::decode_string(m_huffman, data, pos);

        push_helper_new_entry<true, true>(NAME, VALUE);
    }

    // The pattern is 000xxxxx
    void decode_duplicate(std::span<const std::uint8_t> data, std::size_t &pos) {
        const auto IDX = shared_codec::raw::Atom<UInt, Width>::decode_int(data, pos, 5U);
        if (IDX.value() == 0) {
            throw error::http::InvalidIndexError{IDX.value()};
        }

        m_decoding_table->insert(m_decoding_table->at<>(IDX, false));
    }

    // The pattern is 001xxxxx
    void decode_table_size_update(std::span<const std::uint8_t> data, std::size_t &pos) {
        const auto NEW_SIZE = shared_codec::raw::Atom<UInt, Width>::decode_int(data, pos, 5U);
        // TODO: check the specs
        // if (new_size > m_decoding_table->max_size())
        //     throw error::http::TableSizeError{new_size, m_decoding_table->max_size()};

        m_decoding_table->set_max_size(NEW_SIZE);
    }

    /* Decoder Helper */

    // The pattern is  1xxxxxxx
    template <std::output_iterator<std::uint8_t> Out>
    void encode_section_acknowledgment(UInt size, Out out) {
        // We use a 7-bit prefix length because the first bit is used for the prefix.
        shared_codec::raw::Atom<UInt, Width>::encode_int(size, 7U, shared_codec::PrefixHelper::QPACK_DEC_ACK, out);
    }

    // The pattern is  01xxxxxx
    template <std::output_iterator<std::uint8_t> Out>
    void encode_stream_cancellation(UInt size, Out out) {
        // We use a 6-bit prefix length because the first bit is used for the prefix.
        shared_codec::raw::Atom<UInt, Width>::encode_int(
            size, 6U, shared_codec::PrefixHelper::QPACK_DEC_STREAM_CANCELLATION, out);
    }

    // The pattern is  00xxxxxx
    template <std::output_iterator<std::uint8_t> Out>
    void encode_insert_count_increment(UInt size, Out out) {
        // We use a 6-bit prefix length because the first bit is used for the prefix.
        shared_codec::raw::Atom<UInt, Width>::encode_int(
            size, 6U, shared_codec::PrefixHelper::QPACK_DEC_INSERT_COUNT_INCREMENT, out);
    }

    // The pattern is 1xxxxxxx
    void decode_section_acknowledgment(std::span<const std::uint8_t> data, std::size_t &pos) {
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
    void decode_stream_cancellation(std::span<const std::uint8_t> data, std::size_t &pos) {
        const auto STREAM_ID = shared_codec::raw::Atom<UInt, Width>::decode_int(data, pos, 6U).value();
        std::print("Decoded Section Acknowledgment for stream ID: {}\n", STREAM_ID);

        // TODO: implement logic to free blocked stream state associated with this stream ID
        // m_blocked_streams.erase(stream_id);
    }

    // The pattern is  00xxxxxx
    void decode_insert_count_increment(std::span<const std::uint8_t> data, std::size_t &pos) {
        const auto SIZE = shared_codec::raw::Atom<UInt, Width>::decode_int(data, pos, 6U);
        if (SIZE.value() == 0) {
            throw error::http::InvalidIndexError{SIZE.value()};
        }

        UInt max_increment = m_decoding_table->insert_count() - m_known_received_count;
        if (SIZE.value() > max_increment) {
            throw error::http::CompressionError{"Insert Count Increment exceeds unacknowledged insertion count"};
        }

        m_known_received_count += SIZE.value();
    }

    /* Request / Response stream! */

    // Encode

    // The pattern is 1Txxxxxx
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

        add_field(m_encoding_table->at<>(idx.value()));
    }

    // The pattern is 0001xxxx
    template <std::output_iterator<std::uint8_t> Out>
    void encode_post_base_indexed_field(UInt idx, Out out) {
        // We use a 4-bit prefix length because bits 4-7 are used for the integer value.
        shared_codec::raw::Atom<UInt, Width>::encode_int(
            idx, 4U, shared_codec::PrefixHelper::QPACK_POST_BASE_INDEXED_FIELD, out);

        add_field(m_encoding_table->at<true>(idx.value()));
    }

    // The pattern is 01NTxxxx
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
        shared_codec::raw::Atom<UInt, Width>::encode_string(m_huffman, value, out);


        push_helper<false, true>(idx.value(), value, idx.is_static(), 0);
    }

    // The pattern is 0000Nxxx
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
        shared_codec::raw::Atom<UInt, Width>::encode_string(m_huffman, value, out);


        push_helper<false, true, true>(idx.value(), value, idx.is_static(), 0);
    }

    // The pattern is 001NHxxx
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

        push_helper_new_entry<false, true>(name, value);
    }

    // Decode

    // The pattern is 1Txxxxxx
    void decode_indexed_field(std::span<const std::uint8_t> data, std::size_t &pos, std::size_t base) {
        const auto IDX = shared_codec::raw::Atom<UInt, Width>::template decode_int<1>(data, pos, 6U);
        if (IDX.value() == 0) {
            throw error::http::InvalidIndexError{IDX.value()};
        }

        add_field(m_decoding_table->at<>(IDX.value(), IDX.is_static(), base));
    }

    // The pattern is 0001xxxx
    void decode_post_base_indexed_field(std::span<const std::uint8_t> data, std::size_t &pos, std::size_t base) {
        const auto IDX = shared_codec::raw::Atom<UInt, Width>::template decode_int<1>(data, pos, 4U);
        if (IDX.value() == 0) {
            throw error::http::InvalidIndexError{IDX.value()};
        }

        add_field(m_decoding_table->at<true>(IDX.value(), IDX.is_static(), base));
    }

    // The pattern is 01NTxxxx
    void decode_indexed_name(std::span<const std::uint8_t> data, std::size_t &pos, std::size_t base) {
        const auto IDX = shared_codec::raw::Atom<UInt, Width>::template decode_int<2>(data, pos, 4U);
        const auto VALUE = shared_codec::raw::Atom<UInt, Width>::decode_string(m_huffman, data, pos);

        push_helper<>(IDX, VALUE, IDX.is_static(), base);
    }

    // The pattern is 0000Nxxx
    void decode_post_base_indexed_name(std::span<const std::uint8_t> data, std::size_t &pos, std::size_t base) {
        const auto IDX = shared_codec::raw::Atom<UInt, Width>::template decode_int<1>(data, pos, 3U);
        const auto VALUE = shared_codec::raw::Atom<UInt, Width>::decode_string(m_huffman, data, pos);

        push_helper<true, false, true>(IDX, VALUE, IDX.is_static(), base);
    }

    // The pattern is 001NHxxx
    void decode_new_field(std::span<const std::uint8_t> data, std::size_t &pos) {
        const auto NAME = shared_codec::raw::Atom<UInt, Width>::decode_string(m_huffman, data, pos, 3U);
        const auto VALUE = shared_codec::raw::Atom<UInt, Width>::decode_string(m_huffman, data, pos);

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
