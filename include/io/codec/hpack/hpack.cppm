module;
#include <cassert>
#include <iterator>
#include <print>
#include <ranges>
export module io_codec_hpack;

export import :types;
export import :table;

import interfaces;
import io_layer_shared;
import io_codec_shared;
import utils_codec;
import interfaces;
#ifdef CONGELADO_TEST
import boost.ut;
#endif


export namespace io::codec::hpack {

enum class HpackFlushReason : bool
{
    OVERFLOW,
    END
};

template<std::unsigned_integral UInt = std::uint32_t, int Width = 4>
    requires shared_codec::DecodeWidth<Width>
class HpackEncoder : public std::ranges::range_adaptor_closure<HpackEncoder<UInt, Width>>
{
public:
    using FlushCallback = std::function<void(std::span<const std::byte>, HpackFlushReason)>;

    /**
     * @brief Wires up an HPACK encoder over a fixed header list — nothing gets emitted until
     * operator() actually runs, this constructor just stashes the config. Bet.
     * @param table the encoding-side dynamic table; entries indexed or inserted while encoding
     * mutate this table in place, so it must stay in sync with the decoder's view.
     * @param headers the header entries to encode, in order.
     * @param max_frame_size the internal buffer's flush threshold in bytes — once `m_buf` fills
     * up mid-encode, `on_flush` gets called with HpackFlushReason::OVERFLOW.
     * @param on_flush callback invoked with the buffered bytes whenever the buffer overflows or
     * encoding finishes (HpackFlushReason::END).
     * @param use_auto_policy when true, per-field indexing policy is derived from policy_for()
     * (never-index sensitive headers like `cookie`/`authorization`); when false, everything
     * gets WithIndexing.
     * @param use_huffman when true, string literals get Huffman-coded; when false, they're
     * emitted raw.
     */
    explicit HpackEncoder(
        HPackTable& table,
        std::span<const interfaces::io::HeaderEntry> headers,
        std::size_t max_frame_size,
        FlushCallback on_flush,
        bool use_auto_policy = true,
        bool use_huffman = true
    ) noexcept :
        m_table{table},
        m_headers{headers},
        m_flush_size{max_frame_size},
        m_on_flush{std::move(on_flush)},
        m_use_auto_policy{use_auto_policy},
        m_use_huffman{use_huffman}
    {
    }

    /**
     * @brief Encodes every header entry into the internal buffer, flushing on overflow, then
     * fires a final END flush with whatever's left over — this is the whole encode run in one
     * call. Straight motion, no partial-state footguns since the buffer position resets before
     * and after.
     */
    void operator()() const
    {
        // Fresh run, fresh buffer — reset the cursor before touching anything.
        m_buf_pos = 0;
        // Encode every header in order; emit() handles overflow flushes internally.
        for (const auto& entry: m_headers) {
            encode_entry(entry);
        }
        // Whatever's left in the buffer goes out as the final END flush.
        m_on_flush({m_buf.data(), m_buf_pos}, HpackFlushReason::END);
        m_buf_pos = 0;
    }

private:
    /**
     * @brief Pushes a single encoded byte into the internal buffer, auto-flushing (with
     * HpackFlushReason::OVERFLOW) the instant the buffer hits `m_flush_size` — this is the one
     * choke point every encode_* helper drains through, so buffer bounds never actually get
     * violated. `m_buf` is fixed at 16384 bytes; `m_flush_size` just has to stay at or under
     * that or you're cooked.
     * @param byte the single byte to append.
     */
    void emit(std::byte byte) const
    {
        // Append the byte, then check if that just filled the buffer to its threshold.
        m_buf[m_buf_pos++] = byte; // FIXME(clang-tidy): unchecked operator[], consider .at();
                                   // non-constant array index
        if (m_buf_pos == m_flush_size) {
            // Full up — flush what we've got and reset for the next batch.
            m_on_flush({m_buf.data(), m_buf_pos}, HpackFlushReason::OVERFLOW);
            m_buf_pos = 0;
        }
    }

    /**
     * @brief Feeds every byte of a range through emit(), one at a time — the shared plumbing
     * every encode_* helper below rides on top of.
     * @tparam R the range type; must yield std::byte elements.
     * @param range the byte range to drain into the buffer.
     */
    template<typename R>
    void drain(R&& range) const
    {
        std::ranges::for_each(std::forward<R>(range), [this](std::byte byte) {
            emit(byte);
        });
    }

    /**
     * @brief Reinterprets a string_view's chars as a lazy range of std::byte, no copy involved
     * — just a views::transform over the same backing storage.
     * @param view the string to view as bytes.
     * @return a lazy byte range over `view`'s characters.
     */
    [[nodiscard]] static auto string_view_to_byte(std::string_view view) noexcept
    {
        return view | std::views::transform([](char character) {
                   return static_cast<std::byte>(character);
               });
    }

    /**
     * @brief Wraps a string literal in HPACK's string encoding (RFC 7541 §5.2) — length prefix
     * plus the raw or Huffman-coded payload, picked by `m_use_huffman`.
     * @param view the string literal to encode.
     * @return a lazy range of the encoded bytes (length prefix + payload), ready to drain().
     */
    // FIXME(clang-tidy): bugprone-exception-escape — returns a lazy views::transform +
    // EncodeStringAdaptor pipeline; not confident every path through the adaptor's
    // construction/iteration machinery is provably noexcept without tracing std::ranges
    // internals, so not stripping the noexcept or adding a speculative try/catch here.
    [[nodiscard]] auto encode_string(std::string_view view) const noexcept
    {
        return string_view_to_byte(view) |
               shared_codec::lowlevel::EncodeStringAdaptor<Width>{m_use_huffman};
    }

    /**
     * @brief Emits an Indexed Field representation (RFC 7541 §6.1, pattern `1xxxxxxx`) — a
     * single integer, no value string needed, since both name and value already live in a
     * table.
     * @param idx the 1-based unified table index the field is fully found at.
     */
    void encode_indexed(UInt idx) const
    {
        drain(
            idx | shared_codec::lowlevel::EncodeIntAdaptor<UInt>{
                      7U, shared_codec::PrefixHelper::HPACK_INDEXED_FIELD
                  }
        );
    }

    /**
     * @brief Emits a Literal Header Field with Incremental Indexing, name-indexed form
     * (RFC 7541 §6.2.1, pattern `01xxxxxx`) — the name comes from an existing table entry, the
     * value's a fresh literal, and the resulting pair gets inserted into the dynamic table so
     * later fields can reference it as a full match.
     * @param idx the 1-based unified table index the name is found at.
     * @param value the header value to encode as a literal and index.
     */
    void encode_incremental(UInt idx, std::string_view value) const
    {
        // Emit the index integer (6-bit prefix) followed by the value's encoded string.
        drain(
            std::views::concat(
                idx |
                    shared_codec::lowlevel::EncodeIntAdaptor<UInt>{
                        6U, shared_codec::PrefixHelper::HPACK_LITERAL_WITH_INDEXING
                    },
                encode_string(value)
            )
        );
        // Resolve the name behind `idx` and index the fresh pair into the dynamic table.
        std::visit(
            [&](const auto& ptr) {
                m_table.get().insert(ptr->get_name(), value);
            },
            m_table.get().at(idx)
        );
    }

