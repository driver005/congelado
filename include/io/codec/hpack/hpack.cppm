module;
#include <cassert>
#include <ranges>
export module io_codec_hpack;

export import :types;
export import :table;

import io_shared;
import io_layer_shared;
import io_codec_shared;
import utils_codec;
import interfaces;


export namespace io::codec::hpack {

enum class HpackFlushReason : bool { OVERFLOW, END };

template <std::unsigned_integral UInt = std::uint32_t, int Width = 4>
    requires shared_codec::DecodeWidth<Width>
class HpackEncoder : public std::ranges::range_adaptor_closure<HpackEncoder<UInt, Width>> {
  public:
    using FlushCallback = std::function<void(std::span<const std::byte>, HpackFlushReason)>;

    explicit HpackEncoder(HPackTable &table, std::span<const shared::http::HeaderEntry> headers,
                          std::size_t max_frame_size, FlushCallback on_flush, bool use_auto_policy = true,
                          bool use_huffman = true) noexcept
        : m_table{table}, m_headers{headers}, m_flush_size{max_frame_size}, m_on_flush{std::move(on_flush)},
          m_use_auto_policy{use_auto_policy}, m_use_huffman{use_huffman}, m_buf{}, m_buf_pos{0} {}

    template <std::ranges::viewable_range R>
    void operator()() const {
        m_buf_pos = 0;
        for (const auto &entry : m_headers) {
            encode_entry(entry);
        }
        m_on_flush({m_buf.data(), m_buf_pos}, HpackFlushReason::END);
        m_buf_pos = 0;
    }

  private:
    void emit(std::byte byte) const {
        m_buf[m_buf_pos++] = byte;
        if (m_buf_pos == m_flush_size) {
            m_on_flush({m_buf.data(), m_buf_pos}, HpackFlushReason::OVERFLOW);
            m_buf_pos = 0;
        }
    }

    template <typename R>
    void drain(R &&range) const {
        std::ranges::for_each(std::forward<R>(range), [this](std::byte byte) { emit(byte); });
    }

    [[nodiscard]] static auto string_view_to_byte(std::string_view view) noexcept {
        return view | std::views::transform([](char character) { return static_cast<std::byte>(character); });
    }

    [[nodiscard]] auto encode_string(std::string_view view) const noexcept {
        return string_view_to_byte(view) | shared_codec::lowlevel::EncodeStringAdaptor<Width>{m_use_huffman};
    }

    void encode_indexed(UInt idx) const {
        drain(idx |
              shared_codec::lowlevel::EncodeIntAdaptor<UInt>{7U, shared_codec::PrefixHelper::HPACK_INDEXED_FIELD});
    }

    void encode_incremental(UInt idx, std::string_view value) const {
        drain(std::views::concat(idx |
                                     shared_codec::lowlevel::EncodeIntAdaptor<UInt>{
                                         6U, shared_codec::PrefixHelper::HPACK_LITERAL_WITH_INDEXING},
                                 encode_string(value)));
        std::visit([&](const auto &ptr) { m_table.get().insert(ptr->get_name(), value); }, m_table.get().at(idx));
    }

    void encode_incremental_new(std::string_view name, std::string_view value) const {
        drain(std::views::concat(
            std::views::single(std::byte{std::to_underlying(shared_codec::PrefixHelper::HPACK_LITERAL_WITH_INDEXING)}),
            encode_string(name), encode_string(value)));
        m_table.get().insert(name, value);
    }

    void encode_without_indexing(UInt idx, std::string_view value) const {
        drain(std::views::concat(idx |
                                     shared_codec::lowlevel::EncodeIntAdaptor<UInt>{
                                         4U, shared_codec::PrefixHelper::HPACK_LITERAL_WITHOUT_INDEXING},
                                 encode_string(value)));
    }

    void encode_without_indexing_new(std::string_view name, std::string_view value) const {
        drain(std::views::concat(std::views::single(std::byte{
                                     std::to_underlying(shared_codec::PrefixHelper::HPACK_LITERAL_WITHOUT_INDEXING)}),
                                 encode_string(name), encode_string(value)));
    }

