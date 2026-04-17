export module io_codec_hpack;

export import :types;
export import :table;

import io_shared;
import io_codec_shared;


static constexpr std::string COOKIE_SEPARATOR = "; ";

export namespace io::codec::hpack {

template <std::unsigned_integral UInt = std::uint32_t, int Width = 4>
    requires shared_codec::DecodeWidth<Width>
class Hpack {
  public:
    explicit Hpack(HPackTable &decoding_table, HPackTable &encoding_table, shared::http::HttpRequest &req,
                   shared::http::HttpResponse &res)
        : m_decoding_table(decoding_table), m_encoding_table(encoding_table), m_huffman{}, m_request{req},
          m_response{res} {}

    template <std::output_iterator<std::uint8_t> Out>
    void encode(Out out, bool use_auto_encoding_policy = true) {
        for (const auto &field_variant : m_response.get().get_headers()) {
            std::visit(
                [&](const auto &ptr) {
                    if (!ptr)
                        return;

                    using FieldType = std::decay_t<decltype(*ptr)>;

                    if constexpr (std::is_same_v<FieldType, io::shared::http::HeaderField<true>>) {
                        if (ptr->get_name() == "cookie") {
                            encode_cookies(ptr->get_value(), out);
                            return;
                        }
                    }

                    const auto name = ptr->get_name();
                    const auto value = ptr->get_value();

                    const EncodePolicy policy =
                        use_auto_encoding_policy ? policy_for(name) : EncodePolicy::WithIndexing;

                    shared_codec::SearchResult result = m_encoding_table.get().search(name, value);

                    switch (policy) {
                    case EncodePolicy::WithIndexing:
                        if (result.is_full_match()) {
                            encode_indexed(result.index(), out);
                        } else if (result.found()) {
                            encode_incremental(result.index(), value, out);
                        } else {
                            encode_incremental_new(name, value, out);
                        }
                        break;

                    case EncodePolicy::WithoutIndexing:
                        if (result.found()) {
                            encode_without_indexing(result.index(), value, out);
                        } else {
                            encode_without_indexing_new(name, value, out);
                        }
                        break;

                    case EncodePolicy::NeverIndexed:
                        if (result.found()) {
                            encode_never_indexed(result.index(), value, out);
                        } else {
                            encode_never_indexed_new(name, value, out);
                        }
                        break;
                    }
                },
                field_variant);
        }
    }

    void decode(std::span<const std::uint8_t> data) {
        std::size_t pos = 0;

        while (pos < data.size()) {
            auto [rep_type, new_variant] = get_representation_type(data, pos);
            switch (rep_type) {
            case shared_codec::PrefixHelper::HpackIndexedField:
                decode_indexed(data, pos);
                break;
            case shared_codec::PrefixHelper::HpackLiteralWithIndexing: {
                new_variant ? decode_incremental_new(data, pos) : decode_incremental(data, pos);
                break;
            }
            case shared_codec::PrefixHelper::HpackLiteralWithoutIndexing:
                new_variant ? decode_without_indexing_new(data, pos) : decode_without_indexing(data, pos);
                break;
            case shared_codec::PrefixHelper::HpackLiteralNeverIndexed:
                new_variant ? decode_never_indexed_new(data, pos) : decode_never_indexed(data, pos);
                break;
            case shared_codec::PrefixHelper::HpackDynamicTableSizeUpdate:
                decode_table_size_update(data, pos);
                break;
            default:
                throw error::http::DecodeError{"Invalid HPACK representation type"};
            }
        }
    }

    void add_field(const shared::http::HeaderEntry &entry) {
        std::visit([&](const auto &f) { add_field(f); }, entry);
    }

    template <bool IsStatic>
    void add_field(std::shared_ptr<shared::http::HeaderField<IsStatic>> field) {
        m_request.get().insert(field);
    }

