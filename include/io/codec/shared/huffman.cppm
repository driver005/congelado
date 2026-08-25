export module io_codec_shared:huffman;
import std;
import io_error;
import :types;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

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
    // Walk every symbol's canonical Huffman code and thread it into the trie bit by bit.
    for (auto [sym, entry] : std::views::enumerate(CODES)) {
        const auto [code, len] = entry;
        int cur = 0;
        // MSB-first — each bit either follows an existing child or lazily allocates a new node.
        for (int shift : std::views::iota(0, static_cast<int>(len)) | std::views::reverse) {
            const int BIT = static_cast<int>((code >> shift) & 1);
            if (trie.m_nodes[cur].m_child[BIT] < 0) {  // FIXME(clang-tidy): unchecked operator[], consider .at(); non-constant array index
                trie.m_nodes[cur].m_child[BIT] = static_cast<int>(trie.m_size++);  // FIXME(clang-tidy): unchecked operator[], consider .at(); non-constant array index
            }
            cur = trie.m_nodes[cur].m_child[BIT];  // FIXME(clang-tidy): unchecked operator[], consider .at(); non-constant array index
        }
        // Landed on the leaf for this code — tag it with the symbol it decodes to.
        trie.m_nodes[cur].m_sym = static_cast<std::uint32_t>(sym);  // FIXME(clang-tidy): unchecked operator[], consider .at(); non-constant array index
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
    // Build the bit-level trie first — the table below is just this trie collapsed into
    // W-bit-at-a-time transitions.
    Trie trie = build_trie();
    TransTable<W> table;

    // node_to_row maps a trie node to its row in the output table, lazily assigned as nodes
    // get discovered; queue drives the BFS that discovers them.
    std::array<int, TRIE_CAP> node_to_row{};
    node_to_row.fill(-1);
    std::array<int, TRIE_CAP> queue{};
    int q_head = 0;
    int q_tail = 0;

    auto alloc_row = [&](int node) -> int {
        const int ROW = static_cast<int>(table.m_row_count++);
        node_to_row[node] = ROW;  // FIXME(clang-tidy): unchecked operator[], consider .at(); non-constant array index
        return ROW;
    };

    // Seed the BFS at the trie root.
    alloc_row(0);
    queue[q_tail++] = 0;  // FIXME(clang-tidy): unchecked operator[], consider .at(); non-constant array index

    // Standard BFS — pop a row's trie node, work out where every possible W-bit chunk sends it.
    while (q_head < q_tail) {
        const int START = queue[q_head++];  // FIXME(clang-tidy): unchecked operator[], consider .at(); non-constant array index
        const int ROW = node_to_row[START];  // FIXME(clang-tidy): unchecked operator[], consider .at(); non-constant array index

        for (int chunk : std::views::iota(0, CHUNKS)) {
            int cur = START;
            std::uint32_t emitted_sym = SYM_NONE;
            bool invalid = false;

            // Walk this chunk's W bits through the trie, one at a time. Landing on a leaf
            // mid-chunk emits a symbol and resets back to the trie root for the remaining bits.
            for (int shift : std::views::iota(0, W) | std::views::reverse) {
                const int BIT = (chunk >> shift) & 1;
                const int NEXT = trie.m_nodes[cur].m_child[BIT];  // FIXME(clang-tidy): unchecked operator[], consider .at(); non-constant array index
                if (NEXT < 0) {
                    invalid = true;
                    break;
                }
                cur = NEXT;
                if (trie.m_nodes[cur].m_sym != SYM_NONE) {  // FIXME(clang-tidy): unchecked operator[], consider .at(); non-constant array index
                    emitted_sym = trie.m_nodes[cur].m_sym;  // FIXME(clang-tidy): unchecked operator[], consider .at(); non-constant array index
                    cur = 0;
                }
            }

            // Invalid chunk (no such Huffman code) gets a sentinel; otherwise record the
            // destination row (allocating one if this node's new) plus whatever symbol emitted.
            const std::size_t SLOT = (static_cast<std::size_t>(ROW) * CHUNKS) + chunk;
            if (invalid) {
                table.m_entries[SLOT] = {0xFFFFU, 0xFFFFU};  // FIXME(clang-tidy): unchecked operator[], consider .at(); non-constant array index
            } else {
                if (node_to_row[cur] < 0) {  // FIXME(clang-tidy): unchecked operator[], consider .at(); non-constant array index
                    alloc_row(cur);
                    queue[q_tail++] = cur;  // FIXME(clang-tidy): unchecked operator[], consider .at(); non-constant array index
                }
                const std::uint16_t SYM16 =
                    (emitted_sym == SYM_NONE) ? 0xFFFEU : static_cast<std::uint16_t>(emitted_sym);
                table.m_entries[SLOT] = {static_cast<std::uint16_t>(node_to_row[cur]), SYM16};  // FIXME(clang-tidy): unchecked operator[], consider .at(); non-constant array index
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
        using reference = std::byte;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::input_iterator_tag;
        using iterator_concept = std::input_iterator_tag;

        /**
         * @brief Builds a sentinel/empty iterator — not attached to any range, just for `=
         * default`.
         */
        Iterator() = default;

        /**
         * @brief Attaches the iterator to `base` and primes the bit accumulator by pulling in the
         * first symbol's Huffman code.
         * @param base the input range to encode; iterator holds begin/end into it, not a copy.
         */
        explicit Iterator(R &base)
            : m_inner{std::ranges::begin(base)}, m_end{std::ranges::end(base)} {
            advance();
        }

        /**
         * @brief Reads the next fully-buffered output byte off the top of the bit accumulator.
         * @return the byte.
         */
        [[nodiscard]] std::byte operator*() const noexcept {
            return std::byte{static_cast<unsigned char>(m_bits >> (m_shift - 8))};
        }

        /**
         * @brief Consumes the byte just read and pulls more Huffman-coded bits in behind it.
         * @return `*this`, advanced.
         */
        Iterator &operator++() {
            m_shift -= 8;
            advance();
            return *this;
        }

        /**
         * @brief Postfix increment — same motion as prefix, just discards the "old value" nobody
         * asked for.
         */
        void operator++(int) { ++*this; }

        /**
         * @brief Sentinel comparison — this is how range-for knows the view's exhausted.
         * @return true once every input symbol's been consumed and the final padded byte's been
         * emitted.
         */
        [[nodiscard]] bool operator==(std::default_sentinel_t /*unused*/) const noexcept {
            return m_done;
        }

      private:
        /**
         * @brief Pumps more Huffman-coded bits into the 64-bit MSB-first accumulator — either the
         * next input symbol's code, or (once input's exhausted) the EOS padding bits (RFC 7541
         * §5.2: pad with the high-order bits of the EOS code, i.e. all 1s) to round out the final
         * byte.
         * @note Sets `m_done` once there's nothing left to emit — no more input and the
         * accumulator's fully drained (shift back to 0).
         */
        void advance() {
            // Input's exhausted — either we're fully drained (done), or there's a partial byte
            // left that needs EOS padding bits to round it out.
            if (m_inner == m_end) {
                if (m_shift == 0) {
                    m_done = true;
                    return;
                }
                const int PAD = 8 - m_shift;
                m_bits = (m_bits << PAD) | ((1U << PAD) - 1U);
                m_shift = 8;
            } else {
                // Normal motion — pull the next symbol's Huffman code and stack it onto the
                // accumulator.
                const auto [code, len] = CODES[std::to_integer<unsigned char>(*m_inner++)];
                m_bits = (m_bits << len) | static_cast<std::uint64_t>(code);
                m_shift += static_cast<int>(len);
            }
        }

        std::ranges::iterator_t<R> m_inner;
        std::ranges::sentinel_t<R> m_end;
        std::uint64_t m_bits{};
        int m_shift{};
        bool m_done{};
    };

    /** @brief Builds an empty view, no backing range yet — for `= default` scenarios only. */
    HuffmanEncodeView() = default;
    /**
     * @brief Wraps `base` for lazy Huffman encoding — nothing gets encoded until iteration
     * actually starts pulling bytes.
     * @param base the input char/byte range to encode.
     */
    explicit HuffmanEncodeView(R base) : m_base{std::move(base)} {}

    /** @brief Trivial dtor — `m_base` cleans up its own storage, no motion needed here. */
    ~HuffmanEncodeView() = default;

    /**
     * @brief Deleted — copying the base range could be arbitrarily expensive/wrong for arbitrary
     * `R`, so it's off the table.
     */
    HuffmanEncodeView(const HuffmanEncodeView &) = delete;
    /** @brief Deleted for the same reason as the copy ctor. */
    HuffmanEncodeView &operator=(const HuffmanEncodeView &) = delete;
    /** @brief Defaulted — moving just relocates `m_base`, nothing fancy. */
    HuffmanEncodeView(HuffmanEncodeView &&) = default;
    /** @brief Defaulted move-assign, same deal as the move ctor. */
    HuffmanEncodeView &operator=(HuffmanEncodeView &&) = default;

    /**
     * @brief Gets a fresh Iterator primed on `m_base` — starts the lazy encode.
     * @return an Iterator at the start.
     */
    [[nodiscard]] Iterator begin() { return Iterator{m_base}; }
    /** @brief Gets the sentinel that marks "encoding's done". @return a default_sentinel_t. */
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
    // Lazily built on first use (C++11 magic-statics, thread-safe) instead of a static-duration
    // member initialized at load time — build_table<W>() can throw, and this way the throw
    // happens on first real use instead of during static initialization, where it can't be caught.
    [[nodiscard]] static const TransTable<W> &get_table() {
        static const TransTable<W> TABLE = build_table<W>();
        return TABLE;
    }

    class Iterator {
      public:
        using value_type = char;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::input_iterator_tag;
        using iterator_concept = std::input_iterator_tag;

        /**
         * @brief Builds a sentinel/empty iterator — not attached to any range, just for `=
         * default`.
         */
        Iterator() = default;

        /**
         * @brief Attaches the iterator to `base` and decodes the first output character to prime
         * it.
         * @param base the input byte range to Huffman-decode; iterator holds begin/end into it.
         */
        explicit Iterator(R &base)
            : m_inner{std::ranges::begin(base)}, m_end{std::ranges::end(base)} {
            advance();
        }

        /**
         * @brief Gets the currently-decoded output character.
         * @return the last symbol emitted by advance().
         */
        [[nodiscard]] char operator*() const noexcept { return m_current; }

        /**
         * @brief Decodes the next output character.
         * @return `*this`, advanced past the one just emitted.
         */
        Iterator &operator++() {
            advance();
            return *this;
        }

        /**
         * @brief Postfix increment — same motion as prefix, discards the "old value" nobody asked
         * for.
         */
        void operator++(int) { ++*this; }

        /**
         * @brief Sentinel comparison — how range-for knows the decode's finished.
         * @return true once the FSM's fully drained the input with no more symbols to emit.
         */
        [[nodiscard]] bool operator==(std::default_sentinel_t /*unused*/) const noexcept {
            return m_done;
        }

      private:
        /**
         * @brief Drives the W-bit chunk FSM forward, table-lookup-decoding chunks until a real
         * symbol pops out (`sym < 256`), then stashes it in `m_current` and returns.
         * @note Runs the transition table (`TABLE`) chunk-by-chunk against the input bytes,
         * tracking `m_fsm` state across calls — this is the whole decode loop, one character at a
         * time, lazy.
         * @warning A malformed Huffman stream doesn't fail quietly: an invalid code (`sym ==
         * 0xFFFF`), an EOS symbol showing up mid-stream, or a truncated stream with too many
         * leftover padding bits (`m_padding_bits > 7`) all throw
         * `error::http::HuffmanDecodeError`. This is untrusted wire data — treat every throw as a
         * hostile-input signal, not a bug.
         */
        void advance() {
            // Loop chunk-by-chunk until a real symbol pops out or the stream runs dry.
            while (true) {
                // Input's exhausted — only a clean finish if we're byte-aligned and any leftover
                // padding is a valid EOS prefix (7 bits or fewer); anything else is a cut-off
                // stream.
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

                // Pull the next W-bit chunk out of the current byte, advancing to the next byte
                // once every chunk in this one's been consumed.
                const int CHUNK = CHUNK_MASK & (std::to_integer<unsigned>(*m_inner) >>
                                                (8 - (W * (m_chunk_idx + 1))));

                if (++m_chunk_idx == CHUNKS_PER_BYTE) {
                    ++m_inner;
                    m_chunk_idx = 0;
                }

                // Table lookup drives the FSM to its next state and (maybe) hands back a symbol.
                const auto &[ns, sym] =
                    get_table().m_entries[(static_cast<std::size_t>(m_fsm) * CHUNKS) + CHUNK];  // FIXME(clang-tidy): unchecked operator[], consider .at(); non-constant array index
                m_fsm = ns;

                // A real symbol (< 256) is the common case — stash it and return. Otherwise
                // it's either a hard decode error (invalid code, stray EOS) or just more padding
                // bits accumulating with no symbol yet, so keep looping.
                if (sym < 256U) [[likely]] {
                    m_current = static_cast<char>(sym);
                    m_padding_bits = 0;
                    return;
                } else if (sym == 0xFFFFU) [[unlikely]] {
                    throw error::http::HuffmanDecodeError{"invalid Huffman code"};
                } else if (sym == static_cast<std::uint16_t>(SYM_EOS)) {
                    throw error::http::HuffmanDecodeError{"EOS symbol in stream"};
                } else {
                    m_padding_bits += W;
                }
            }
        }

        std::ranges::iterator_t<R> m_inner;
        std::ranges::sentinel_t<R> m_end;
        std::uint32_t m_fsm{};
        int m_chunk_idx{};
        char m_current{};
        bool m_done{};
        int m_padding_bits{};
    };

    /** @brief Builds an empty view, no backing range yet — for `= default` scenarios only. */
    HuffmanDecodeView() = default;
    /**
     * @brief Wraps `base` for lazy Huffman decoding — nothing gets decoded until iteration
     * actually starts pulling characters.
     * @param base the input `std::byte` range to decode.
     */
    explicit HuffmanDecodeView(R &&base) : m_base{std::move(base)} {}

    /** @brief Trivial dtor — `m_base` cleans up its own storage, no motion needed here. */
    ~HuffmanDecodeView() = default;

    /**
     * @brief Deleted — copying the base range could be arbitrarily expensive/wrong for arbitrary
     * `R`.
     */
    HuffmanDecodeView(const HuffmanDecodeView &) = delete;
    /** @brief Deleted for the same reason as the copy ctor. */
    HuffmanDecodeView &operator=(const HuffmanDecodeView &) = delete;
    /** @brief Defaulted — moving just relocates `m_base`, nothing fancy. */
    HuffmanDecodeView(HuffmanDecodeView &&) = default;
    /** @brief Defaulted move-assign, same deal as the move ctor. */
    HuffmanDecodeView &operator=(HuffmanDecodeView &&) = default;

    /**
     * @brief Gets a fresh Iterator primed on `m_base` — starts the lazy decode.
     * @return an Iterator at the start.
     */
    [[nodiscard]] Iterator begin() { return Iterator{m_base}; }
    /** @brief Gets the sentinel that marks "decoding's done". @return a default_sentinel_t. */
    [[nodiscard]] std::default_sentinel_t end() const noexcept { return {}; }

  private:
    R m_base;
};

struct HuffmanEncodeAdaptor : std::ranges::range_adaptor_closure<HuffmanEncodeAdaptor> {
    /**
     * @brief Pipe-adaptor call: turns `range | HuffmanEncodeAdaptor{}` into a lazy
     * HuffmanEncodeView over it. This is what makes `some_range | huffman::Huffman<>::encode()`
     * read clean, no cap.
     * @tparam R the viewable range type piped in.
     * @param range the char/byte range to encode.
     * @return a HuffmanEncodeView wrapping `range`.
     */
    template <std::ranges::viewable_range R>
        requires std::convertible_to<std::ranges::range_value_t<R>, std::byte>
    [[nodiscard]] auto operator()(R &&range) const {
        return HuffmanEncodeView<std::views::all_t<R>>{std::views::all(std::forward<R>(range))};
    }
};

template <int W = 4>
    requires DecodeWidth<W>
struct HuffmanDecodeAdaptor : std::ranges::range_adaptor_closure<HuffmanDecodeAdaptor<W>> {
    /**
     * @brief Pipe-adaptor call: turns `range | HuffmanDecodeAdaptor<W>{}` into a lazy
     * HuffmanDecodeView over it.
     * @tparam R the viewable range type piped in.
     * @param range the `std::byte` range to decode.
     * @return a HuffmanDecodeView<W, ...> wrapping `range`.
     */
    template <std::ranges::viewable_range R>
        requires std::same_as<std::ranges::range_value_t<R>, std::byte>
    [[nodiscard]] auto operator()(R &&range) const {
        return HuffmanDecodeView<W, std::views::all_t<R>>{std::views::all(std::forward<R>(range))};
    }
};

template <int W = 4>
    requires DecodeWidth<W>
struct Huffman {
    /**
     * @brief Gets a pipeable encode adaptor.
     * @return a fresh HuffmanEncodeAdaptor, stateless so any instance works.
     */
    [[nodiscard]] static HuffmanEncodeAdaptor encode() noexcept { return {}; }
    /**
     * @brief Gets a pipeable decode adaptor at chunk-width `W`.
     * @return a fresh HuffmanDecodeAdaptor<W>, stateless.
     */
    [[nodiscard]] static HuffmanDecodeAdaptor<W> decode() noexcept { return {}; }
};

} // namespace io::shared_codec::huffman

#ifdef CONGELADO_TEST
namespace io::shared_codec::huffman::tests {
using namespace boost::ut;

suite<"Huffman encode/decode"> huffman_round_trip_suite = [] {
    "round-trips an empty string"_test = [] {
        std::string original;
        std::vector<std::byte> byte_view;

        std::vector<std::byte> encoded;
        for (std::byte value : byte_view | Huffman<4>::encode()) {
            encoded.push_back(value);
        }
        expect(encoded.empty());

        std::string decoded;
        for (char character : encoded | Huffman<4>::decode()) {
            decoded += character;
        }
        expect(decoded == original);
    };

    "round-trips a short ASCII string"_test = [] {
        std::string original = "hello world";
        std::vector<std::byte> byte_view;
        for (char character : original) {
            byte_view.push_back(static_cast<std::byte>(character));
        }

        std::vector<std::byte> encoded;
        for (std::byte value : byte_view | Huffman<4>::encode()) {
            encoded.push_back(value);
        }

        std::string decoded;
        for (char character : encoded | Huffman<4>::decode()) {
            decoded += character;
        }
        expect(decoded == original);
    };

    "matches the RFC 7541 C.4.1 known encoding for \"www.example.com\""_test = [] {
        std::string original = "www.example.com";
        std::vector<std::byte> byte_view;
        for (char character : original) {
            byte_view.push_back(static_cast<std::byte>(character));
        }

        std::vector<std::byte> encoded;
        for (std::byte value : byte_view | Huffman<4>::encode()) {
            encoded.push_back(value);
        }

        std::vector<std::byte> expected{std::byte{0xf1}, std::byte{0xe3}, std::byte{0xc2}, std::byte{0xe5},
                                        std::byte{0xf2}, std::byte{0x3a}, std::byte{0x6b}, std::byte{0xa0},
                                        std::byte{0xab}, std::byte{0x90}, std::byte{0xf4}, std::byte{0xff}};
        // Plain `==` on two std::vector<std::byte> forces boost::ut's failure-diagnostic printer
        // to instantiate operator<<(ostream&, std::byte), which doesn't exist — wrapping in
        // std::ranges::equal() keeps the comparison a plain bool instead.
        expect(std::ranges::equal(encoded, expected));
    };

    "decoding an all-ones stream throws HuffmanDecodeError"_test = [] {
        std::vector<std::byte> garbage{std::byte{0xFF}, std::byte{0xFF}, std::byte{0xFF}, std::byte{0xFF}};

        expect(throws<error::http::HuffmanDecodeError>([&] {
            std::string decoded;
            for (char character : garbage | Huffman<4>::decode()) {
                decoded += character;
            }
        }));
    };
};

} // namespace io::shared_codec::huffman::tests
#endif