    /**
     * @brief Emits a Literal Header Field with Incremental Indexing, new-name form
     * (RFC 7541 §6.2.1, pattern `01000000`) — both name and value are fresh literals, and the
     * pair gets inserted into the dynamic table for future reuse.
     * @param name the header name to encode as a literal and index.
     * @param value the header value to encode as a literal and index.
     */
    void encode_incremental_new(std::string_view name, std::string_view value) const
    {
        // Prefix byte, then the name and value as encoded strings — no index needed since
        // both are fresh literals.
        drain(
            std::views::concat(
                std::views::single(
                    std::byte{
                        std::to_underlying(shared_codec::PrefixHelper::HPACK_LITERAL_WITH_INDEXING)
                    }
                ),
                encode_string(name), encode_string(value)
            )
        );
        // Cache the pair for future reuse.
        m_table.get().insert(name, value);
    }

    /**
     * @brief Emits a Literal Header Field without Indexing, name-indexed form
     * (RFC 7541 §6.2.2, pattern `0000xxxx`) — no dynamic table mutation, this field's lowkey
     * not worth caching (one-off values, typically).
     * @param idx the 1-based unified table index the name is found at.
     * @param value the header value to encode as a literal.
     */
    void encode_without_indexing(UInt idx, std::string_view value) const
    {
        drain(
            std::views::concat(
                idx |
                    shared_codec::lowlevel::EncodeIntAdaptor<UInt>{
                        4U, shared_codec::PrefixHelper::HPACK_LITERAL_WITHOUT_INDEXING
                    },
                encode_string(value)
            )
        );
    }

    /**
     * @brief Emits a Literal Header Field without Indexing, new-name form (RFC 7541 §6.2.2,
     * pattern `00000000`) — fresh name and value, neither gets cached in the dynamic table.
     * @param name the header name to encode as a literal.
     * @param value the header value to encode as a literal.
     */
    void encode_without_indexing_new(std::string_view name, std::string_view value) const
    {
        drain(
            std::views::concat(
                std::views::single(
                    std::byte{std::to_underlying(
                        shared_codec::PrefixHelper::HPACK_LITERAL_WITHOUT_INDEXING
                    )}
                ),
                encode_string(name), encode_string(value)
            )
        );
    }

    /**
     * @brief Emits a Literal Header Field Never Indexed, name-indexed form (RFC 7541 §6.2.3,
     * pattern `0001xxxx`) — same wire shape as encode_without_indexing but flagged so
     * intermediaries must forward it unindexed too. This is the encoding path for sensitive
     * headers like `authorization`/`cookie` that policy_for() flags — no cap, don't skip this
     * for secrets.
     * @param idx the 1-based unified table index the name is found at.
     * @param value the header value to encode as a literal.
     */
    void encode_never_indexed(UInt idx, std::string_view value) const
    {
        drain(
            std::views::concat(
                idx |
                    shared_codec::lowlevel::EncodeIntAdaptor<UInt>{
                        4U, shared_codec::PrefixHelper::HPACK_LITERAL_NEVER_INDEXED
                    },
                encode_string(value)
            )
        );
    }

    /**
     * @brief Emits a Literal Header Field Never Indexed, new-name form (RFC 7541 §6.2.3,
     * pattern `00010000`) — fresh name and value, both flagged never-index for sensitive data
     * that shouldn't get cached anywhere along the chain.
     * @param name the header name to encode as a literal.
     * @param value the header value to encode as a literal.
     */
    void encode_never_indexed_new(std::string_view name, std::string_view value) const
    {
        drain(
            std::views::concat(
                std::views::single(
                    std::byte{
                        std::to_underlying(shared_codec::PrefixHelper::HPACK_LITERAL_NEVER_INDEXED)
                    }
                ),
                encode_string(name), encode_string(value)
            )
        );
    }

    /**
     * @brief Splits a `cookie` header's crumbs on `"; "` and encodes each one as its own
     * separate `cookie` field — RFC 7541 §7.2.1's recommended cookie-splitting so individual
     * crumbs can be indexed and reused independently instead of the whole cookie string having
     * to match byte-for-byte on every request. Empty crumbs get silently skipped.
     * @param value the raw, unsplit Cookie header value.
     */
    void encode_cookies(std::string_view value) const
    {
        static constexpr std::string_view SEP = "; ";
        // Split the raw cookie string on "; " and encode each crumb as its own field.
        std::ranges::for_each(value | std::views::split(SEP), [&](auto crumb_range) {
            std::string_view crumb{crumb_range.begin(), crumb_range.end()};
            // Skip empty crumbs (e.g. from a trailing separator) — nothing to encode.
            if (crumb.empty()) {
                return;
            }
            // Always WithIndexing for crumbs — search first so a repeat crumb reuses its
            // existing table entry instead of getting re-literal'd.
            encode_hpack_field(
                "cookie", crumb, EncodePolicy::WITH_INDEXING, m_table.get().search("cookie", crumb)
            );
        });
    }

    /**
     * @brief Dispatches a single name/value pair to the right wire representation based on its
     * EncodePolicy and what search() already found for it — full match goes indexed, name-only
     * match goes literal-with-indexed-name, no match goes literal-with-new-name. This is the
     * decision table every field's encoding funnels through.
     * @param name the header name to encode.
     * @param value the header value to encode.
     * @param policy which representation family to use (WithIndexing / WithoutIndexing /
     * NeverIndexed).
     * @param result the table search result for `name`/`value`, used to pick indexed vs.
     * literal forms.
     */
    void encode_hpack_field(
        std::string_view name,
        std::string_view value,
        EncodePolicy policy,
        shared_codec::SearchResult result
    ) const
    {
        switch (policy) {
            // Indexing allowed — pick the cheapest representation the search result affords.
            case EncodePolicy::WITH_INDEXING:
                if (result.is_full_match()) {
                    // Both name and value already in a table — just point at it.
                    encode_indexed(static_cast<UInt>(result.index()));
                } else if (result.found()) {
                    // Name's known, value's fresh — index the name, literal the value.
                    encode_incremental(static_cast<UInt>(result.index()), value);
                } else {
                    // Neither half is known — both go out as fresh literals.
                    encode_incremental_new(name, value);
                }
                break;
            // Caller doesn't want this cached, but the name can still ride an existing index.
            case EncodePolicy::WITHOUT_INDEXING:
                if (result.found()) {
                    encode_without_indexing(static_cast<UInt>(result.index()), value);
                } else {
                    encode_without_indexing_new(name, value);
                }
                break;
            // Sensitive header — same shape as WithoutIndexing, but flagged so intermediaries
            // must forward it unindexed too.
            case EncodePolicy::NEVER_INDEXED:
                if (result.found()) {
                    encode_never_indexed(static_cast<UInt>(result.index()), value);
                } else {
                    encode_never_indexed_new(name, value);
                }
                break;
        }
    }

