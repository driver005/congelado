export module hpack;

import shared;

export import :types;
export import :table;

export namespace codec::hpack {

template <std::unsigned_integral UInt = std::uint32_t, int Width = 4>
    requires shared::DecodeWidth<Width>
class Hpack {
  public:
    explicit Hpack(std::size_t table_size = 4096)
        : m_decoding_table{table_size}, m_encoding_table{table_size}, m_huffman{} {}

    std::vector<std::uint8_t> encode(std::span<const shared::HeaderField> headers,
                                     bool use_auto_encoding_policy = true) {
        std::vector<std::uint8_t> out;
        auto it = std::back_inserter(out);

        for (auto &field : headers) {
            if (field.name() == "cookie") {
                encode_cookies(field.value(), it);
                continue;
            }

            const EncodePolicy policy =
                use_auto_encoding_policy ? policy_for(field.name()) : EncodePolicy::WithIndexing;

            shared::SearchResult r = m_encoding_table.search(field.name(), field.value());

            switch (policy) {
            case EncodePolicy::WithIndexing:
                if (r.is_full_match()) {
                    encode_indexed(r.index(), it);
                } else if (r.found()) {
                    encode_incremental(r.index(), field.value(), it);
                    m_encoding_table.insert(std::string{field.name()}, std::string{field.value()});
                } else {
                    encode_incremental_new(field.name(), field.value(), it);
                    m_encoding_table.insert(std::string{field.name()}, std::string{field.value()});
                }
                break;

            case EncodePolicy::WithoutIndexing:
                if (r.found()) {
                    encode_without_indexing(r.index(), field.value(), it);
                } else {
                    encode_without_indexing_new(field.name(), field.value(), it);
                }
                break;

            case EncodePolicy::NeverIndexed:
                if (r.found()) {
                    encode_never_indexed(r.index(), field.value(), it);
                } else {
                    encode_never_indexed_new(field.name(), field.value(), it);
                }
                break;
            }
        }

        return out;
    }

    std::vector<shared::HeaderField> decode(std::span<const std::uint8_t> data) {
        std::vector<shared::HeaderField> out;
        std::vector<std::string> cookies;
        std::size_t pos = 0;

        auto push_field = [&](shared::HeaderField field) {
            if (field.name() == "cookie")
                cookies.push_back(std::string{field.value()});
            else
                out.push_back(std::move(field));
        };

        while (pos < data.size()) {
            auto [rep_type, new_variant] = get_representation_type(data, pos);
            switch (rep_type) {
            case shared::PrefixHelper::IndexedField:
                push_field(decode_indexed(data, pos));
                break;
            case shared::PrefixHelper::LiteralWithIndexing: {
                shared::HeaderField field =
                    new_variant ? decode_incremental_new(data, pos) : decode_incremental(data, pos);
                m_decoding_table.insert(std::string{field.name()}, std::string{field.value()});
                push_field(std::move(field));
                break;
            }
            case shared::PrefixHelper::LiteralWithoutIndexing:
                push_field(new_variant ? decode_without_indexing_new(data, pos) : decode_without_indexing(data, pos));
                break;
            case shared::PrefixHelper::LiteralNeverIndexed:
                push_field(new_variant ? decode_never_indexed_new(data, pos) : decode_never_indexed(data, pos));
                break;
            case shared::PrefixHelper::DynamicTableSizeUpdate:
                m_decoding_table.set_max_size(decode_table_size_update(data, pos));
                break;
            }
        }

        if (!cookies.empty()) {
            std::string merged;
            for (bool first = true; auto &c : cookies) {
                if (!first)
                    merged += "; ";
                merged += std::move(c);
                first = false;
            }
            out.push_back(shared::HeaderField{"cookie", std::move(merged)});
        }

        return out;
    }

  private:
    // Representation type detection

