export module io_codec_shared:huffman;
import std;
import io_error;
import :types;

namespace io::shared_codec::huffman {

inline constexpr std::uint32_t SYM_COUNT = 257;
inline constexpr std::uint32_t SYM_EOS = 256;
inline constexpr std::uint32_t SYM_NONE = 0xFFFF'FFFEU;
inline constexpr std::uint32_t SYM_INVALID = 0xFFFF'FFFFU;

inline constexpr std::size_t TRIE_CAP = 600;
inline constexpr std::size_t TABLE_CAP = 9600;

inline constexpr std::pair<std::uint32_t, std::uint8_t> CODES[SYM_COUNT] = {
    /*   0 */ {0x1ff8, 13},     /*   1 */ {0x7fffd8, 23},
    /*   2 */ {0xfffffe2, 28},  /*   3 */ {0xfffffe3, 28},
    /*   4 */ {0xfffffe4, 28},  /*   5 */ {0xfffffe5, 28},
    /*   6 */ {0xfffffe6, 28},  /*   7 */ {0xfffffe7, 28},
    /*   8 */ {0xfffffe8, 28},  /*   9 */ {0xffffea, 24},
    /*  10 */ {0x3ffffffc, 30}, /*  11 */ {0xfffffe9, 28},
    /*  12 */ {0xfffffea, 28},  /*  13 */ {0x3ffffffd, 30},
    /*  14 */ {0xfffffeb, 28},  /*  15 */ {0xfffffec, 28},
    /*  16 */ {0xfffffed, 28},  /*  17 */ {0xfffffee, 28},
    /*  18 */ {0xfffffef, 28},  /*  19 */ {0xffffff0, 28},
    /*  20 */ {0xffffff1, 28},  /*  21 */ {0xffffff2, 28},
    /*  22 */ {0x3ffffffe, 30}, /*  23 */ {0xffffff3, 28},
    /*  24 */ {0xffffff4, 28},  /*  25 */ {0xffffff5, 28},
    /*  26 */ {0xffffff6, 28},  /*  27 */ {0xffffff7, 28},
    /*  28 */ {0xffffff8, 28},  /*  29 */ {0xffffff9, 28},
    /*  30 */ {0xffffffa, 28},  /*  31 */ {0xffffffb, 28},
    /* ' '*/ {0x14, 6},         /* '!'*/ {0x3f8, 10},
    /* '"'*/ {0x3f9, 10},       /* '#'*/ {0xffa, 12},
    /* '$'*/ {0x1ff9, 13},      /* '%'*/ {0x15, 6},
    /* '&'*/ {0xf8, 8},         /* '''*/ {0x7fa, 11},
    /* '('*/ {0x3fa, 10},       /* ')'*/ {0x3fb, 10},
    /* '*'*/ {0xf9, 8},         /* '+'*/ {0x7fb, 11},
    /* ','*/ {0xfa, 8},         /* '-'*/ {0x16, 6},
    /* '.'*/ {0x17, 6},         /* '/'*/ {0x18, 6},
    /* '0'*/ {0x0, 5},          /* '1'*/ {0x1, 5},
    /* '2'*/ {0x2, 5},          /* '3'*/ {0x19, 6},
    /* '4'*/ {0x1a, 6},         /* '5'*/ {0x1b, 6},
    /* '6'*/ {0x1c, 6},         /* '7'*/ {0x1d, 6},
    /* '8'*/ {0x1e, 6},         /* '9'*/ {0x1f, 6},
    /* ':'*/ {0x5c, 7},         /* ';'*/ {0xfb, 8},
    /* '<'*/ {0x7ffc, 15},      /* '='*/ {0x20, 6},
    /* '>'*/ {0xffb, 12},       /* '?'*/ {0x3fc, 10},
    /* '@'*/ {0x1ffa, 13},      /* 'A'*/ {0x21, 6},
    /* 'B'*/ {0x5d, 7},         /* 'C'*/ {0x5e, 7},
    /* 'D'*/ {0x5f, 7},         /* 'E'*/ {0x60, 7},
    /* 'F'*/ {0x61, 7},         /* 'G'*/ {0x62, 7},
    /* 'H'*/ {0x63, 7},         /* 'I'*/ {0x64, 7},
    /* 'J'*/ {0x65, 7},         /* 'K'*/ {0x66, 7},
    /* 'L'*/ {0x67, 7},         /* 'M'*/ {0x68, 7},
    /* 'N'*/ {0x69, 7},         /* 'O'*/ {0x6a, 7},
    /* 'P'*/ {0x6b, 7},         /* 'Q'*/ {0x6c, 7},
    /* 'R'*/ {0x6d, 7},         /* 'S'*/ {0x6e, 7},
    /* 'T'*/ {0x6f, 7},         /* 'U'*/ {0x70, 7},
    /* 'V'*/ {0x71, 7},         /* 'W'*/ {0x72, 7},
    /* 'X'*/ {0xfc, 8},         /* 'Y'*/ {0x73, 7},
    /* 'Z'*/ {0xfd, 8},         /* '['*/ {0x1ffb, 13},
    /* '\'*/ {0x7fff0, 19},     /* ']'*/ {0x1ffc, 13},
    /* '^'*/ {0x3ffc, 14},      /* '_'*/ {0x22, 6},
    /* '`'*/ {0x7ffd, 15},      /* 'a'*/ {0x3, 5},
    /* 'b'*/ {0x23, 6},         /* 'c'*/ {0x4, 5},
    /* 'd'*/ {0x24, 6},         /* 'e'*/ {0x5, 5},
    /* 'f'*/ {0x25, 6},         /* 'g'*/ {0x26, 6},
    /* 'h'*/ {0x27, 6},         /* 'i'*/ {0x6, 5},
    /* 'j'*/ {0x74, 7},         /* 'k'*/ {0x75, 7},
    /* 'l'*/ {0x28, 6},         /* 'm'*/ {0x29, 6},
    /* 'n'*/ {0x2a, 6},         /* 'o'*/ {0x7, 5},
    /* 'p'*/ {0x2b, 6},         /* 'q'*/ {0x76, 7},
    /* 'r'*/ {0x2c, 6},         /* 's'*/ {0x8, 5},
    /* 't'*/ {0x9, 5},          /* 'u'*/ {0x2d, 6},
    /* 'v'*/ {0x77, 7},         /* 'w'*/ {0x78, 7},
    /* 'x'*/ {0x79, 7},         /* 'y'*/ {0x7a, 7},
    /* 'z'*/ {0x7b, 7},         /* '{'*/ {0x7ffe, 15},
    /* '|'*/ {0x7fc, 11},       /* '}'*/ {0x3ffd, 14},
    /* '~'*/ {0x1ffd, 13},
    /* 127*/ {0xffffffc, 28},
    /* 128*/ {0xfffe6, 20},     /* 129*/ {0x3fffd2, 22},
    /* 130*/ {0xfffe7, 20},     /* 131*/ {0xfffe8, 20},
    /* 132*/ {0x3fffd3, 22},    /* 133*/ {0x3fffd4, 22},
    /* 134*/ {0x3fffd5, 22},    /* 135*/ {0x7fffd9, 23},
    /* 136*/ {0x3fffd6, 22},    /* 137*/ {0x7fffda, 23},
    /* 138*/ {0x7fffdb, 23},    /* 139*/ {0x7fffdc, 23},
    /* 140*/ {0x7fffdd, 23},    /* 141*/ {0x7fffde, 23},
    /* 142*/ {0xffffeb, 24},    /* 143*/ {0x7fffdf, 23},
    /* 144*/ {0xffffec, 24},    /* 145*/ {0xffffed, 24},
    /* 146*/ {0x3fffd7, 22},    /* 147*/ {0x7fffe0, 23},
    /* 148*/ {0xffffee, 24},    /* 149*/ {0x7fffe1, 23},
    /* 150*/ {0x7fffe2, 23},    /* 151*/ {0x7fffe3, 23},
    /* 152*/ {0x7fffe4, 23},    /* 153*/ {0x1fffdc, 21},
    /* 154*/ {0x3fffd8, 22},    /* 155*/ {0x7fffe5, 23},
    /* 156*/ {0x3fffd9, 22},    /* 157*/ {0x7fffe6, 23},
    /* 158*/ {0x7fffe7, 23},    /* 159*/ {0xffffef, 24},
    /* 160*/ {0x3fffda, 22},    /* 161*/ {0x1fffdd, 21},
    /* 162*/ {0xfffe9, 20},     /* 163*/ {0x3fffdb, 22},
    /* 164*/ {0x3fffdc, 22},    /* 165*/ {0x7fffe8, 23},
    /* 166*/ {0x7fffe9, 23},    /* 167*/ {0x1fffde, 21},
    /* 168*/ {0x7fffea, 23},    /* 169*/ {0x3fffdd, 22},
    /* 170*/ {0x3fffde, 22},    /* 171*/ {0xfffff0, 24},
    /* 172*/ {0x1fffdf, 21},    /* 173*/ {0x3fffdf, 22},
    /* 174*/ {0x7fffeb, 23},    /* 175*/ {0x7fffec, 23},
    /* 176*/ {0x1fffe0, 21},    /* 177*/ {0x1fffe1, 21},
    /* 178*/ {0x3fffe0, 22},    /* 179*/ {0x1fffe2, 21},
    /* 180*/ {0x7fffed, 23},    /* 181*/ {0x3fffe1, 22},
    /* 182*/ {0x7fffee, 23},    /* 183*/ {0x7fffef, 23},
    /* 184*/ {0xfffea, 20},     /* 185*/ {0x3fffe2, 22},
    /* 186*/ {0x3fffe3, 22},    /* 187*/ {0x3fffe4, 22},
    /* 188*/ {0x7ffff0, 23},    /* 189*/ {0x3fffe5, 22},
    /* 190*/ {0x3fffe6, 22},    /* 191*/ {0x7ffff1, 23},
    /* 192*/ {0x3ffffe0, 26},   /* 193*/ {0x3ffffe1, 26},
    /* 194*/ {0xfffeb, 20},     /* 195*/ {0x7fff1, 19},
    /* 196*/ {0x3fffe7, 22},    /* 197*/ {0x7ffff2, 23},
    /* 198*/ {0x3fffe8, 22},    /* 199*/ {0x1ffffec, 25},
    /* 200*/ {0x3ffffe2, 26},   /* 201*/ {0x3ffffe3, 26},
    /* 202*/ {0x3ffffe4, 26},   /* 203*/ {0x7ffffde, 27},
    /* 204*/ {0x7ffffdf, 27},   /* 205*/ {0x3ffffe5, 26},
    /* 206*/ {0xfffff1, 24},    /* 207*/ {0x1ffffed, 25},
    /* 208*/ {0x7fff2, 19},     /* 209*/ {0x1fffe3, 21},
    /* 210*/ {0x3ffffe6, 26},   /* 211*/ {0x7ffffe0, 27},
    /* 212*/ {0x7ffffe1, 27},   /* 213*/ {0x3ffffe7, 26},
    /* 214*/ {0x7ffffe2, 27},   /* 215*/ {0xfffff2, 24},
    /* 216*/ {0x1fffe4, 21},    /* 217*/ {0x1fffe5, 21},
    /* 218*/ {0x3ffffe8, 26},   /* 219*/ {0x3ffffe9, 26},
    /* 220*/ {0xffffffd, 28},   /* 221*/ {0x7ffffe3, 27},
    /* 222*/ {0x7ffffe4, 27},   /* 223*/ {0x7ffffe5, 27},
    /* 224*/ {0xfffec, 20},     /* 225*/ {0xfffff3, 24},
    /* 226*/ {0xfffed, 20},     /* 227*/ {0x1fffe6, 21},
    /* 228*/ {0x3fffe9, 22},    /* 229*/ {0x1fffe7, 21},
    /* 230*/ {0x1fffe8, 21},    /* 231*/ {0x7ffff3, 23},
    /* 232*/ {0x3fffea, 22},    /* 233*/ {0x3fffeb, 22},
    /* 234*/ {0x1ffffee, 25},   /* 235*/ {0x1ffffef, 25},
    /* 236*/ {0xfffff4, 24},    /* 237*/ {0xfffff5, 24},
    /* 238*/ {0x3ffffea, 26},   /* 239*/ {0x7ffff4, 23},
    /* 240*/ {0x3ffffeb, 26},   /* 241*/ {0x7ffffe6, 27},
    /* 242*/ {0x3ffffec, 26},   /* 243*/ {0x3ffffed, 26},
    /* 244*/ {0x7ffffe7, 27},   /* 245*/ {0x7ffffe8, 27},
    /* 246*/ {0x7ffffe9, 27},   /* 247*/ {0x7ffffea, 27},
    /* 248*/ {0x7ffffeb, 27},   /* 249*/ {0xffffffe, 28},
    /* 250*/ {0x7ffffec, 27},   /* 251*/ {0x7ffffed, 27},
    /* 252*/ {0x7ffffee, 27},   /* 253*/ {0x7ffffef, 27},
    /* 254*/ {0x7fffff0, 27},   /* 255*/ {0x3ffffee, 26},
    /* EOS*/ {0x3fffffff, 30},
};

struct TrieNode {
    int m_child[2] = {-1, -1};
    std::uint32_t m_sym = SYM_NONE;
};

struct Trie {
    std::array<TrieNode, TRIE_CAP> m_nodes{};
    std::size_t m_size = 1;
};

// TODO: make consteval
Trie build_trie() {
    Trie trie;
    for (auto [sym, entry] : std::views::enumerate(CODES)) {
        const auto [code, len] = entry;
        int cur = 0;
        for (int shift : std::views::iota(0, static_cast<int>(len)) | std::views::reverse) {
            const int BIT = (code >> shift) & 1;
            if (trie.m_nodes[cur].m_child[BIT] < 0)
                trie.m_nodes[cur].m_child[BIT] = static_cast<int>(trie.m_size++);
            cur = trie.m_nodes[cur].m_child[BIT];
        }
        trie.m_nodes[cur].m_sym = static_cast<std::uint32_t>(sym);
    }
    return trie;
}

template <int W>
struct TransTable {
    static constexpr int CHUNK_COUNT = 1 << W;
    std::array<std::pair<std::uint16_t, std::uint16_t>, TABLE_CAP> m_entries{};
    std::uint16_t m_row_count = 0;
};

// TODO: make consteval
template <int W>
TransTable<W> build_table() {
    constexpr int CHUNKS = TransTable<W>::CHUNK_COUNT;
    Trie trie = build_trie();
    TransTable<W> table;

    std::array<int, TRIE_CAP> node_to_row{};
    node_to_row.fill(-1);
    std::array<int, TRIE_CAP> queue{};
    int q_head = 0;
    int q_tail = 0;

    auto alloc_row = [&](int node) -> int {
        const int ROW = static_cast<int>(table.m_row_count++);
        node_to_row[node] = ROW;
        return ROW;
    };

    alloc_row(0);
    queue[q_tail++] = 0;

    while (q_head < q_tail) {
        const int START = queue[q_head++];
        const int ROW = node_to_row[START];

        for (int chunk : std::views::iota(0, CHUNKS)) {
            int cur = START;
            std::uint32_t emitted_sym = SYM_NONE;
            bool invalid = false;

            for (int shift : std::views::iota(0, W) | std::views::reverse) {
                const int BIT = (chunk >> shift) & 1;
                const int NEXT = trie.m_nodes[cur].m_child[BIT];
                if (NEXT < 0) {
                    invalid = true;
                    break;
                }
                cur = NEXT;
                if (trie.m_nodes[cur].m_sym != SYM_NONE) {
                    emitted_sym = trie.m_nodes[cur].m_sym;
                    cur = 0;
                }
            }

            const std::size_t SLOT = (static_cast<std::size_t>(ROW) * CHUNKS) + chunk;
            if (invalid) {
                table.m_entries[SLOT] = {0xFFFFU, 0xFFFFU};
            } else {
                if (node_to_row[cur] < 0) {
                    alloc_row(cur);
                    queue[q_tail++] = cur;
                }
                const std::uint16_t SYM16 =
                    (emitted_sym == SYM_NONE) ? 0xFFFEU : static_cast<std::uint16_t>(emitted_sym);
                table.m_entries[SLOT] = {static_cast<std::uint16_t>(node_to_row[cur]), SYM16};
            }
        }
    }
    return table;
}

} // namespace io::shared_codec::huffman