    /**
     * @brief Encodes one HeaderEntry end-to-end — resolves its name (translating well-known
     * Tokens to strings for static fields), special-cases `cookie` to go through
     * encode_cookies() instead of a normal field encode, then picks an EncodePolicy (auto or
     * forced WithIndexing) and hands off to encode_hpack_field().
     * @param entry the header entry (static-token or dynamic-string field) to encode.
     */
    void encode_entry(const interfaces::io::HeaderEntry& entry) const
    {
        std::visit(
            [&](const auto& ptr) {
                using FieldType = std::decay_t<decltype(*ptr)>;
                // Static (Token-based) fields need their name translated to a string;
                // dynamic fields already store their name as one.
                std::string_view name;
                if constexpr (std::is_same_v<FieldType, interfaces::io::HeaderField<true>>) {
                    name = interfaces::io::types::token_to_string(ptr->get_name());
                } else {
                    name = ptr->get_name();
                }

                const std::string_view VALUE = ptr->get_value();

                // Cookie gets a special encoding path (crumb-splitting) instead of the
                // normal single-field flow — bail out early once that's handled.
                if constexpr (std::is_same_v<FieldType, interfaces::io::HeaderField<true>>) {
                    if (ptr->get_name() == interfaces::io::types::Token::COOKIE) {
                        encode_cookies(VALUE);
                        return;
                    }
                }

                // Normal field — resolve the indexing policy and hand off to the decision
                // table that picks the actual wire representation.
                const EncodePolicy POLICY =
                    m_use_auto_policy ? policy_for(name) : EncodePolicy::WITH_INDEXING;
                encode_hpack_field(name, VALUE, POLICY, m_table.get().search(name, VALUE));
            },
            entry
        );
    }