    std::pair<shared::PrefixHelper, bool> get_representation_type(std::span<const std::uint8_t> data,
                                                                  std::size_t &pos) {
        std::uint8_t rep = data[pos];
        auto rep_type = shared::detect_representation(rep);
        bool new_variant = false;

        // IndexedField and DynamicTableSizeUpdate have no new-variant form.
        if (rep_type != shared::PrefixHelper::IndexedField &&
            rep_type != shared::PrefixHelper::DynamicTableSizeUpdate) {
            // All non-prefix bits are 0 → new variant (literal name follows).
            if (!(rep & ~std::to_underlying(rep_type))) {
                ++pos; // consume the type byte
                new_variant = true;
            }
        }

        return {rep_type, new_variant};
    }

    // Cookies

    template <std::output_iterator<std::uint8_t> Out>
    void encode_cookies(std::string_view value, Out out) {
        static constexpr std::string_view kSep = "; ";

        for (auto crumb_range : value | std::views::split(kSep)) {
            std::string_view crumb{crumb_range.begin(), crumb_range.end()};
            if (crumb.empty())
                continue;

            shared::SearchResult r = m_encoding_table.search("cookie", crumb);
            if (r.is_full_match()) {
                encode_indexed(r.index(), out);
            } else if (r.found()) {
                encode_incremental(r.index(), crumb, out);
                m_encoding_table.insert("cookie", std::string{crumb});
            } else {
                encode_incremental_new("cookie", crumb, out);
                m_encoding_table.insert("cookie", std::string{crumb});
            }
        }
    }

    // Encode primitives

    // 1xxxxxxx
    template <std::output_iterator<std::uint8_t> Out>
    void encode_indexed(UInt idx, Out out) {
        shared::raw::Atom<UInt, Width>::encode_int(idx, 7u, shared::PrefixHelper::IndexedField, out);
    }

    // 01xxxxxx + value string
    template <std::output_iterator<std::uint8_t> Out>
    void encode_incremental(UInt idx, std::string_view value, Out out) {
        shared::raw::Atom<UInt, Width>::encode_int(idx, 6u, shared::PrefixHelper::LiteralWithIndexing, out);
        shared::raw::Atom<UInt, Width>::encode_stirng(&m_huffman, value, out);
    }

    // 01000000 + name string + value string
    template <std::output_iterator<std::uint8_t> Out>
    void encode_incremental_new(std::string_view name, std::string_view value, Out out) {
        *out++ = std::to_underlying(shared::PrefixHelper::LiteralWithIndexing);
        shared::raw::Atom<UInt, Width>::encode_stirng(&m_huffman, name, out);
        shared::raw::Atom<UInt, Width>::encode_stirng(&m_huffman, value, out);
    }

    // 0000xxxx + value string
    template <std::output_iterator<std::uint8_t> Out>
    void encode_without_indexing(UInt idx, std::string_view value, Out out) {
        shared::raw::Atom<UInt, Width>::encode_int(idx, 4u, shared::PrefixHelper::LiteralWithoutIndexing, out);
        shared::raw::Atom<UInt, Width>::encode_stirng(&m_huffman, value, out);
    }

    // 00000000 + name string + value string
    template <std::output_iterator<std::uint8_t> Out>
    void encode_without_indexing_new(std::string_view name, std::string_view value, Out out) {
        *out++ = std::to_underlying(shared::PrefixHelper::LiteralWithoutIndexing);
        shared::raw::Atom<UInt, Width>::encode_stirng(&m_huffman, name, out);
        shared::raw::Atom<UInt, Width>::encode_stirng(&m_huffman, value, out);
    }

    // 0001xxxx + value string
    template <std::output_iterator<std::uint8_t> Out>
    void encode_never_indexed(UInt idx, std::string_view value, Out out) {
        shared::raw::Atom<UInt, Width>::encode_int(idx, 4u, shared::PrefixHelper::LiteralNeverIndexed, out);
        shared::raw::Atom<UInt, Width>::encode_stirng(&m_huffman, value, out);
    }

    // 00010000 + name string + value string
    template <std::output_iterator<std::uint8_t> Out>
    void encode_never_indexed_new(std::string_view name, std::string_view value, Out out) {
        *out++ = std::to_underlying(shared::PrefixHelper::LiteralNeverIndexed);
        shared::raw::Atom<UInt, Width>::encode_stirng(&m_huffman, name, out);
        shared::raw::Atom<UInt, Width>::encode_stirng(&m_huffman, value, out);
    }