    void encode_never_indexed(UInt idx, std::string_view value) const {
        drain(std::views::concat(idx |
                                     shared_codec::lowlevel::EncodeIntAdaptor<UInt>{
                                         4U, shared_codec::PrefixHelper::HPACK_LITERAL_NEVER_INDEXED},
                                 encode_string(value)));
    }

    void encode_never_indexed_new(std::string_view name, std::string_view value) const {
        drain(std::views::concat(
            std::views::single(std::byte{std::to_underlying(shared_codec::PrefixHelper::HPACK_LITERAL_NEVER_INDEXED)}),
            encode_string(name), encode_string(value)));
    }

    void encode_cookies(std::string_view value) const {
        static constexpr std::string_view SEP = "; ";
        std::ranges::for_each(value | std::views::split(SEP), [&](auto crumb_range) {
            std::string_view crumb{crumb_range.begin(), crumb_range.end()};
            if (crumb.empty()) {
                return;
            }
            encode_hpack_field("cookie", crumb, EncodePolicy::WithIndexing, m_table.get().search("cookie", crumb));
        });
    }

    void encode_hpack_field(std::string_view name, std::string_view value, EncodePolicy policy,
                            shared_codec::SearchResult result) const {
        switch (policy) {
        case EncodePolicy::WithIndexing:
            if (result.is_full_match()) {
                encode_indexed(static_cast<UInt>(result.index()));
            } else if (result.found()) {
                encode_incremental(static_cast<UInt>(result.index()), value);
            } else {
                encode_incremental_new(name, value);
            }
            break;
        case EncodePolicy::WithoutIndexing:
            if (result.found()) {
                encode_without_indexing(static_cast<UInt>(result.index()), value);
            } else {
                encode_without_indexing_new(name, value);
            }
            break;
        case EncodePolicy::NeverIndexed:
            if (result.found()) {
                encode_never_indexed(static_cast<UInt>(result.index()), value);
            } else {
                encode_never_indexed_new(name, value);
            }
            break;
        }
    }

    void encode_entry(const shared::http::HeaderEntry &entry) const {
        std::visit(
            [&](const auto &ptr) {
                using FieldType = std::decay_t<decltype(*ptr)>;
                std::string_view name;
                if constexpr (std::is_same_v<FieldType, shared::http::HeaderField<true>>) {
                    name = shared::http::token_to_string(ptr->get_name());
                } else {
                    name = ptr->get_name();
                }

                const std::string_view VALUE = ptr->get_value();

                if constexpr (std::is_same_v<FieldType, shared::http::HeaderField<true>>) {
                    if (ptr->get_name() == shared::http::Token::COOKIE) {
                        encode_cookies(VALUE);
                        return;
                    }
                }

                const EncodePolicy POLICY = m_use_auto_policy ? policy_for(name) : EncodePolicy::WithIndexing;
                encode_hpack_field(name, VALUE, POLICY, m_table.get().search(name, VALUE));
            },
            entry);
    }

    mutable std::reference_wrapper<HPackTable> m_table;
    std::span<const shared::http::HeaderEntry> m_headers;
    std::size_t m_flush_size;
    FlushCallback m_on_flush;
    bool m_use_auto_policy;
    bool m_use_huffman;
    mutable std::array<std::byte, 16384> m_buf;
    mutable std::size_t m_buf_pos;
};

template <std::unsigned_integral UInt = std::uint32_t>
class HpackTableSizeUpdateAdaptor : public std::ranges::range_adaptor_closure<HpackTableSizeUpdateAdaptor<UInt>> {
  public:
    explicit HpackTableSizeUpdateAdaptor(HPackTable &table) noexcept : m_table{table} {}

    [[nodiscard]] auto operator()(UInt size) const {
        m_table.get().set_max_size(size);
        return size | shared_codec::lowlevel::EncodeIntAdaptor<UInt>{
                          5U, shared_codec::PrefixHelper::HPACK_DYNAMIC_TABLE_SIZE_UPDATE};
    }

  private:
    std::reference_wrapper<HPackTable> m_table;
};

template <typename Protocol, std::unsigned_integral UInt = std::uint32_t, int Width = 4>
    requires shared_codec::DecodeWidth<Width>
class HpackDecoderAdapter : public std::ranges::range_adaptor_closure<HpackDecoderAdapter<Protocol, UInt, Width>> {
  public:
    explicit HpackDecoderAdapter(HPackTable &table, interfaces::IRequest<Protocol> &req) noexcept
        : m_table{table}, m_request{req} {}