    mutable std::reference_wrapper<HPackTable> m_table;
    std::span<const interfaces::io::HeaderEntry> m_headers;
    std::size_t m_flush_size;
    FlushCallback m_on_flush;
    bool m_use_auto_policy;
    bool m_use_huffman;
    mutable std::array<std::byte, 16'384> m_buf{};
    mutable std::size_t m_buf_pos{0};
};

template<std::unsigned_integral UInt = std::uint32_t>
class HpackTableSizeUpdateAdaptor :
    public std::ranges::range_adaptor_closure<HpackTableSizeUpdateAdaptor<UInt>>
{
public:
    /**
     * @brief Wires up a range adaptor that applies a dynamic table size update as a side
     * effect of encoding it.
     * @param table the table to resize when the adaptor is invoked.
     */
    explicit HpackTableSizeUpdateAdaptor(HPackTable& table) noexcept :
        m_table{table}
    {
    }

    /**
     * @brief Resizes `m_table` to `size` and emits the corresponding Dynamic Table Size Update
     * representation (RFC 7541 §6.3, pattern `001xxxxx`) — table and wire stay in lockstep
     * since the resize happens right here, no separate call needed.
     * @param size the new dynamic table max size in bytes.
     * @return a lazy range of the encoded update bytes.
     */
    [[nodiscard]] auto operator()(UInt size) const
    {
        // Apply the resize to the table first, then produce the matching wire bytes —
        // table and wire never get to disagree since this is the only path in.
        m_table.get().set_max_size(size);
        return size | shared_codec::lowlevel::EncodeIntAdaptor<UInt>{
                          5U, shared_codec::PrefixHelper::HPACK_DYNAMIC_TABLE_SIZE_UPDATE
                      };
    }

private:
    std::reference_wrapper<HPackTable> m_table;
};

template<
    std::unsigned_integral UInt = std::uint32_t,
    int Width = 4,
    typename Target = interfaces::io::IRequest>
    requires shared_codec::DecodeWidth<Width>
class HpackDecoderAdapter :
    public std::ranges::range_adaptor_closure<HpackDecoderAdapter<UInt, Width, Target>>
{
public:
    /**
     * @brief Wires up an HPACK decoder over a fixed table and header sink — decoding writes
     * headers straight into `target` as it goes, no intermediate buffer of decoded fields.
     * @param table the decoding-side dynamic table, mutated in place as indexed/literal fields
     * get inserted.
     * @param target the request or response that decoded headers get pushed into via
     * add_header().
     */
    explicit HpackDecoderAdapter(HPackTable& table, Target& target) noexcept :
        m_table{table},
        m_target{target}
    {
    }

    /**
     * @brief Walks a byte range field-by-field, detecting each representation type and
     * dispatching to the matching decode_* helper until the whole range is consumed. Every
     * field push flows straight into `m_request`.
     * @tparam R a viewable range whose elements are std::byte.
     * @param range the encoded HPACK byte range to decode.
     * @return the total number of bytes consumed across all decoded fields.
     * @throws error::http::DecodeError if a byte's high bits don't match any known HPACK
     * representation type.
     */
    template<std::ranges::viewable_range R>
        requires std::same_as<std::ranges::range_value_t<R>, std::byte>
    [[nodiscard]] std::size_t operator()(R&& range) const
    {
        auto data = std::forward<R>(range);
        const auto TOTAL = static_cast<std::size_t>(std::ranges::distance(data));
        std::size_t offset = 0;


        // Walking `data` byte-by-byte is just how the loop advances — the actual
        // consumption is field-at-a-time via `offset`, so most iterations are no-ops
        // once `offset` has already caught up to (or past) the current position.
        std::ranges::for_each(data, [&](const std::byte&) {
            // Whole range's consumed, nothing left to decode.
            if (offset >= TOTAL) {
                return;
            }

            // Slice from the current offset and classify the field sitting there.
            auto slice = data | std::views::drop(offset);
            const auto [rep_type, is_new] = detect(slice);

            // Dispatch to the matching decode_* helper and advance offset by however
            // many bytes it consumed.
            offset += [&]() -> std::size_t {
                switch (rep_type) {
                    case shared_codec::PrefixHelper::HPACK_INDEXED_FIELD:
                        {
                            return decode_indexed(slice);
                        }

                    case shared_codec::PrefixHelper::HPACK_LITERAL_WITH_INDEXING:
                        {
                            return is_new ? decode_incremental_new(slice)
                                          : decode_incremental(slice);
                        }

                    case shared_codec::PrefixHelper::HPACK_LITERAL_NEVER_INDEXED:
                    case shared_codec::PrefixHelper::HPACK_LITERAL_WITHOUT_INDEXING:
                        {
                            return is_new ? decode_literal_new<false>(slice)
                                          : decode_literal<false>(slice, 4U);
                        }

                    case shared_codec::PrefixHelper::HPACK_DYNAMIC_TABLE_SIZE_UPDATE:
                        {
                            return decode_table_size_update(slice);
                        }

                    default:
                        {
                            // High bits didn't match any known representation — peer sent
                            // garbage, nothing to do but bail.
                            throw error::http::DecodeError{"invalid HPACK representation type"};
                        }
                }
            }();
        });

        return offset;
    }

private:
    /**
     * @brief Peeks the first byte of a field to classify its representation type and whether
     * it's the "new name" variant (literal name follows) versus the "indexed name" variant —
     * new-name detection works because those forms zero out every bit below the prefix mask,
     * so `!(byte & ~prefix_mask)` flags it. Doesn't consume anything; that's on the caller.
     * @tparam R a viewable range whose first element is the representation byte.
     * @param range the byte range starting at the field to classify.
     * @return the detected PrefixHelper representation type paired with whether it's the
     * new-name variant.
     */
    template<std::ranges::viewable_range R>
    [[nodiscard]] std::pair<shared_codec::PrefixHelper, bool> detect(R&& range) const
    {
        // Peek the representation byte and classify its high bits.
        const auto REP = std::forward<R>(range) | std::views::take(1) |
                         utils::codec::ReadBigEndianAdaptor<std::uint8_t>{};
        const auto REP_TYPE = shared_codec::detect_representation_hpack(REP);

        // Indexed Field and Dynamic Table Size Update have no "new name" variant, so
        // only check the zero-bits-below-prefix condition for the other types.
        const auto PREFIX_MASK = std::to_underlying(REP_TYPE);
        const bool IS_NEW =
            REP_TYPE != shared_codec::PrefixHelper::HPACK_INDEXED_FIELD &&
            REP_TYPE != shared_codec::PrefixHelper::HPACK_DYNAMIC_TABLE_SIZE_UPDATE &&
            !(REP & ~PREFIX_MASK);

        return {REP_TYPE, IS_NEW};
    }

    /**
     * @brief Resolves a name-indexed field to its literal value and either indexes it into the
     * dynamic table (adding it to the request afterward) or just pushes it straight to the
     * request unindexed — the fork that literal-with-indexing vs.
     * literal-without-indexing/never-indexed decoding needs.
     * @tparam Indexable when true, the name/value pair also gets inserted into the dynamic
     * table before being added to the request; when false, it's added directly with no table
     * mutation.
     * @param idx the 1-based unified table index the name is found at; 0 is never valid.
     * @param value the decoded header value.
     * @throws error::http::InvalidIndexError<UInt> if `idx` is 0.
     */
    template<bool Indexable = true>
    void push_helper(UInt idx, std::string_view value) const
    {
        // 0 is never a valid index on the wire — a peer sending it is straight busted.
        if (idx == 0) {
            throw error::http::InvalidIndexError<UInt>{idx};
        }
        std::visit(
            [&](const auto& ptr) {
                if constexpr (Indexable) {
                    // Incremental indexing — insert into the dynamic table, then push
                    // the freshly-inserted entry (not the lookup we started from) so
                    // it reflects whatever ended up actually stored.
                    const auto INS_IDX = m_table.get().insert(ptr->get_name(), value);
                    if (
                        auto entry = m_table.get()[HPackStatic::STATIC_SIZE + 1 + INS_IDX]
                    ) { // FIXME(clang-tidy): unchecked operator[], consider .at()
                        add_field(*entry);
                    }
                } else {
                    // Without-indexing / never-indexed — push straight to the target,
                    // no table mutation.
                    m_target.get().add_header(ptr->get_name(), value);
                }
            },
            m_table.get().at(idx)
        );
    }

    /**
     * @brief Same fork as push_helper(), but for the new-name variants where both name and
     * value arrived as fresh literals rather than a table lookup.
     * @tparam Indexable when true, the pair gets inserted into the dynamic table before being
     * added to the request; when false, it's added directly.
     * @param name the decoded header name.
     * @param value the decoded header value.
     * @throws error::http::EmptyNameError if `name` is empty.
     */
    template<bool Indexable = true>
    void push_helper_new(std::string_view name, std::string_view value) const
    {
        // RFC 7541 forbids an empty header name — no cap, straight reject it.
        if (name.empty()) {
            throw error::http::EmptyNameError{};
        }
        if constexpr (Indexable) {
            // Index the fresh pair, then push whatever actually landed in the table.
            const auto INS_IDX = m_table.get().insert(name, value);
            if (auto entry =
                    m_table
                        .get()[HPackStatic::STATIC_SIZE + 1 + INS_IDX]) { // FIXME(clang-tidy):
                                                                          // unchecked operator[],
                                                                          // consider .at()
                add_field(*entry);
            }
        } else {
            // No indexing — straight to the target.
            m_target.get().add_header(name, value);
        }
    }

    /**
     * @brief Pushes a resolved header entry onto the target, translating well-known Tokens
     * back to their string form for static fields since the sink deals in strings either way —
     * bet, no reason to leak the Token enum past this boundary.
     * @param entry the header entry (static-token or dynamic-string field) to add.
     */
    void add_field(const interfaces::io::HeaderEntry& entry) const
    {
        std::visit(
            [&](const auto& ptr) {
                using FieldType = std::decay_t<decltype(*ptr)>;
                // Static fields store a Token, not a string — translate it back before
                // it reaches the sink, which only deals in strings.
                if constexpr (std::is_same_v<FieldType, interfaces::io::HeaderField<true>>) {
                    m_target.get().add_header(
                        interfaces::io::types::token_to_string(ptr->get_name()), ptr->get_value()
                    );
                } else {
                    m_target.get().add_header(ptr->get_name(), ptr->get_value());
                }
            },
            entry
        );
    }

    // 1xxxxxxx
    /**
     * @brief Decodes an Indexed Field representation (RFC 7541 §6.1) — a single table index,
     * pushed straight to the request via add_field(). No cap, simplest field in the format.
     * @tparam R a viewable range starting at the representation byte.
     * @param range the byte range to decode from.
     * @return the number of bytes consumed by the integer.
     * @throws error::http::InvalidIndexError<UInt> if the decoded index is 0.
     */
    template<std::ranges::viewable_range R>
    [[nodiscard]] std::size_t decode_indexed(R&& range) const
    {
        // Pull the 7-bit-prefixed integer index off the wire.
        const auto IDX =
            std::forward<R>(range) | shared_codec::lowlevel::DecodeIntAdaptor<UInt>{7U};
        // 0 is never valid — reject before it hits the table lookup.
        if (IDX.value() == 0) {
            throw error::http::InvalidIndexError<UInt>{IDX.value()};
        }
        // Resolve and push straight through, no literal to decode for this form.
        add_field(m_table.get().at(IDX.value()));
        return IDX.consumed();
    }

    // 01xxxxxx + value
    /**
     * @brief Decodes a Literal Header Field with Incremental Indexing, name-indexed form
     * (RFC 7541 §6.2.1) — table-indexed name plus a literal value, indexed into the dynamic
     * table via push_helper<true>().
     * @tparam R a viewable range starting at the representation byte.
     * @param range the byte range to decode from.
     * @return the total bytes consumed (index integer + value string).
     */
    template<std::ranges::viewable_range R>
    [[nodiscard]] std::size_t decode_incremental(R&& range) const
    {
        auto data = std::forward<R>(range);
        // Decode the name index (6-bit prefix) first — `data` is reused right after for the
        // value string, so this first pass reads it as an lvalue rather than forwarding
        // (potentially moving-from) it; the actual forward happens only at the last use below.
        const auto IDX = data | shared_codec::lowlevel::DecodeIntAdaptor<UInt>{6U};
        auto [value, value_size] = std::forward<R>(data) | std::views::drop(IDX.consumed()) |
                                   shared_codec::lowlevel::DecodeStringAdaptor<Width>{};
        // Indexable — the pair gets cached in the dynamic table on the way to the request.
        push_helper<true>(IDX.value(), value);
        return IDX.consumed() + value_size;
    }

    // 01000000 + name + value
    /**
     * @brief Decodes a Literal Header Field with Incremental Indexing, new-name form
     * (RFC 7541 §6.2.1) — both name and value arrive as literals and get indexed together via
     * push_helper_new<true>().
     * @tparam R a viewable range starting at the representation byte.
     * @param range the byte range to decode from.
     * @return the total bytes consumed (name string + value string, including the leading
     * prefix byte folded into the name's count).
     */
    template<std::ranges::viewable_range R>
    [[nodiscard]] std::size_t decode_incremental_new(R&& range) const
    {
        auto data = std::forward<R>(range);
        // Skip the single prefix byte and decode the literal name string.
        auto [name, name_size] =
            data | std::views::drop(1) | shared_codec::lowlevel::DecodeStringAdaptor<Width>{};
        // Fold the prefix byte into name_size so the running offset stays correct.
        ++name_size;

        // Value string picks up right where the name left off.
        auto [value, value_size] = data | std::views::drop(name_size) |
                                   shared_codec::lowlevel::DecodeStringAdaptor<Width>{};

        // Both are fresh — index the pair together.
        push_helper_new<true>(name, value);
        return name_size + value_size;
    }

    // 0000xxxx / 0001xxxx + value
    /**
     * @brief Decodes a Literal Header Field without Indexing or Never Indexed, name-indexed
     * form (RFC 7541 §6.2.2/§6.2.3) — table-indexed name plus a literal value, pushed to the
     * request unindexed via push_helper<false>(). Shared between both variants since they only
     * differ in prefix width and semantics, not wire shape.
     * @tparam Indexable forwarded to push_helper — always false for both callers of this
     * function.
     * @tparam R a viewable range starting at the representation byte.
     * @param range the byte range to decode from.
     * @param prefix_bits the integer prefix width for the index (4 bits for both variants
     * here).
     * @return the total bytes consumed (index integer + value string).
     */
    template<bool Indexable, std::ranges::viewable_range R>
    [[nodiscard]] std::size_t decode_literal(R&& range, std::uint8_t prefix_bits) const
    {
        auto data = std::forward<R>(range);
        // Decode the name index, width given by the caller (4 bits for both variants
        // that reach this helper), then the literal value string right after it.
        const auto IDX = data | shared_codec::lowlevel::DecodeIntAdaptor<UInt>{prefix_bits};
        auto [value, value_size] = data | std::views::drop(IDX.consumed()) |
                                   shared_codec::lowlevel::DecodeStringAdaptor<Width>{};
        // Both callers pass Indexable = false — never cached, straight to the request.
        push_helper<Indexable>(IDX.value(), value);
        return IDX.consumed() + value_size;
    }

    // 00000000 / 00010000 + name + value
    /**
     * @brief Decodes a Literal Header Field without Indexing or Never Indexed, new-name form
     * (RFC 7541 §6.2.2/§6.2.3) — both name and value arrive as literals, pushed to the request
     * unindexed via push_helper_new<Indexable>().
     * @tparam Indexable forwarded to push_helper_new — always false for both callers here.
     * @tparam R a viewable range starting at the representation byte.
     * @param range the byte range to decode from.
     * @return the total bytes consumed (name string + value string).
     */
    template<bool Indexable, std::ranges::viewable_range R>
    [[nodiscard]] std::size_t decode_literal_new(R&& range) const
    {
        auto data = std::forward<R>(range);
        // Same prefix-byte-skip-then-decode-name shape as decode_incremental_new().
        auto [name, name_size] =
            data | std::views::drop(1) | shared_codec::lowlevel::DecodeStringAdaptor<Width>{};
        ++name_size;

        auto [value, value_size] = data | std::views::drop(name_size) |
                                   shared_codec::lowlevel::DecodeStringAdaptor<Width>{};

        // Both callers pass Indexable = false — pushed unindexed to the request.
        push_helper_new<Indexable>(name, value);
        return name_size + value_size;
    }

    // 001xxxxx
    /**
     * @brief Decodes a Dynamic Table Size Update instruction (RFC 7541 §6.3) and applies it —
     * the peer is telling us to shrink (or re-grow up to the negotiated max) the decoding
     * table's byte budget, evicting oldest entries as needed.
     * @tparam R a viewable range starting at the representation byte.
     * @param data the byte range to decode from.
     * @return the number of bytes consumed by the size integer.
     * @throws error::http::TableSizeError if the requested size exceeds the table's configured
     * maximum — the peer isn't allowed to grow past what was negotiated.
     */
    template<std::ranges::viewable_range R>
    [[nodiscard]] std::size_t decode_table_size_update(R&& data) const
    {
        // Decode the 5-bit-prefixed new size integer.
        const auto NEW_SIZE =
            std::forward<R>(data) | shared_codec::lowlevel::DecodeIntAdaptor<UInt>{5U};
        // Peer isn't allowed to grow past whatever was negotiated for this table.
        if (NEW_SIZE.value() > m_table.get().max_size()) {
            throw error::http::TableSizeError{NEW_SIZE.value(), m_table.get().max_size()};
        }
        // Within bounds — apply it, evicting oldest entries as needed under the hood.
        m_table.get().set_max_size(NEW_SIZE.value());
        return NEW_SIZE.consumed();
    }

    std::reference_wrapper<HPackTable> m_table;
    std::reference_wrapper<Target> m_target;
};

template<std::unsigned_integral UInt = std::uint32_t, int Width = 4>
    requires shared_codec::DecodeWidth<Width>
class Hpack
{
public:
    /**
     * @brief Wires up the top-level HPACK codec facade — separate encoding/decoding tables
     * since a connection's send and receive directions each track their own dynamic table
     * state independently, per RFC 7541.
     * @param decoding_table the dynamic table used when decoding inbound headers.
     * @param encoding_table the dynamic table used when encoding outbound headers.
     * @param req the request that decoded headers get written into (server-side inbound).
     * @param res the response whose headers get encoded on outbound, and the target decoded
     * headers get written into on the client side (inbound responses).
     * @param is_server true on a server session (inbound headers decode into `req`); false on a
     * client session (inbound headers decode into `res`).
     * @param use_huffman when true, encoded string literals get Huffman-coded.
     */
    explicit Hpack(
        HPackTable& decoding_table,
        HPackTable& encoding_table,
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        bool is_server,
        bool use_huffman = true
    ) noexcept :
        m_encoding_table{encoding_table},
        m_decoding_table{decoding_table},
        m_request{req},
        m_response{res},
        m_is_server{is_server},
        m_use_huffman{use_huffman}
    {
    }