export namespace io::shared_codec::huffman {

// HuffmanEncodeView
//
// Lazy std::byte-producing range over an input char/byte sequence.
//
// Algorithm: 64-bit MSB-first accumulator.
template <std::ranges::input_range R>
    requires std::convertible_to<std::ranges::range_value_t<R>, std::byte>
class HuffmanEncodeView {
  public:
    class Iterator {
      public:
        using value_type = std::byte;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::input_iterator_tag;
        using iterator_concept = std::input_iterator_tag;

        Iterator() = default;

        explicit Iterator(R &base)
            : m_inner{std::ranges::begin(base)}, m_end{std::ranges::end(base)}, m_bits{}, m_shift{}, m_done{} {
            advance();
        }

        [[nodiscard]] std::byte operator*() const noexcept {
            return std::byte{static_cast<unsigned char>(m_bits >> (m_shift - 8))};
        }

        Iterator &operator++() {
            m_shift -= 8;
            advance();
            return *this;
        }

        void operator++(int) { ++*this; }

        [[nodiscard]] bool operator==(std::default_sentinel_t /*unused*/) const noexcept { return m_done; }

      private:
        void advance() {
            if (m_inner == m_end) {
                if (m_shift == 0) {
                    m_done = true;
                    return;
                }
                const int PAD = 8 - m_shift;
                m_bits = (m_bits << PAD) | ((1U << PAD) - 1U);
                m_shift = 8;
            } else {
                const auto [code, len] = CODES[std::to_integer<unsigned char>(*m_inner++)];
                m_bits = (m_bits << len) | static_cast<std::uint64_t>(code);
                m_shift += static_cast<int>(len);
            }
        }

