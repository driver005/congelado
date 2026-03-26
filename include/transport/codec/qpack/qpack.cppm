export module qpack;

import shared;
import error;

export import :types;
export import :table;

export namespace codec::qpack {

template <std::unsigned_integral UInt = std::uint32_t, int Width = 4>
    requires shared::DecodeWidth<Width>
class QPack {
  public:
  private:
    /* Encoder / Decoder stream operations */

    // 1Txxxxxx
    template <std::output_iterator<std::uint8_t> Out>
    void encode_with_indexed_name(UInt idx, bool is_static, std::string_view value, Out out) {
        // The pattern is 1Txxxxxx
        // Prefix is 1 (0x80), and T is the 7th bit (0x40)
        auto prefix = shared::PrefixHelper::QpackIndexedField;
        // Set the T-bit if it's a static entry.
        if (is_static) {
            prefix |= 0x40;
        }

        // We use a 5-bit prefix length because bits 0-5 are used for the integer value.
        shared::raw::Atom<UInt, Width>::encode_int(idx, 5u, prefix, out);
        shared::raw::Atom<UInt, Width>::encode_string(m_huffman, value, out);
    }


    // 001xxxxx
    template <std::output_iterator<std::uint8_t> Out>
    void encode_table_size_update(UInt size, Out out) {
        shared::raw::Atom<UInt, Width>::encode_int(size, 5u, shared::PrefixHelper::QpackDynamicTableSizeUpdate, out);
    }

    // 1Txxxxxx
    UInt decode_with_indexed_name(std::span<const std::uint8_t> data, std::size_t &pos) {
        const auto idx = shared::raw::Atom<UInt, Width>::decode_int(data, pos, 7u);
        if (idx == 0)
            throw error::http::InvalidIndexError{idx};
        auto [name, value] = m_decoding_table.at(idx);
        return {std::string{name}, std::string{value}};
    }

    // 001xxxxx
    UInt decode_table_size_update(std::span<const std::uint8_t> data, std::size_t &pos) {
        const auto new_size = shared::raw::Atom<UInt, Width>::decode_int(data, pos, 5u);
        if (new_size > m_decoding_table.max_size())
            throw error::http::TableSizeError{new_size, m_decoding_table.max_size()};
        return new_size;
    }

    table::HeaderTable m_decoding_table;
    table::HeaderTable m_encoding_table;
    shared::huffman::Huffman<> m_huffman;
};
} // namespace codec::qpack