    template <std::ranges::viewable_range R>
        requires std::same_as<std::ranges::range_value_t<R>, std::byte>
    [[nodiscard]] std::size_t operator()(R &&range) const {
        auto data = std::forward<R>(range);
        const auto TOTAL = static_cast<std::size_t>(std::ranges::size(data));
        std::size_t offset = 0;

        std::ranges::for_each(data, [&](const std::byte &) {
            if (offset >= TOTAL) {
                return;
            }

            auto slice = data | std::views::drop(offset);
            const auto [rep_type, is_new] = detect(slice);

            offset += [&]() -> std::size_t {
                switch (rep_type) {
                case shared_codec::PrefixHelper::HPACK_INDEXED_FIELD: {
                    return decode_indexed(slice);
                }

                case shared_codec::PrefixHelper::HPACK_LITERAL_WITH_INDEXING: {
                    return is_new ? decode_incremental_new(slice) : decode_incremental(slice);
                }

                case shared_codec::PrefixHelper::HPACK_LITERAL_NEVER_INDEXED:
                case shared_codec::PrefixHelper::HPACK_LITERAL_WITHOUT_INDEXING: {
                    return is_new ? decode_literal_new<false>(slice) : decode_literal<false>(slice, 4U);
                }

                case shared_codec::PrefixHelper::HPACK_DYNAMIC_TABLE_SIZE_UPDATE: {
                    return decode_table_size_update(slice);
                }

                default: {
                    throw error::http::DecodeError{"invalid HPACK representation type"};
                }
                }
            }();
        });

        return offset;
    }

  private:
    template <std::ranges::viewable_range R>
    [[nodiscard]] std::pair<shared_codec::PrefixHelper, bool> detect(R &&range) const {
        const auto REP =
            std::forward<R>(range) | std::views::take(1) | utils::codec::ReadBigEndianAdaptor<std::uint8_t>{};
        const auto REP_TYPE = shared_codec::detect_representation_hpack(REP);

        const auto PREFIX_MASK = std::to_underlying(REP_TYPE);
        const bool IS_NEW = REP_TYPE != shared_codec::PrefixHelper::HPACK_INDEXED_FIELD &&
                            REP_TYPE != shared_codec::PrefixHelper::HPACK_DYNAMIC_TABLE_SIZE_UPDATE &&
                            !(REP & ~PREFIX_MASK);

        return {REP_TYPE, IS_NEW};
    }

    template <bool Indexable = true>
    void push_helper(UInt idx, std::string_view value) const {
        if (idx == 0) {
            throw error::http::InvalidIndexError<UInt>{idx};
        }
        std::visit(
            [&](const auto &ptr) {
                if constexpr (Indexable) {
                    const auto INS_IDX = m_table.get().insert(ptr->get_name(), value);
                    if (auto entry = m_table.get()[HPackStatic::STATIC_SIZE + 1 + INS_IDX]) {
                        add_field(*entry);
                    }
                } else {
                    m_request.get().add_header(ptr->get_name(), value);
                }
            },
            m_table.get().at(idx));
    }

    template <bool Indexable = true>
    void push_helper_new(std::string_view name, std::string_view value) const {
        if (name.empty()) {
            throw error::http::EmptyNameError{};
        }
        if constexpr (Indexable) {
            const auto INS_IDX = m_table.get().insert(name, value);
            if (auto entry = m_table.get()[HPackStatic::STATIC_SIZE + 1 + INS_IDX]) {
                add_field(*entry);
            }
        } else {
            m_request.get().add_header(name, value);
        }
    }

    void add_field(const shared::http::HeaderEntry &entry) const {
        std::visit(
            [&](const auto &ptr) {
                using FieldType = std::decay_t<decltype(*ptr)>;
                if constexpr (std::is_same_v<FieldType, shared::http::HeaderField<true>>) {
                    m_request.get().add_header(shared::http::token_to_string(ptr->get_name()), ptr->get_value());
                } else {
                    m_request.get().add_header(ptr->get_name(), ptr->get_value());
                }
            },
            entry);
    }