        std::ranges::iterator_t<R> m_inner;
        std::ranges::sentinel_t<R> m_end;
        std::uint64_t m_bits;
        int m_shift;
        bool m_done;
    };

    HuffmanEncodeView() = default;
    explicit HuffmanEncodeView(R base) : m_base{std::move(base)} {}

    ~HuffmanEncodeView() = default;

    HuffmanEncodeView(const HuffmanEncodeView &) = delete;
    HuffmanEncodeView &operator=(const HuffmanEncodeView &) = delete;
    HuffmanEncodeView(HuffmanEncodeView &&) = default;
    HuffmanEncodeView &operator=(HuffmanEncodeView &&) = default;

    [[nodiscard]] Iterator begin() { return Iterator{m_base}; }
    [[nodiscard]] std::default_sentinel_t end() const noexcept { return {}; }

  private:
    R m_base;
};

template <std::ranges::input_range R>
HuffmanEncodeView(R &&) -> HuffmanEncodeView<std::views::all_t<R>>;

// HuffmanDecodeView
//
// Lazy char-producing range over a std::byte input sequence.
//
// Algorithm: W-bit chunk FSM, buffer-drain pattern.
template <int W, std::ranges::input_range R>
    requires DecodeWidth<W> && std::same_as<std::ranges::range_value_t<R>, std::byte>
class HuffmanDecodeView {
  public:
    static constexpr int CHUNKS = 1 << W;
    static constexpr int CHUNKS_PER_BYTE = 8 / W;
    static constexpr int CHUNK_MASK = CHUNKS - 1;
    // TODO: make constexpr when GCC supports it
    inline static const TransTable<W> TABLE = build_table<W>();