    /**
     * @brief Builds an HpackEncoder over the response's current headers — call the returned
     * range-adaptor-closure to actually run the encode.
     * @tparam R unused range parameter (kept for interface symmetry with decode(), the encoder
     * pulls headers from `m_response` directly).
     * @param on_flush callback invoked with encoded bytes whenever the internal buffer
     * overflows or the encode finishes.
     * @param use_auto_policy when true, indexing policy is auto-derived per header
     * (never-index sensitive headers); when false, everything is WithIndexing.
     * @return a configured HpackEncoder, ready to invoke.
     */
    template<std::ranges::range R>
    [[nodiscard]] auto
    encode(HpackEncoder<UInt, Width>::FlushCallback&& on_flush, bool use_auto_policy = true)
    {
        return HpackEncoder<UInt, Width>{
            m_encoding_table, m_response.get().get_headers(), std::move(on_flush), use_auto_policy,
            m_use_huffman,
        };
    }

    /**
     * @brief Decodes a full HPACK byte block into the role-appropriate sink — `m_request` on a
     * server, `m_response` on a client — using the decoding table for index resolution and
     * inserts.
     * @tparam R a viewable range whose elements are std::byte.
     * @param data the encoded HPACK bytes to decode.
     * @return the total number of bytes consumed.
     * @throws error::http::DecodeError if a representation byte doesn't match any known type.
     */
    template<std::ranges::viewable_range R>
        requires std::same_as<std::ranges::range_value_t<R>, std::byte>
    [[nodiscard]] std::size_t decode(R&& data)
    {
        if (m_is_server) {
            return std::views::all(std::forward<R>(data)) |
                   HpackDecoderAdapter<UInt, Width, interfaces::io::IRequest>{
                       m_decoding_table, m_request
                   };
        }
        return std::views::all(std::forward<R>(data)) |
               HpackDecoderAdapter<UInt, Width, interfaces::io::IResponse>{
                   m_decoding_table, m_response
               };
    }