    // 1xxxxxxx
    template <std::ranges::viewable_range R>
    [[nodiscard]] std::size_t decode_indexed(R &&range) const {
        const auto IDX = std::forward<R>(range) | shared_codec::lowlevel::DecodeIntAdaptor<UInt>{7U};
        if (IDX.value() == 0) {
            throw error::http::InvalidIndexError<UInt>{IDX.value()};
        }
        add_field(m_table.get().at(IDX.value()));
        return IDX.consumed();
    }

    // 01xxxxxx + value
    template <std::ranges::viewable_range R>
    [[nodiscard]] std::size_t decode_incremental(R &&range) const {
        auto data = std::forward<R>(range);
        const auto IDX = std::forward<R>(data) | shared_codec::lowlevel::DecodeIntAdaptor<UInt>{6U};
        auto [value, value_size] = std::forward<R>(data) | std::views::drop(IDX.consumed()) |
                                   shared_codec::lowlevel::DecodeStringAdaptor<Width>{};
        push_helper<true>(IDX.value(), value);
        return IDX.consumed() + value_size;
    }

    // 01000000 + name + value
    template <std::ranges::viewable_range R>
    [[nodiscard]] std::size_t decode_incremental_new(R &&range) const {
        auto data = std::forward<R>(range);
        auto [name, name_size] = data | std::views::drop(1) | shared_codec::lowlevel::DecodeStringAdaptor<Width>{};
        ++name_size;

        auto [value, value_size] =
            data | std::views::drop(name_size) | shared_codec::lowlevel::DecodeStringAdaptor<Width>{};

        push_helper_new<true>(name, value);
        return name_size + value_size;
    }

    // 0000xxxx / 0001xxxx + value
    template <bool Indexable, std::ranges::viewable_range R>
    [[nodiscard]] std::size_t decode_literal(R &&range, std::uint8_t prefix_bits) const {
        auto data = std::forward<R>(range);
        const auto IDX = data | shared_codec::lowlevel::DecodeIntAdaptor<UInt>{prefix_bits};
        auto [value, value_size] =
            data | std::views::drop(IDX.consumed()) | shared_codec::lowlevel::DecodeStringAdaptor<Width>{};
        push_helper<Indexable>(IDX.value(), value);
        return IDX.consumed() + value_size;
    }

    // 00000000 / 00010000 + name + value
    template <bool Indexable, std::ranges::viewable_range R>
    [[nodiscard]] std::size_t decode_literal_new(R &&range) const {
        auto data = std::forward<R>(range);
        auto [name, name_size] = data | std::views::drop(1) | shared_codec::lowlevel::DecodeStringAdaptor<Width>{};
        ++name_size;

        auto [value, value_size] =
            data | std::views::drop(name_size) | shared_codec::lowlevel::DecodeStringAdaptor<Width>{};

        push_helper_new<Indexable>(name, value);
        return name_size + value_size;
    }

    // 001xxxxx
    template <std::ranges::viewable_range R>
    [[nodiscard]] std::size_t decode_table_size_update(R &&data) const {
        const auto NEW_SIZE = std::forward<R>(data) | shared_codec::lowlevel::DecodeIntAdaptor<UInt>{5U};
        if (NEW_SIZE.value() > m_table.get().max_size()) {
            throw error::http::TableSizeError{NEW_SIZE.value(), m_table.get().max_size()};
        }
        m_table.get().set_max_size(NEW_SIZE.value());
        return NEW_SIZE.consumed();
    }

    std::reference_wrapper<HPackTable> m_table;
    std::reference_wrapper<interfaces::IRequest<Protocol>> m_request;
};

template <typename Protocol, std::unsigned_integral UInt = std::uint32_t, int Width = 4>
    requires shared_codec::DecodeWidth<Width>
class Hpack {
  public:
    explicit Hpack(HPackTable &decoding_table, HPackTable &encoding_table, interfaces::IRequest<Protocol> &req,
                   interfaces::IResponse<Protocol> &res, bool use_huffman = true) noexcept
        : m_encoding_table{encoding_table}, m_decoding_table{decoding_table}, m_request{req}, m_response{res},
          m_use_huffman{use_huffman} {}