    class Iterator {
      public:
        using value_type = char;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::input_iterator_tag;
        using iterator_concept = std::input_iterator_tag;

        Iterator() = default;

        explicit Iterator(R &base)
            : m_inner{std::ranges::begin(base)}, m_end{std::ranges::end(base)}, m_chunk_idx{0}, m_current{}, m_done{},
              m_padding_bits{} {}

        [[nodiscard]] char operator*() const noexcept { return m_current; }

        Iterator &operator++() {
            advance();
            return *this;
        }

        void operator++(int) { ++*this; }

        [[nodiscard]] bool operator==(std::default_sentinel_t /*unused*/) const noexcept { return m_done; }

      private:
        void advance() {
            if (m_inner == m_end) {
                if (m_chunk_idx != 0) {
                    throw error::http::HuffmanDecodeError{"truncated Huffman stream"};
                }
                if (m_fsm != 0 && m_padding_bits > 7) {
                    throw error::http::HuffmanDecodeError{"truncated Huffman stream"};
                }
                m_done = true;
                return;
            }

            const int CHUNK = CHUNK_MASK & (std::to_integer<unsigned>(*m_inner) >> (8 - (W * (m_chunk_idx + 1))));

            if (++m_chunk_idx == CHUNKS_PER_BYTE) {
                ++m_inner;
                m_chunk_idx = 0;
            }

            const auto &[ns, sym] = TABLE.m_entries[(static_cast<std::size_t>(m_fsm) * CHUNKS) + CHUNK];
            m_fsm = ns;

            if (sym < 256U) [[likely]] {
                m_current = static_cast<char>(sym);
                m_padding_bits = 0;
            } else if (sym == 0xFFFFU) [[unlikely]] {
                throw error::http::HuffmanDecodeError{"invalid Huffman code"};
            } else if (sym == static_cast<std::uint16_t>(SYM_EOS)) {
                throw error::http::HuffmanDecodeError{"EOS symbol in stream"};
            } else {
                m_padding_bits += W;
            }
        }