  private:
    // Representation type detection
    std::pair<shared_codec::PrefixHelper, bool> get_representation_type(std::span<const std::uint8_t> data,
                                                                        std::size_t &pos) {
        std::uint8_t rep = data[pos];
        auto rep_type = shared_codec::detect_representation_hpack(rep);
        bool new_variant = false;

        // IndexedField and DynamicTableSizeUpdate have no new-variant form.
        if (rep_type != shared_codec::PrefixHelper::HpackIndexedField &&
            rep_type != shared_codec::PrefixHelper::HpackDynamicTableSizeUpdate) {
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
        for (auto crumb_range : value | std::views::split(COOKIE_SEPARATOR)) {
            std::string_view crumb{crumb_range.begin(), crumb_range.end()};
            if (crumb.empty())
                continue;

            shared_codec::SearchResult result = m_encoding_table.get().search("cookie", crumb);

            if (result.is_full_match()) {
                encode_indexed(result.index(), out);
            } else if (result.found()) {
                encode_incremental(result.index(), crumb, out);
            } else {
                encode_incremental_new("cookie", crumb, out);
            }
        }
    }

    // Helper

    template <bool IsIndexable = true, bool IsDecoder = true>
    void push_helper(UInt idx, std::string_view value) {
        std::println("Pushing field with index {} and value '{}'", idx, value);
        if (idx == 0)
            throw error::http::InvalidIndexError{idx};

        auto field = m_decoding_table.get().at(idx);

        std::visit(
            [&](auto &&field_ptr) {
                if constexpr (IsIndexable) {
                    std::visit(
                        [&](auto &&inserted_field_ptr) {
                            if (inserted_field_ptr) {
                                m_request.get().insert(inserted_field_ptr);
                            }
                        },
                        [&] -> shared::http::HeaderEntry {
                            if constexpr (IsDecoder) {
                                auto ins_idx = m_decoding_table.get().insert(field_ptr->get_name(), value);
                                return m_decoding_table.get()[HPackStatic::STATIC_SIZE + 1 + ins_idx].value();
                            } else {
                                auto ins_idx = m_encoding_table.get().insert(field_ptr->get_name(), value);
                                return m_encoding_table.get()[HPackStatic::STATIC_SIZE + 1 + ins_idx].value();
                            }
                        }());

                } else {
                    m_request.get().insert(field_ptr->get_name(), value);
                }
            },
            field);
    }

    template <bool IsIndexable = true, bool IsDecoder = true>
    void push_helper_new_entry(std::string_view name, std::string_view value) {
        if (name.empty())
            throw error::http::EmptyNameError{};

        if constexpr (IsIndexable) {
            const auto new_field = [&] -> shared::http::HeaderEntry {
                if constexpr (IsDecoder) {
                    auto ins_idx = m_decoding_table.get().insert(name, value);
                    return m_decoding_table.get()[HPackStatic::STATIC_SIZE + 1 + ins_idx].value();
                } else {
                    auto ins_idx = m_encoding_table.get().insert(name, value);
                    return m_encoding_table.get()[HPackStatic::STATIC_SIZE + 1 + ins_idx].value();
                }
            }();

            std::visit(
                [&](auto &&inserted_field_ptr) {
                    if (inserted_field_ptr) {
                        m_request.get().insert(inserted_field_ptr);
                    }
                },
                new_field);
        } else {
            m_request.get().insert(name, value);
        }
    }

    // Encode primitives

    // 1xxxxxxx
    template <std::output_iterator<std::uint8_t> Out>
    void encode_indexed(UInt idx, Out out) {
        shared_codec::raw::Atom<UInt, Width>::encode_int(idx, 7u, shared_codec::PrefixHelper::HpackIndexedField, out);
    }

    // 01xxxxxx + value string
    template <std::output_iterator<std::uint8_t> Out>
    void encode_incremental(UInt idx, std::string_view value, Out out) {
        shared_codec::raw::Atom<UInt, Width>::encode_int(idx, 6u, shared_codec::PrefixHelper::HpackLiteralWithIndexing,
                                                         out);
        shared_codec::raw::Atom<UInt, Width>::encode_stirng(&m_huffman, value, out);

        push_helper<true, false>(idx, value);
    }

    // 01000000 + name string + value string
    template <std::output_iterator<std::uint8_t> Out>
    void encode_incremental_new(std::string_view name, std::string_view value, Out out) {
        *out++ = std::to_underlying(shared_codec::PrefixHelper::HpackLiteralWithIndexing);
        shared_codec::raw::Atom<UInt, Width>::encode_stirng(&m_huffman, name, out);
        shared_codec::raw::Atom<UInt, Width>::encode_stirng(&m_huffman, value, out);

        push_helper_new_entry<true, false>(name, value);
    }

    // 0000xxxx + value string
    template <std::output_iterator<std::uint8_t> Out>
    void encode_without_indexing(UInt idx, std::string_view value, Out out) {
        shared_codec::raw::Atom<UInt, Width>::encode_int(idx, 4u,
                                                         shared_codec::PrefixHelper::HpackLiteralWithoutIndexing, out);
        shared_codec::raw::Atom<UInt, Width>::encode_stirng(&m_huffman, value, out);
    }

    // 00000000 + name string + value string
    template <std::output_iterator<std::uint8_t> Out>
    void encode_without_indexing_new(std::string_view name, std::string_view value, Out out) {
        *out++ = std::to_underlying(shared_codec::PrefixHelper::HpackLiteralWithoutIndexing);
        shared_codec::raw::Atom<UInt, Width>::encode_stirng(&m_huffman, name, out);
        shared_codec::raw::Atom<UInt, Width>::encode_stirng(&m_huffman, value, out);
    }

    // 0001xxxx + value string
    template <std::output_iterator<std::uint8_t> Out>
    void encode_never_indexed(UInt idx, std::string_view value, Out out) {
        shared_codec::raw::Atom<UInt, Width>::encode_int(idx, 4u, shared_codec::PrefixHelper::HpackLiteralNeverIndexed,
                                                         out);
        shared_codec::raw::Atom<UInt, Width>::encode_stirng(&m_huffman, value, out);
    }

    // 00010000 + name string + value string
    template <std::output_iterator<std::uint8_t> Out>
    void encode_never_indexed_new(std::string_view name, std::string_view value, Out out) {
        *out++ = std::to_underlying(shared_codec::PrefixHelper::HpackLiteralNeverIndexed);
        shared_codec::raw::Atom<UInt, Width>::encode_stirng(&m_huffman, name, out);
        shared_codec::raw::Atom<UInt, Width>::encode_stirng(&m_huffman, value, out);
    }

    // 001xxxxx
    template <std::output_iterator<std::uint8_t> Out>
    void encode_table_size_update(UInt size, Out out) {
        m_encoding_table.get().set_max_size(size);

        shared_codec::raw::Atom<UInt, Width>::encode_int(size, 5u,
                                                         shared_codec::PrefixHelper::HpackDynamicTableSizeUpdate, out);
    }

    // Decode primitives

    // 1xxxxxxx
    void decode_indexed(std::span<const std::uint8_t> data, std::size_t &pos) {
        const auto idx = shared_codec::raw::Atom<UInt, Width>::decode_int(data, pos, 7u);
        if (idx.value() == 0)
            throw error::http::InvalidIndexError<UInt>{idx.value()};

        add_field(m_decoding_table.get().at(idx.value()));
    }

    // 01xxxxxx
    void decode_incremental(std::span<const std::uint8_t> data, std::size_t &pos) {
        const auto idx = shared_codec::raw::Atom<UInt, Width>::decode_int(data, pos, 6u);
        const auto value = shared_codec::raw::Atom<UInt, Width>::decode_string(m_huffman, data, pos);

        push_helper<>(idx.value(), value);
    }

    // 01000000
    void decode_incremental_new(std::span<const std::uint8_t> data, std::size_t &pos) {
        const auto name = shared_codec::raw::Atom<UInt, Width>::decode_string(m_huffman, data, pos);
        const auto value = shared_codec::raw::Atom<UInt, Width>::decode_string(m_huffman, data, pos);

        push_helper_new_entry<>(name, value);
    }

    // 0000xxxx
    void decode_without_indexing(std::span<const std::uint8_t> data, std::size_t &pos) {
        const auto idx = shared_codec::raw::Atom<UInt, Width>::decode_int(data, pos, 4u);
        const auto value = shared_codec::raw::Atom<UInt, Width>::decode_string(m_huffman, data, pos);

        push_helper<false>(idx.value(), value);
    }

    // 00000000
    void decode_without_indexing_new(std::span<const std::uint8_t> data, std::size_t &pos) {
        auto name = shared_codec::raw::Atom<UInt, Width>::decode_string(m_huffman, data, pos);
        const auto value = shared_codec::raw::Atom<UInt, Width>::decode_string(m_huffman, data, pos);

        push_helper_new_entry<false>(name, value);
    }

    // 0001xxxx
    void decode_never_indexed(std::span<const std::uint8_t> data, std::size_t &pos) {
        const auto idx = shared_codec::raw::Atom<UInt, Width>::decode_int(data, pos, 4u);
        const auto value = shared_codec::raw::Atom<UInt, Width>::decode_string(m_huffman, data, pos);

        push_helper<false>(idx.value(), value);
    }

    // 00010000
    void decode_never_indexed_new(std::span<const std::uint8_t> data, std::size_t &pos) {
        auto name = shared_codec::raw::Atom<UInt, Width>::decode_string(m_huffman, data, pos);
        const auto value = shared_codec::raw::Atom<UInt, Width>::decode_string(m_huffman, data, pos);

        push_helper_new_entry<false>(name, value);
    }

    // 001xxxxx
    void decode_table_size_update(std::span<const std::uint8_t> data, std::size_t &pos) {
        const auto new_size = shared_codec::raw::Atom<UInt, Width>::decode_int(data, pos, 5u);
        if (new_size.value() > m_decoding_table.get().max_size())
            throw error::http::TableSizeError{new_size.value(), m_decoding_table.get().max_size()};

        m_decoding_table.get().set_max_size(new_size.value());
    }

    std::reference_wrapper<HPackTable> m_decoding_table;
    std::reference_wrapper<HPackTable> m_encoding_table;
    shared_codec::huffman::Huffman<> m_huffman;
    std::reference_wrapper<shared::http::HttpRequest> m_request;
    std::reference_wrapper<shared::http::HttpResponse> m_response;
};

} // namespace io::codec::hpack