    // 001xxxxx
    template <std::output_iterator<std::uint8_t> Out>
    void encode_table_size_update(UInt size, Out out) {
        shared::raw::Atom<UInt, Width>::encode_int(size, 5u, shared::PrefixHelper::DynamicTableSizeUpdate, out);
    }

    // Decode primitives

    // 1xxxxxxx
    shared::HeaderField decode_indexed(std::span<const std::uint8_t> data, std::size_t &pos) {
        const auto idx = shared::raw::Atom<UInt, Width>::decode_int(data, pos, 7u);
        if (idx == 0)
            throw error::http::InvalidIndexError{idx};
        auto [name, value] = m_decoding_table.at(idx);
        return {std::string{name}, std::string{value}};
    }

    // 01xxxxxx
    shared::HeaderField decode_incremental(std::span<const std::uint8_t> data, std::size_t &pos) {
        const auto idx = shared::raw::Atom<UInt, Width>::decode_int(data, pos, 6u);
        if (idx == 0)
            throw error::http::InvalidIndexError{idx};
        auto [name, unused] = m_decoding_table.at(idx);
        return {std::string{name}, shared::raw::Atom<UInt, Width>::decode_string(m_huffman, data, pos)};
    }

    // 01000000
    shared::HeaderField decode_incremental_new(std::span<const std::uint8_t> data, std::size_t &pos) {
        auto name = shared::raw::Atom<UInt, Width>::decode_string(m_huffman, data, pos);
        if (name.empty())
            throw error::http::EmptyNameError{};
        return {std::move(name), shared::raw::Atom<UInt, Width>::decode_string(m_huffman, data, pos)};
    }

    // 0000xxxx
    shared::HeaderField decode_without_indexing(std::span<const std::uint8_t> data, std::size_t &pos) {
        const auto idx = shared::raw::Atom<UInt, Width>::decode_int(data, pos, 4u);
        if (idx == 0)
            throw error::http::InvalidIndexError{idx};
        auto [name, unused] = m_decoding_table.at(idx);
        return {std::string{name}, shared::raw::Atom<UInt, Width>::decode_string(m_huffman, data, pos)};
    }

    // 00000000
    shared::HeaderField decode_without_indexing_new(std::span<const std::uint8_t> data, std::size_t &pos) {
        auto name = shared::raw::Atom<UInt, Width>::decode_string(m_huffman, data, pos);
        if (name.empty())
            throw error::http::EmptyNameError{};
        return {std::move(name), shared::raw::Atom<UInt, Width>::decode_string(m_huffman, data, pos)};
    }

    // 0001xxxx
    shared::HeaderField decode_never_indexed(std::span<const std::uint8_t> data, std::size_t &pos) {
        const auto idx = shared::raw::Atom<UInt, Width>::decode_int(data, pos, 4u);
        if (idx == 0)
            throw error::http::InvalidIndexError{idx};
        auto [name, unused] = m_decoding_table.at(idx);
        return {std::string{name}, shared::raw::Atom<UInt, Width>::decode_string(m_huffman, data, pos)};
    }

    // 00010000
    shared::HeaderField decode_never_indexed_new(std::span<const std::uint8_t> data, std::size_t &pos) {
        auto name = shared::raw::Atom<UInt, Width>::decode_string(m_huffman, data, pos);
        if (name.empty())
            throw error::http::EmptyNameError{};
        return {std::move(name), shared::raw::Atom<UInt, Width>::decode_string(m_huffman, data, pos)};
    }

    // 001xxxxx
    std::size_t decode_table_size_update(std::span<const std::uint8_t> data, std::size_t &pos) {
        const auto new_size = shared::raw::Atom<UInt, Width>::decode_int(data, pos, 5u);
        if (new_size > m_decoding_table.max_size())
            throw error::http::TableSizeError{new_size, m_decoding_table.max_size()};
        return new_size;
    }

    table::HeaderTable m_decoding_table;
    table::HeaderTable m_encoding_table;
    shared::huffman::Huffman<> m_huffman;
};

} // namespace codec::hpack