    /**
     * @brief Decodes a full HPACK byte block into an arbitrary caller-supplied request instead
     * of `m_request` — same shared decoding table (so dynamic-table state stays in sync with
     * the peer regardless of which request object the fields land in), just a different write
     * target. The seam a claimed HTTP/2 stream's trailers block decodes through, since
     * `m_request` is already the stream's primary (already-dispatched) request by the time
     * trailers arrive.
     * @tparam R a viewable range whose elements are std::byte.
     * @param target the request to decode the header fields into.
     * @param data the encoded HPACK bytes to decode.
     * @return the total number of bytes consumed.
     * @throws error::http::DecodeError if a representation byte doesn't match any known type.
     */
    template<std::ranges::viewable_range R>
        requires std::same_as<std::ranges::range_value_t<R>, std::byte>
    [[nodiscard]] std::size_t decode_into(interfaces::io::IRequest& target, R&& data)
    {
        return std::views::all(std::forward<R>(data)) |
               HpackDecoderAdapter<UInt, Width>{m_decoding_table, target};
    }

    /**
     * @brief Resizes the encoding table and produces the matching Dynamic Table Size Update
     * wire bytes so the peer's decoder stays in sync.
     * @param size the new dynamic table max size in bytes.
     * @return a lazy range of the encoded update bytes.
     */
    [[nodiscard]] auto encode_table_size_update(UInt size)
    {
        return size | HpackTableSizeUpdateAdaptor<UInt>{m_encoding_table};
    }

private:
    std::reference_wrapper<HPackTable> m_encoding_table;
    std::reference_wrapper<HPackTable> m_decoding_table;
    std::reference_wrapper<interfaces::io::IRequest> m_request;
    std::reference_wrapper<interfaces::io::IResponse> m_response;
    bool m_is_server;
    bool m_use_huffman;
};

} // namespace io::codec::hpack

#ifdef CONGELADO_TEST
namespace io::codec::hpack::tests {
using namespace boost::ut;

// Minimal decode target: HpackDecoderAdapter is templated on Target purely so it can be
// unit-tested without pulling in a real io_layer_http2 request (which imports this module,
// so a real one would be circular).
class FakeHeaderTarget
{
public:
    void add_header(std::string_view name, std::string_view value)
    {
        m_headers.emplace_back(std::string(name), std::string(value));
    }

    void add_header(interfaces::io::types::Token token, std::string_view value)
    {
        add_header(interfaces::io::types::token_to_string(token), value);
    }

    [[nodiscard]] const std::vector<std::pair<std::string, std::string>>&
    get_headers() const noexcept
    {
        return m_headers;
    }

private:
    std::vector<std::pair<std::string, std::string>> m_headers;
};

using TestDecoder = HpackDecoderAdapter<std::uint32_t, 4, FakeHeaderTarget>;

suite<"HpackEncoder"> hpack_encoder_suite = [] {
    "encodes an already-known static header as a single Indexed Field byte"_test = [] {
        HPackTable table;
        std::vector<interfaces::io::HeaderEntry> headers{
            std::make_shared<interfaces::io::HeaderField<true>>(
                interfaces::io::types::Token::METHOD, "GET"
            )
        };
        std::vector<std::byte> encoded;

        HpackEncoder<std::uint32_t, 4> encoder{
            table, headers, 16'384, [&](std::span<const std::byte> chunk, HpackFlushReason) {
                encoded.insert(encoded.end(), chunk.begin(), chunk.end());
            }
        };
        encoder();

        expect(encoded.size() == 1U);
        // Plain `==` on std::byte forces boost::ut's failure-diagnostic printer to instantiate
        // operator<<(ostream&, std::byte), which doesn't exist — comparing via std::to_integer
        // keeps this a plain integer comparison instead.
        expect(std::to_integer<int>(encoded[0]) == 0x82);
    };