        std::ranges::iterator_t<R> m_inner;
        std::ranges::sentinel_t<R> m_end;
        std::uint32_t m_fsm{};
        int m_chunk_idx;
        char m_current;
        bool m_done;
        int m_padding_bits;
    };

    HuffmanDecodeView() = default;
    explicit HuffmanDecodeView(R &&base) : m_base{std::forward<R>(base)} {}

    ~HuffmanDecodeView() = default;

    HuffmanDecodeView(const HuffmanDecodeView &) = delete;
    HuffmanDecodeView &operator=(const HuffmanDecodeView &) = delete;
    HuffmanDecodeView(HuffmanDecodeView &&) = default;
    HuffmanDecodeView &operator=(HuffmanDecodeView &&) = default;

    [[nodiscard]] Iterator begin() { return Iterator{m_base}; }
    [[nodiscard]] std::default_sentinel_t end() const noexcept { return {}; }

  private:
    R m_base;
};

struct HuffmanEncodeAdaptor : std::ranges::range_adaptor_closure<HuffmanEncodeAdaptor> {
    template <std::ranges::viewable_range R>
        requires std::convertible_to<std::ranges::range_value_t<R>, std::byte>
    [[nodiscard]] auto operator()(R &&range) const {
        return HuffmanEncodeView<std::views::all_t<R>>{std::views::all(std::forward<R>(range))};
    }
};

template <int W = 4>
    requires DecodeWidth<W>
struct HuffmanDecodeAdaptor : std::ranges::range_adaptor_closure<HuffmanDecodeAdaptor<W>> {
    template <std::ranges::viewable_range R>
        requires std::same_as<std::ranges::range_value_t<R>, std::byte>
    [[nodiscard]] auto operator()(R &&range) const {
        return HuffmanDecodeView<W, std::views::all_t<R>>{std::views::all(std::forward<R>(range))};
    }
};

template <int W = 4>
    requires DecodeWidth<W>
struct Huffman {
    [[nodiscard]] static HuffmanEncodeAdaptor encode() noexcept { return {}; }
    [[nodiscard]] static HuffmanDecodeAdaptor<W> decode() noexcept { return {}; }
};

} // namespace io::shared_codec::huffman