    template <std::ranges::range R>
    [[nodiscard]] auto encode(HpackEncoder<UInt, Width>::FlushCallback &&on_flush, bool use_auto_policy = true) {
        return HpackEncoder<UInt, Width>{
            m_encoding_table, m_response.get().get_headers(), std::move(on_flush), use_auto_policy, m_use_huffman,
        };
    }

    template <std::ranges::viewable_range R>
        requires std::same_as<std::ranges::range_value_t<R>, std::byte>
    [[nodiscard]] std::size_t decode(R &&data) {
        return std::views::all(std::forward<R>(data)) |
               HpackDecoderAdapter<Protocol, UInt, Width>{m_decoding_table, m_request};
    }

    [[nodiscard]] auto encode_table_size_update(UInt size) {
        return size | HpackTableSizeUpdateAdaptor<UInt>{m_encoding_table};
    }

  private:
    std::reference_wrapper<HPackTable> m_encoding_table;
    std::reference_wrapper<HPackTable> m_decoding_table;
    std::reference_wrapper<interfaces::IRequest<Protocol>> m_request;
    std::reference_wrapper<interfaces::IResponse<Protocol>> m_response;
    bool m_use_huffman;
};

} // namespace io::codec::hpack


// export namespace io::codec::hpack {
// static constexpr std::string COOKIE_SEPARATOR = "; ";
//
// template <std::unsigned_integral UInt = std::uint32_t, int Width = 4>
//     requires shared_codec::DecodeWidth<Width>
// class Hpack {
//   public:
//     explicit Hpack(HPackTable &decoding_table, HPackTable &encoding_table, shared::http::HttpRequest &req,
//                    shared::http::HttpResponse &res)
//         : m_decoding_table(decoding_table), m_encoding_table(encoding_table), m_huffman{}, m_request{req},
//           m_response{res} {}
//
//     template <std::output_iterator<std::uint8_t> Out>
//     void encode(Out out, bool use_auto_encoding_policy = true) {
//         for (const auto &field_variant : m_response.get().get_headers()) {
//             std::visit(
//                 [&](const auto &ptr) {
//                     if (!ptr)
//                         return;
//
//                     using FieldType = std::decay_t<decltype(*ptr)>;
//
//                     if constexpr (std::is_same_v<FieldType, io::shared::http::HeaderField<true>>) {
//                         if (ptr->get_name() == "cookie") {
//                             encode_cookies(ptr->get_value(), out);
//                             return;
//                         }
//                     }
//
//                     const auto name = ptr->get_name();
//                     const auto value = ptr->get_value();
//
//                     const EncodePolicy policy =
//                         use_auto_encoding_policy ? policy_for(name) : EncodePolicy::WithIndexing;
//
//                     shared_codec::SearchResult result = m_encoding_table.get().search(name, value);
//
//                     switch (policy) {
//                     case EncodePolicy::WithIndexing:
//                         if (result.is_full_match()) {
//                             encode_indexed(result.index(), out);
//                         } else if (result.found()) {
//                             encode_incremental(result.index(), value, out);
//                         } else {
//                             encode_incremental_new(name, value, out);
//                         }
//                         break;
//
//                     case EncodePolicy::WithoutIndexing:
//                         if (result.found()) {
//                             encode_without_indexing(result.index(), value, out);
//                         } else {
//                             encode_without_indexing_new(name, value, out);
//                         }
//                         break;
//
//                     case EncodePolicy::NeverIndexed:
//                         if (result.found()) {
//                             encode_never_indexed(result.index(), value, out);
//                         } else {
//                             encode_never_indexed_new(name, value, out);
//                         }
//                         break;
//                     }
//                 },
//                 field_variant);
//         }
//     }
//
//     void decode(std::span<const std::uint8_t> data) {
//         std::size_t pos = 0;
//
//         while (pos < data.size()) {
//             auto [rep_type, new_variant] = get_representation_type(data, pos);
//             switch (rep_type) {
//             case shared_codec::PrefixHelper::HpackIndexedField:
//                 decode_indexed(data, pos);
//                 break;
//             case shared_codec::PrefixHelper::HpackLiteralWithIndexing: {
//                 new_variant ? decode_incremental_new(data, pos) : decode_incremental(data, pos);
//                 break;
//             }
//             case shared_codec::PrefixHelper::HpackLiteralWithoutIndexing:
//                 new_variant ? decode_without_indexing_new(data, pos) : decode_without_indexing(data, pos);
//                 break;
//             case shared_codec::PrefixHelper::HpackLiteralNeverIndexed:
//                 new_variant ? decode_never_indexed_new(data, pos) : decode_never_indexed(data, pos);
//                 break;
//             case shared_codec::PrefixHelper::HpackDynamicTableSizeUpdate:
//                 decode_table_size_update(data, pos);
//                 break;
//             default:
//                 throw error::http::DecodeError{"Invalid HPACK representation type"};
//             }
//         }
//     }
//
//     void add_field(const shared::http::HeaderEntry &entry) {
//         std::visit([&](const auto &f) { add_field(f); }, entry);
//     }
//
//     template <bool IsStatic>
//     void add_field(std::shared_ptr<shared::http::HeaderField<IsStatic>> field) {
//         m_request.get().insert(field);
//     }
//
//   private:
//     // Representation type detection
//     std::pair<shared_codec::PrefixHelper, bool> get_representation_type(std::span<const std::uint8_t> data,
//                                                                         std::size_t &pos) {
//         std::uint8_t rep = data[pos];
//         auto rep_type = shared_codec::detect_representation_hpack(rep);
//         bool new_variant = false;
//
//         // IndexedField and DynamicTableSizeUpdate have no new-variant form.
//         if (rep_type != shared_codec::PrefixHelper::HpackIndexedField &&
//             rep_type != shared_codec::PrefixHelper::HpackDynamicTableSizeUpdate) {
//             // All non-prefix bits are 0 → new variant (literal name follows).
//             if (!(rep & ~std::to_underlying(rep_type))) {
//                 new_variant = true;
//             }
//         }
//
//         return {rep_type, new_variant};
//     }
//
//     // Cookies
//
//     template <std::output_iterator<std::uint8_t> Out>
//     void encode_cookies(std::string_view value, Out out) {
//         for (auto crumb_range : value | std::views::split(COOKIE_SEPARATOR)) {
//             std::string_view crumb{crumb_range.begin(), crumb_range.end()};
//             if (crumb.empty())
//                 continue;
//
//             shared_codec::SearchResult result = m_encoding_table.get().search("cookie", crumb);
//
//             if (result.is_full_match()) {
//                 encode_indexed(result.index(), out);
//             } else if (result.found()) {
//                 encode_incremental(result.index(), crumb, out);
//             } else {
//                 encode_incremental_new("cookie", crumb, out);
//             }
//         }
//     }
//
//     // Helper
//
//     template <bool IsIndexable = true, bool IsDecoder = true>
//     void push_helper(UInt idx, std::string_view value) {
//         std::println("Pushing field with index {} and value '{}'", idx, value);
//         if (idx == 0)
//             throw error::http::InvalidIndexError{idx};
//
//         auto field = m_decoding_table.get().at(idx);
//
//         std::visit(
//             [&](auto &&field_ptr) {
//                 if constexpr (IsIndexable) {
//                     std::visit(
//                         [&](auto &&inserted_field_ptr) {
//                             if (inserted_field_ptr) {
//                                 m_request.get().insert(inserted_field_ptr);
//                             }
//                         },
//                         [&] -> shared::http::HeaderEntry {
//                             if constexpr (IsDecoder) {
//                                 auto ins_idx = m_decoding_table.get().insert(field_ptr->get_name(), value);
//                                 return m_decoding_table.get()[HPackStatic::STATIC_SIZE + 1 + ins_idx].value();
//                             } else {
//                                 auto ins_idx = m_encoding_table.get().insert(field_ptr->get_name(), value);
//                                 return m_encoding_table.get()[HPackStatic::STATIC_SIZE + 1 + ins_idx].value();
//                             }
//                         }());
//
//                 } else {
//                     m_request.get().insert(field_ptr->get_name(), value);
//                 }
//             },
//             field);
//     }
//
//     template <bool IsIndexable = true, bool IsDecoder = true>
//     void push_helper_new_entry(std::string_view name, std::string_view value) {
//         if (name.empty())
//             throw error::http::EmptyNameError{};
//
//         if constexpr (IsIndexable) {
//             const auto new_field = [&] -> shared::http::HeaderEntry {
//                 if constexpr (IsDecoder) {
//                     auto ins_idx = m_decoding_table.get().insert(name, value);
//                     return m_decoding_table.get()[HPackStatic::STATIC_SIZE + 1 + ins_idx].value();
//                 } else {
//                     auto ins_idx = m_encoding_table.get().insert(name, value);
//                     return m_encoding_table.get()[HPackStatic::STATIC_SIZE + 1 + ins_idx].value();
//                 }
//             }();
//
//             std::visit(
//                 [&](auto &&inserted_field_ptr) {
//                     if (inserted_field_ptr) {
//                         m_request.get().insert(inserted_field_ptr);
//                     }
//                 },
//                 new_field);
//         } else {
//             m_request.get().insert(name, value);
//         }
//     }
//
//     // Encode primitives
//
//     // 1xxxxxxx
//     template <std::output_iterator<std::uint8_t> Out>
//     void encode_indexed(UInt idx, Out out) {
//         shared_codec::raw::Atom<UInt, Width>::encode_int(idx, 7u, shared_codec::PrefixHelper::HpackIndexedField,
//         out);
//     }
//
//     // 01xxxxxx + value string
//     template <std::output_iterator<std::uint8_t> Out>
//     void encode_incremental(UInt idx, std::string_view value, Out out) {
//         shared_codec::raw::Atom<UInt, Width>::encode_int(idx, 6u,
//         shared_codec::PrefixHelper::HpackLiteralWithIndexing,
//                                                          out);
//         shared_codec::raw::Atom<UInt, Width>::encode_stirng(&m_huffman, value, out);
//
//         push_helper<true, false>(idx, value);
//     }
//
//     // 01000000 + name string + value string
//     template <std::output_iterator<std::uint8_t> Out>
//     void encode_incremental_new(std::string_view name, std::string_view value, Out out) {
//         *out++ = std::to_underlying(shared_codec::PrefixHelper::HpackLiteralWithIndexing);
//         shared_codec::raw::Atom<UInt, Width>::encode_stirng(&m_huffman, name, out);
//         shared_codec::raw::Atom<UInt, Width>::encode_stirng(&m_huffman, value, out);
//
//         push_helper_new_entry<true, false>(name, value);
//     }
//
//     // 0000xxxx + value string
//     template <std::output_iterator<std::uint8_t> Out>
//     void encode_without_indexing(UInt idx, std::string_view value, Out out) {
//         shared_codec::raw::Atom<UInt, Width>::encode_int(idx, 4u,
//                                                          shared_codec::PrefixHelper::HpackLiteralWithoutIndexing,
//                                                          out);
//         shared_codec::raw::Atom<UInt, Width>::encode_stirng(&m_huffman, value, out);
//     }
//
//     // 00000000 + name string + value string
//     template <std::output_iterator<std::uint8_t> Out>
//     void encode_without_indexing_new(std::string_view name, std::string_view value, Out out) {
//         *out++ = std::to_underlying(shared_codec::PrefixHelper::HpackLiteralWithoutIndexing);
//         shared_codec::raw::Atom<UInt, Width>::encode_stirng(&m_huffman, name, out);
//         shared_codec::raw::Atom<UInt, Width>::encode_stirng(&m_huffman, value, out);
//     }
//
//     // 0001xxxx + value string
//     template <std::output_iterator<std::uint8_t> Out>
//     void encode_never_indexed(UInt idx, std::string_view value, Out out) {
//         shared_codec::raw::Atom<UInt, Width>::encode_int(idx, 4u,
//         shared_codec::PrefixHelper::HpackLiteralNeverIndexed,
//                                                          out);
//         shared_codec::raw::Atom<UInt, Width>::encode_stirng(&m_huffman, value, out);
//     }
//
//     // 00010000 + name string + value string
//     template <std::output_iterator<std::uint8_t> Out>
//     void encode_never_indexed_new(std::string_view name, std::string_view value, Out out) {
//         *out++ = std::to_underlying(shared_codec::PrefixHelper::HpackLiteralNeverIndexed);
//         shared_codec::raw::Atom<UInt, Width>::encode_stirng(&m_huffman, name, out);
//         shared_codec::raw::Atom<UInt, Width>::encode_stirng(&m_huffman, value, out);
//     }
//
//     // 001xxxxx
//     template <std::output_iterator<std::uint8_t> Out>
//     void encode_table_size_update(UInt size, Out out) {
//         m_encoding_table.get().set_max_size(size);
//
//         shared_codec::raw::Atom<UInt, Width>::encode_int(size, 5u,
//                                                          shared_codec::PrefixHelper::HpackDynamicTableSizeUpdate,
//                                                          out);
//     }
//
//     // Decode primitives
//
//     // 1xxxxxxx
//     void decode_indexed(std::span<const std::uint8_t> data, std::size_t &pos) {
//         const auto idx = shared_codec::raw::Atom<UInt, Width>::decode_int(data, pos, 7u);
//         if (idx.value() == 0)
//             throw error::http::InvalidIndexError<UInt>{idx.value()};
//
//         add_field(m_decoding_table.get().at(idx.value()));
//     }
//
//     // 01xxxxxx
//     void decode_incremental(std::span<const std::uint8_t> data, std::size_t &pos) {
//         const auto idx = shared_codec::raw::Atom<UInt, Width>::decode_int(data, pos, 6u);
//         const auto value = shared_codec::raw::Atom<UInt, Width>::decode_string(m_huffman, data, pos);
//
//         push_helper<>(idx.value(), value);
//     }
//
//     // 01000000
//     void decode_incremental_new(std::span<const std::uint8_t> data, std::size_t &pos) {
//         const auto name = shared_codec::raw::Atom<UInt, Width>::decode_string(m_huffman, data, pos);
//         const auto value = shared_codec::raw::Atom<UInt, Width>::decode_string(m_huffman, data, pos);
//
//         push_helper_new_entry<>(name, value);
//     }
//
//     // 0000xxxx
//     void decode_without_indexing(std::span<const std::uint8_t> data, std::size_t &pos) {
//         const auto idx = shared_codec::raw::Atom<UInt, Width>::decode_int(data, pos, 4u);
//         const auto value = shared_codec::raw::Atom<UInt, Width>::decode_string(m_huffman, data, pos);
//
//         push_helper<false>(idx.value(), value);
//     }
//
//     // 00000000
//     void decode_without_indexing_new(std::span<const std::uint8_t> data, std::size_t &pos) {
//         auto name = shared_codec::raw::Atom<UInt, Width>::decode_string(m_huffman, data, pos);
//         const auto value = shared_codec::raw::Atom<UInt, Width>::decode_string(m_huffman, data, pos);
//
//         push_helper_new_entry<false>(name, value);
//     }
//
//     // 0001xxxx
//     void decode_never_indexed(std::span<const std::uint8_t> data, std::size_t &pos) {
//         const auto idx = shared_codec::raw::Atom<UInt, Width>::decode_int(data, pos, 4u);
//         const auto value = shared_codec::raw::Atom<UInt, Width>::decode_string(m_huffman, data, pos);
//
//         push_helper<false>(idx.value(), value);
//     }
//
//     // 00010000
//     void decode_never_indexed_new(std::span<const std::uint8_t> data, std::size_t &pos) {
//         auto name = shared_codec::raw::Atom<UInt, Width>::decode_string(m_huffman, data, pos);
//         const auto value = shared_codec::raw::Atom<UInt, Width>::decode_string(m_huffman, data, pos);
//
//         push_helper_new_entry<false>(name, value);
//     }
//
//     // 001xxxxx
//     void decode_table_size_update(std::span<const std::uint8_t> data, std::size_t &pos) {
//         const auto new_size = shared_codec::raw::Atom<UInt, Width>::decode_int(data, pos, 5u);
//         if (new_size.value() > m_decoding_table.get().max_size())
//             throw error::http::TableSizeError{new_size.value(), m_decoding_table.get().max_size()};
//
//         m_decoding_table.get().set_max_size(new_size.value());
//     }
//
//     std::reference_wrapper<HPackTable> m_decoding_table;
//     std::reference_wrapper<HPackTable> m_encoding_table;
//     shared_codec::huffman::Huffman<> m_huffman;
//     std::reference_wrapper<shared::http::HttpRequest> m_request;
//     std::reference_wrapper<shared::http::HttpResponse> m_response;
// };
//
// } // namespace io::codec::hpack