    "encoding a fresh literal header inserts it into the dynamic table"_test = [] {
        HPackTable table;
        std::vector<interfaces::io::HeaderEntry> headers{
            std::make_shared<interfaces::io::HeaderField<false>>("x-custom", "value1")
        };
        std::vector<std::byte> encoded;

        HpackEncoder<std::uint32_t, 4> encoder{
            table,
            headers,
            16'384,
            [&](std::span<const std::byte> chunk, HpackFlushReason) {
                encoded.insert(encoded.end(), chunk.begin(), chunk.end());
            },
            false,
            false
        };
        encoder();

        expect(not encoded.empty());
        expect(table.dynamic_count() == 1U);
    };

    // Pins the exact-16384-byte flush boundary at the safe `max_frame_size` default (matching
    // `m_buf`'s fixed capacity). A `max_frame_size` above 16384 would make the `m_buf_pos ==
    // m_flush_size` check unreachable before `m_buf`'s array bound — a real OOB write — so that
    // case is deliberately NOT exercised here for test-binary safety.
    "a header block landing exactly at the 16384-byte boundary flushes right there"_test = [] {
        HPackTable table;
        // name="x" (2 encoded bytes: 1 length-prefix + 1 char), value is 16378 raw chars (3
        // length-prefix bytes since 16378 - 127 needs two 7-bit continuation octets, + 16378
        // char bytes = 16381), plus the 1 literal-with-indexing prefix byte: 1 + 2 + 16381 =
        // 16384 exactly.
        const std::string VALUE(16'378, 'a');
        std::vector<interfaces::io::HeaderEntry> headers{
            std::make_shared<interfaces::io::HeaderField<false>>("x", VALUE)
        };

        std::vector<std::pair<std::size_t, HpackFlushReason>> flushes;
        HpackEncoder<std::uint32_t, 4> encoder{
            table,
            headers,
            16'384,
            [&](std::span<const std::byte> chunk, HpackFlushReason reason) {
                flushes.emplace_back(chunk.size(), reason);
            },
            false,
            false
        };
        encoder();

        expect(flushes.size() == 2U) << fatal;
        expect(flushes[0].first == 16'384U);
        expect(flushes[0].second == HpackFlushReason::OVERFLOW);
        expect(flushes[1].first == 0U);
        expect(flushes[1].second == HpackFlushReason::END);
    };
};

suite<"HpackEncoder/HpackDecoderAdapter round-trip"> hpack_round_trip_suite = [] {
    "round-trips a repeated header through encode then decode"_test = [] {
        HPackTable encode_table;
        HPackTable decode_table;

        std::vector<interfaces::io::HeaderEntry> headers{
            std::make_shared<interfaces::io::HeaderField<false>>("x-custom", "value1"),
            std::make_shared<interfaces::io::HeaderField<false>>("x-custom", "value1"),
        };
        std::vector<std::byte> encoded;

        HpackEncoder<std::uint32_t, 4> encoder{
            encode_table,
            headers,
            16'384,
            [&](std::span<const std::byte> chunk, HpackFlushReason) {
                encoded.insert(encoded.end(), chunk.begin(), chunk.end());
            },
            false,
            false
        };
        encoder();

        FakeHeaderTarget target;
        TestDecoder decoder{decode_table, target};
        std::size_t consumed = decoder(encoded);

        expect(consumed == encoded.size());
        expect(target.get_headers().size() == 2U);
        expect(target.get_headers()[0].first == "x-custom");
        expect(target.get_headers()[0].second == "value1");
        expect(target.get_headers()[1].first == "x-custom");
        expect(target.get_headers()[1].second == "value1");
    };

    "round-trips a cookie header split into individual crumbs"_test = [] {
        HPackTable encode_table;
        HPackTable decode_table;

        std::vector<interfaces::io::HeaderEntry> headers{
            std::make_shared<interfaces::io::HeaderField<true>>(
                interfaces::io::types::Token::COOKIE, "a=1; b=2"
            ),
        };
        std::vector<std::byte> encoded;

        HpackEncoder<std::uint32_t, 4> encoder{
            encode_table,
            headers,
            16'384,
            [&](std::span<const std::byte> chunk, HpackFlushReason) {
                encoded.insert(encoded.end(), chunk.begin(), chunk.end());
            },
            false,
            false
        };
        encoder();

        FakeHeaderTarget target;
        TestDecoder decoder{decode_table, target};
        std::ignore = decoder(encoded);

        expect(target.get_headers().size() == 2U);
        expect(target.get_headers()[0].first == "cookie");
        expect(target.get_headers()[0].second == "a=1");
        expect(target.get_headers()[1].first == "cookie");
        expect(target.get_headers()[1].second == "b=2");
    };
};

suite<"HpackDecoderAdapter error paths"> hpack_decoder_error_suite = [] {
    "decoding an Indexed Field with index 0 throws"_test = [] {
        HPackTable table;
        FakeHeaderTarget target;
        TestDecoder decoder{table, target};
        std::vector<std::byte> encoded{std::byte{0x80}};

        expect(throws<error::http::InvalidIndexError<std::uint32_t>>([&] {
            std::ignore = decoder(encoded);
        }));
    };

    "decoding a table size update past the negotiated max throws"_test = [] {
        HPackTable table{100};
        FakeHeaderTarget target;
        TestDecoder decoder{table, target};

        auto size_range = 200U | shared_codec::lowlevel::EncodeIntAdaptor<std::uint32_t>{
                                     5U, shared_codec::PrefixHelper::HPACK_DYNAMIC_TABLE_SIZE_UPDATE
                                 };
        std::vector<std::byte> encoded(size_range.begin(), size_range.end());

        expect(throws<error::http::TableSizeError>([&] {
            std::ignore = decoder(encoded);
        }));
    };

    "decoding a within-budget table size update resizes the table"_test = [] {
        HPackTable table{100};
        FakeHeaderTarget target;
        TestDecoder decoder{table, target};

        auto size_range = 20U | shared_codec::lowlevel::EncodeIntAdaptor<std::uint32_t>{
                                    5U, shared_codec::PrefixHelper::HPACK_DYNAMIC_TABLE_SIZE_UPDATE
                                };
        std::vector<std::byte> encoded(size_range.begin(), size_range.end());
        std::ignore = decoder(encoded);

        expect(table.max_size() == 20U);
    };
};

suite<"HpackTableSizeUpdateAdaptor"> hpack_table_size_update_adaptor_suite = [] {
    "resizes the table and emits the matching wire bytes"_test = [] {
        HPackTable table;
        auto encoded_range = 20U | HpackTableSizeUpdateAdaptor<std::uint32_t>{table};
        std::vector<std::byte> encoded(encoded_range.begin(), encoded_range.end());

        expect(table.max_size() == 20U);
        expect(encoded.size() == 1U);
        expect(std::to_integer<int>(encoded[0]) == 0x34);
    };
};

} // namespace io::codec::hpack::tests
#endif

// export namespace io::codec::hpack {
// static constexpr std::string COOKIE_SEPARATOR = "; ";
//
// template <std::unsigned_integral UInt = std::uint32_t, int Width = 4>
//     requires shared_codec::DecodeWidth<Width>
// class Hpack {
//   public:
//     explicit Hpack(HPackTable &decoding_table, HPackTable &encoding_table,
//     shared::http::HttpRequest &req,
//                    shared::http::HttpResponse &res)
//         : m_decoding_table(decoding_table), m_encoding_table(encoding_table), m_huffman{},
//         m_request{req},
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
//                     if constexpr (std::is_same_v<FieldType,
//                     io::interfaces::io::HeaderField<true>>)
//                     {
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
//                         use_auto_encoding_policy ? policy_for(name) :
//                         EncodePolicy::WITH_INDEXING;
//
//                     shared_codec::SearchResult result = m_encoding_table.get().search(name,
//                     value);
//
//                     switch (policy) {
//                     case EncodePolicy::WITH_INDEXING:
//                         if (result.is_full_match()) {
//                             encode_indexed(result.index(), out);
//                         } else if (result.found()) {
//                             encode_incremental(result.index(), value, out);
//                         } else {
//                             encode_incremental_new(name, value, out);
//                         }
//                         break;
//
//                     case EncodePolicy::WITHOUT_INDEXING:
//                         if (result.found()) {
//                             encode_without_indexing(result.index(), value, out);
//                         } else {
//                             encode_without_indexing_new(name, value, out);
//                         }
//                         break;
//
//                     case EncodePolicy::NEVER_INDEXED:
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
//                 new_variant ? decode_without_indexing_new(data, pos) :
//                 decode_without_indexing(data, pos); break;
//             case shared_codec::PrefixHelper::HpackLiteralNeverIndexed:
//                 new_variant ? decode_never_indexed_new(data, pos) : decode_never_indexed(data,
//                 pos); break;
//             case shared_codec::PrefixHelper::HpackDynamicTableSizeUpdate:
//                 decode_table_size_update(data, pos);
//                 break;
//             default:
//                 throw error::http::DecodeError{"Invalid HPACK representation type"};
//             }
//         }
//     }
//
//     void add_field(const interfaces::io::HeaderEntry &entry) {
//         std::visit([&](const auto &f) { add_field(f); }, entry);
//     }
//
//     template <bool IsStatic>
//     void add_field(std::shared_ptr<interfaces::io::HeaderField<IsStatic>> field) {
//         m_request.get().insert(field);
//     }
//
//   private:
//     // Representation type detection
//     std::pair<shared_codec::PrefixHelper, bool> get_representation_type(std::span<const
//     std::uint8_t> data,
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
//                         [&] -> interfaces::io::HeaderEntry {
//                             if constexpr (IsDecoder) {
//                                 auto ins_idx =
//                                 m_decoding_table.get().insert(field_ptr->get_name(), value);
//                                 return m_decoding_table.get()[HPackStatic::STATIC_SIZE + 1 +
//                                 ins_idx].value();
//                             } else {
//                                 auto ins_idx =
//                                 m_encoding_table.get().insert(field_ptr->get_name(), value);
//                                 return m_encoding_table.get()[HPackStatic::STATIC_SIZE + 1 +
//                                 ins_idx].value();
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
//             const auto new_field = [&] -> interfaces::io::HeaderEntry {
//                 if constexpr (IsDecoder) {
//                     auto ins_idx = m_decoding_table.get().insert(name, value);
//                     return m_decoding_table.get()[HPackStatic::STATIC_SIZE + 1 +
//                     ins_idx].value();
//                 } else {
//                     auto ins_idx = m_encoding_table.get().insert(name, value);
//                     return m_encoding_table.get()[HPackStatic::STATIC_SIZE + 1 +
//                     ins_idx].value();
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
//         shared_codec::raw::Atom<UInt, Width>::encode_int(idx, 7u,
//         shared_codec::PrefixHelper::HpackIndexedField, out);
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
//         const auto value = shared_codec::raw::Atom<UInt, Width>::decode_string(m_huffman, data,
//         pos);
//
//         push_helper<>(idx.value(), value);
//     }
//
//     // 01000000
//     void decode_incremental_new(std::span<const std::uint8_t> data, std::size_t &pos) {
//         const auto name = shared_codec::raw::Atom<UInt, Width>::decode_string(m_huffman, data,
//         pos); const auto value = shared_codec::raw::Atom<UInt, Width>::decode_string(m_huffman,
//         data, pos);
//
//         push_helper_new_entry<>(name, value);
//     }
//
//     // 0000xxxx
//     void decode_without_indexing(std::span<const std::uint8_t> data, std::size_t &pos) {
//         const auto idx = shared_codec::raw::Atom<UInt, Width>::decode_int(data, pos, 4u);
//         const auto value = shared_codec::raw::Atom<UInt, Width>::decode_string(m_huffman, data,
//         pos);
//
//         push_helper<false>(idx.value(), value);
//     }
//
//     // 00000000
//     void decode_without_indexing_new(std::span<const std::uint8_t> data, std::size_t &pos) {
//         auto name = shared_codec::raw::Atom<UInt, Width>::decode_string(m_huffman, data, pos);
//         const auto value = shared_codec::raw::Atom<UInt, Width>::decode_string(m_huffman, data,
//         pos);
//
//         push_helper_new_entry<false>(name, value);
//     }
//
//     // 0001xxxx
//     void decode_never_indexed(std::span<const std::uint8_t> data, std::size_t &pos) {
//         const auto idx = shared_codec::raw::Atom<UInt, Width>::decode_int(data, pos, 4u);
//         const auto value = shared_codec::raw::Atom<UInt, Width>::decode_string(m_huffman, data,
//         pos);
//
//         push_helper<false>(idx.value(), value);
//     }
//
//     // 00010000
//     void decode_never_indexed_new(std::span<const std::uint8_t> data, std::size_t &pos) {
//         auto name = shared_codec::raw::Atom<UInt, Width>::decode_string(m_huffman, data, pos);
//         const auto value = shared_codec::raw::Atom<UInt, Width>::decode_string(m_huffman, data,
//         pos);
//
//         push_helper_new_entry<false>(name, value);
//     }
//
//     // 001xxxxx
//     void decode_table_size_update(std::span<const std::uint8_t> data, std::size_t &pos) {
//         const auto new_size = shared_codec::raw::Atom<UInt, Width>::decode_int(data, pos, 5u);
//         if (new_size.value() > m_decoding_table.get().max_size())
//             throw error::http::TableSizeError{new_size.value(),
//             m_decoding_table.get().max_size()};
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
