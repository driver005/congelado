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
inline constexpr std::uint32_t SYM_NONE = 0xFF'FF'FF'FEU;
inline constexpr std::uint32_t SYM_INVALID = 0xFF'FF'FF'FFU;

inline constexpr std::size_t TRIE_CAP = 600;
inline constexpr std::size_t TABLE_CAP = 9'600;

inline constexpr std::pair<std::uint32_t, std::uint8_t> CODES[SYM_COUNT] = {
    /*   0 */ {0x1F'F8, 13},
    /*   1 */ {0x7F'FF'D8, 23},
    /*   2 */ {0xF'FF'FF'E2, 28},
    /*   3 */ {0xF'FF'FF'E3, 28},
    /*   4 */ {0xF'FF'FF'E4, 28},
    /*   5 */ {0xF'FF'FF'E5, 28},
    /*   6 */ {0xF'FF'FF'E6, 28},
    /*   7 */ {0xF'FF'FF'E7, 28},
    /*   8 */ {0xF'FF'FF'E8, 28},
    /*   9 */ {0xFF'FF'EA, 24},
    /*  10 */ {0x3F'FF'FF'FC, 30},
    /*  11 */ {0xF'FF'FF'E9, 28},
    /*  12 */ {0xF'FF'FF'EA, 28},
    /*  13 */ {0x3F'FF'FF'FD, 30},
    /*  14 */ {0xF'FF'FF'EB, 28},
    /*  15 */ {0xF'FF'FF'EC, 28},
    /*  16 */ {0xF'FF'FF'ED, 28},
    /*  17 */ {0xF'FF'FF'EE, 28},
    /*  18 */ {0xF'FF'FF'EF, 28},
    /*  19 */ {0xF'FF'FF'F0, 28},
    /*  20 */ {0xF'FF'FF'F1, 28},
    /*  21 */ {0xF'FF'FF'F2, 28},
    /*  22 */ {0x3F'FF'FF'FE, 30},
    /*  23 */ {0xF'FF'FF'F3, 28},
    /*  24 */ {0xF'FF'FF'F4, 28},
    /*  25 */ {0xF'FF'FF'F5, 28},
    /*  26 */ {0xF'FF'FF'F6, 28},
    /*  27 */ {0xF'FF'FF'F7, 28},
    /*  28 */ {0xF'FF'FF'F8, 28},
    /*  29 */ {0xF'FF'FF'F9, 28},
    /*  30 */ {0xF'FF'FF'FA, 28},
    /*  31 */ {0xF'FF'FF'FB, 28},
    /* ' '*/ {0x14, 6},
    /* '!'*/ {0x3'F8, 10},
    /* '"'*/ {0x3'F9, 10},
    /* '#'*/ {0xF'FA, 12},
    /* '$'*/ {0x1F'F9, 13},
    /* '%'*/ {0x15, 6},
    /* '&'*/ {0xF8, 8},
    /* '''*/ {0x7'FA, 11},
    /* '('*/ {0x3'FA, 10},
    /* ')'*/ {0x3'FB, 10},
    /* '*'*/ {0xF9, 8},
    /* '+'*/ {0x7'FB, 11},
    /* ','*/ {0xFA, 8},
    /* '-'*/ {0x16, 6},
    /* '.'*/ {0x17, 6},
    /* '/'*/ {0x18, 6},
    /* '0'*/ {0x0, 5},
    /* '1'*/ {0x1, 5},
    /* '2'*/ {0x2, 5},
    /* '3'*/ {0x19, 6},
    /* '4'*/ {0x1A, 6},
    /* '5'*/ {0x1B, 6},
    /* '6'*/ {0x1C, 6},
    /* '7'*/ {0x1D, 6},
    /* '8'*/ {0x1E, 6},
    /* '9'*/ {0x1F, 6},
    /* ':'*/ {0x5C, 7},
    /* ';'*/ {0xFB, 8},
    /* '<'*/ {0x7F'FC, 15},
    /* '='*/ {0x20, 6},
    /* '>'*/ {0xF'FB, 12},
    /* '?'*/ {0x3'FC, 10},
    /* '@'*/ {0x1F'FA, 13},
    /* 'A'*/ {0x21, 6},
    /* 'B'*/ {0x5D, 7},
    /* 'C'*/ {0x5E, 7},
    /* 'D'*/ {0x5F, 7},
    /* 'E'*/ {0x60, 7},
    /* 'F'*/ {0x61, 7},
    /* 'G'*/ {0x62, 7},
    /* 'H'*/ {0x63, 7},
    /* 'I'*/ {0x64, 7},
    /* 'J'*/ {0x65, 7},
    /* 'K'*/ {0x66, 7},
    /* 'L'*/ {0x67, 7},
    /* 'M'*/ {0x68, 7},
    /* 'N'*/ {0x69, 7},
    /* 'O'*/ {0x6A, 7},
    /* 'P'*/ {0x6B, 7},
    /* 'Q'*/ {0x6C, 7},
    /* 'R'*/ {0x6D, 7},
    /* 'S'*/ {0x6E, 7},
    /* 'T'*/ {0x6F, 7},
    /* 'U'*/ {0x70, 7},
    /* 'V'*/ {0x71, 7},
    /* 'W'*/ {0x72, 7},
    /* 'X'*/ {0xFC, 8},
    /* 'Y'*/ {0x73, 7},
    /* 'Z'*/ {0xFD, 8},
    /* '['*/ {0x1F'FB, 13},
    /* '\'*/ {0x7'FF'F0, 19},
    /* ']'*/ {0x1F'FC, 13},
    /* '^'*/ {0x3F'FC, 14},
    /* '_'*/ {0x22, 6},
    /* '`'*/ {0x7F'FD, 15},
    /* 'a'*/ {0x3, 5},
    /* 'b'*/ {0x23, 6},
    /* 'c'*/ {0x4, 5},
    /* 'd'*/ {0x24, 6},
    /* 'e'*/ {0x5, 5},
    /* 'f'*/ {0x25, 6},
    /* 'g'*/ {0x26, 6},
    /* 'h'*/ {0x27, 6},
    /* 'i'*/ {0x6, 5},
    /* 'j'*/ {0x74, 7},
    /* 'k'*/ {0x75, 7},
    /* 'l'*/ {0x28, 6},
    /* 'm'*/ {0x29, 6},
    /* 'n'*/ {0x2A, 6},
    /* 'o'*/ {0x7, 5},
    /* 'p'*/ {0x2B, 6},
    /* 'q'*/ {0x76, 7},
    /* 'r'*/ {0x2C, 6},
    /* 's'*/ {0x8, 5},
    /* 't'*/ {0x9, 5},
    /* 'u'*/ {0x2D, 6},
    /* 'v'*/ {0x77, 7},
    /* 'w'*/ {0x78, 7},
    /* 'x'*/ {0x79, 7},
    /* 'y'*/ {0x7A, 7},
    /* 'z'*/ {0x7B, 7},
    /* '{'*/ {0x7F'FE, 15},
    /* '|'*/ {0x7'FC, 11},
    /* '}'*/ {0x3F'FD, 14},
    /* '~'*/ {0x1F'FD, 13},
    /* 127*/ {0xF'FF'FF'FC, 28},
    /* 128*/ {0xF'FF'E6, 20},
    /* 129*/ {0x3F'FF'D2, 22},
    /* 130*/ {0xF'FF'E7, 20},
    /* 131*/ {0xF'FF'E8, 20},
    /* 132*/ {0x3F'FF'D3, 22},
    /* 133*/ {0x3F'FF'D4, 22},
    /* 134*/ {0x3F'FF'D5, 22},
    /* 135*/ {0x7F'FF'D9, 23},
    /* 136*/ {0x3F'FF'D6, 22},
    /* 137*/ {0x7F'FF'DA, 23},
    /* 138*/ {0x7F'FF'DB, 23},
    /* 139*/ {0x7F'FF'DC, 23},
    /* 140*/ {0x7F'FF'DD, 23},
    /* 141*/ {0x7F'FF'DE, 23},
    /* 142*/ {0xFF'FF'EB, 24},
    /* 143*/ {0x7F'FF'DF, 23},
    /* 144*/ {0xFF'FF'EC, 24},
    /* 145*/ {0xFF'FF'ED, 24},
    /* 146*/ {0x3F'FF'D7, 22},
    /* 147*/ {0x7F'FF'E0, 23},
    /* 148*/ {0xFF'FF'EE, 24},
    /* 149*/ {0x7F'FF'E1, 23},
    /* 150*/ {0x7F'FF'E2, 23},
    /* 151*/ {0x7F'FF'E3, 23},
    /* 152*/ {0x7F'FF'E4, 23},
    /* 153*/ {0x1F'FF'DC, 21},
    /* 154*/ {0x3F'FF'D8, 22},
    /* 155*/ {0x7F'FF'E5, 23},
    /* 156*/ {0x3F'FF'D9, 22},
    /* 157*/ {0x7F'FF'E6, 23},
    /* 158*/ {0x7F'FF'E7, 23},
    /* 159*/ {0xFF'FF'EF, 24},
    /* 160*/ {0x3F'FF'DA, 22},
    /* 161*/ {0x1F'FF'DD, 21},
    /* 162*/ {0xF'FF'E9, 20},
    /* 163*/ {0x3F'FF'DB, 22},
    /* 164*/ {0x3F'FF'DC, 22},
    /* 165*/ {0x7F'FF'E8, 23},
    /* 166*/ {0x7F'FF'E9, 23},
    /* 167*/ {0x1F'FF'DE, 21},
    /* 168*/ {0x7F'FF'EA, 23},
    /* 169*/ {0x3F'FF'DD, 22},
    /* 170*/ {0x3F'FF'DE, 22},
    /* 171*/ {0xFF'FF'F0, 24},
    /* 172*/ {0x1F'FF'DF, 21},
    /* 173*/ {0x3F'FF'DF, 22},
    /* 174*/ {0x7F'FF'EB, 23},
    /* 175*/ {0x7F'FF'EC, 23},
    /* 176*/ {0x1F'FF'E0, 21},
    /* 177*/ {0x1F'FF'E1, 21},
    /* 178*/ {0x3F'FF'E0, 22},
    /* 179*/ {0x1F'FF'E2, 21},
    /* 180*/ {0x7F'FF'ED, 23},
    /* 181*/ {0x3F'FF'E1, 22},
    /* 182*/ {0x7F'FF'EE, 23},
    /* 183*/ {0x7F'FF'EF, 23},
    /* 184*/ {0xF'FF'EA, 20},
    /* 185*/ {0x3F'FF'E2, 22},
    /* 186*/ {0x3F'FF'E3, 22},
    /* 187*/ {0x3F'FF'E4, 22},
    /* 188*/ {0x7F'FF'F0, 23},
    /* 189*/ {0x3F'FF'E5, 22},
    /* 190*/ {0x3F'FF'E6, 22},
    /* 191*/ {0x7F'FF'F1, 23},
    /* 192*/ {0x3'FF'FF'E0, 26},
    /* 193*/ {0x3'FF'FF'E1, 26},
    /* 194*/ {0xF'FF'EB, 20},
    /* 195*/ {0x7'FF'F1, 19},
    /* 196*/ {0x3F'FF'E7, 22},
    /* 197*/ {0x7F'FF'F2, 23},
    /* 198*/ {0x3F'FF'E8, 22},
    /* 199*/ {0x1'FF'FF'EC, 25},
    /* 200*/ {0x3'FF'FF'E2, 26},
    /* 201*/ {0x3'FF'FF'E3, 26},
    /* 202*/ {0x3'FF'FF'E4, 26},
    /* 203*/ {0x7'FF'FF'DE, 27},
    /* 204*/ {0x7'FF'FF'DF, 27},
    /* 205*/ {0x3'FF'FF'E5, 26},
    /* 206*/ {0xFF'FF'F1, 24},
    /* 207*/ {0x1'FF'FF'ED, 25},
    /* 208*/ {0x7'FF'F2, 19},
    /* 209*/ {0x1F'FF'E3, 21},
    /* 210*/ {0x3'FF'FF'E6, 26},
    /* 211*/ {0x7'FF'FF'E0, 27},
    /* 212*/ {0x7'FF'FF'E1, 27},
    /* 213*/ {0x3'FF'FF'E7, 26},
    /* 214*/ {0x7'FF'FF'E2, 27},
    /* 215*/ {0xFF'FF'F2, 24},
    /* 216*/ {0x1F'FF'E4, 21},
    /* 217*/ {0x1F'FF'E5, 21},
    /* 218*/ {0x3'FF'FF'E8, 26},
    /* 219*/ {0x3'FF'FF'E9, 26},
    /* 220*/ {0xF'FF'FF'FD, 28},
    /* 221*/ {0x7'FF'FF'E3, 27},
    /* 222*/ {0x7'FF'FF'E4, 27},
    /* 223*/ {0x7'FF'FF'E5, 27},
    /* 224*/ {0xF'FF'EC, 20},
    /* 225*/ {0xFF'FF'F3, 24},
    /* 226*/ {0xF'FF'ED, 20},
    /* 227*/ {0x1F'FF'E6, 21},
    /* 228*/ {0x3F'FF'E9, 22},
    /* 229*/ {0x1F'FF'E7, 21},
    /* 230*/ {0x1F'FF'E8, 21},
    /* 231*/ {0x7F'FF'F3, 23},
    /* 232*/ {0x3F'FF'EA, 22},
    /* 233*/ {0x3F'FF'EB, 22},
    /* 234*/ {0x1'FF'FF'EE, 25},
    /* 235*/ {0x1'FF'FF'EF, 25},
    /* 236*/ {0xFF'FF'F4, 24},
    /* 237*/ {0xFF'FF'F5, 24},
    /* 238*/ {0x3'FF'FF'EA, 26},
    /* 239*/ {0x7F'FF'F4, 23},
    /* 240*/ {0x3'FF'FF'EB, 26},
    /* 241*/ {0x7'FF'FF'E6, 27},
    /* 242*/ {0x3'FF'FF'EC, 26},
    /* 243*/ {0x3'FF'FF'ED, 26},
    /* 244*/ {0x7'FF'FF'E7, 27},
    /* 245*/ {0x7'FF'FF'E8, 27},
    /* 246*/ {0x7'FF'FF'E9, 27},
    /* 247*/ {0x7'FF'FF'EA, 27},
    /* 248*/ {0x7'FF'FF'EB, 27},
    /* 249*/ {0xF'FF'FF'FE, 28},
    /* 250*/ {0x7'FF'FF'EC, 27},
    /* 251*/ {0x7'FF'FF'ED, 27},
    /* 252*/ {0x7'FF'FF'EE, 27},
    /* 253*/ {0x7'FF'FF'EF, 27},
    /* 254*/ {0x7'FF'FF'F0, 27},
    /* 255*/ {0x3'FF'FF'EE, 26},
    /* EOS*/ {0x3F'FF'FF'FF, 30},
};

struct TrieNode
{
    int m_child[2] = {-1, -1};
    std::uint32_t m_sym = SYM_NONE;
};

struct Trie
{
    std::array<TrieNode, TRIE_CAP> m_nodes{};
    std::size_t m_size = 1;
};

// TODO: make consteval
Trie build_trie()
{
    Trie trie;
    // Walk every symbol's canonical Huffman code and thread it into the trie bit by bit.
    for (auto [sym, entry]: std::views::enumerate(CODES)) {
        const auto [code, len] = entry;
        int cur = 0;
        // MSB-first — each bit either follows an existing child or lazily allocates a new node.
        for (int shift: std::views::iota(0, static_cast<int>(len)) | std::views::reverse) {
            const int BIT = static_cast<int>((code >> shift) & 1);
            if (trie.m_nodes[cur].m_child[BIT] < 0) { // FIXME(clang-tidy): unchecked operator[],
                                                      // consider .at(); non-constant array index
                trie.m_nodes[cur].m_child[BIT] =
                    static_cast<int>(trie.m_size++); // FIXME(clang-tidy): unchecked operator[],
                                                     // consider .at(); non-constant array index
            }
            cur = trie.m_nodes[cur].m_child[BIT]; // FIXME(clang-tidy): unchecked operator[],
                                                  // consider .at(); non-constant array index
        }
        // Landed on the leaf for this code — tag it with the symbol it decodes to.
        trie.m_nodes[cur].m_sym = static_cast<std::uint32_t>(
            sym
        ); // FIXME(clang-tidy): unchecked operator[], consider .at(); non-constant array index
    }
    return trie;
}

template<int W>
struct TransTable
{
    static constexpr int CHUNK_COUNT = 1 << W;
    std::array<std::pair<std::uint16_t, std::uint16_t>, TABLE_CAP> m_entries{};
    std::uint16_t m_row_count = 0;
};

// TODO: make consteval
template<int W>
TransTable<W> build_table()
{
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
        node_to_row[node] = ROW; // FIXME(clang-tidy): unchecked operator[], consider .at();
                                 // non-constant array index
        return ROW;
    };

    // Seed the BFS at the trie root.
    alloc_row(0);
    queue[q_tail++] =
        0; // FIXME(clang-tidy): unchecked operator[], consider .at(); non-constant array index

    // Standard BFS — pop a row's trie node, work out where every possible W-bit chunk sends it.
    while (q_head < q_tail) {
        const int START = queue[q_head++];  // FIXME(clang-tidy): unchecked operator[], consider
                                            // .at(); non-constant array index
        const int ROW = node_to_row[START]; // FIXME(clang-tidy): unchecked operator[], consider
                                            // .at(); non-constant array index

        for (int chunk: std::views::iota(0, CHUNKS)) {
            int cur = START;
            std::uint32_t emitted_sym = SYM_NONE;
            bool invalid = false;

            // Walk this chunk's W bits through the trie, one at a time. Landing on a leaf
            // mid-chunk emits a symbol and resets back to the trie root for the remaining bits.
            for (int shift: std::views::iota(0, W) | std::views::reverse) {
                const int BIT = (chunk >> shift) & 1;
                const int NEXT =
                    trie.m_nodes[cur].m_child[BIT]; // FIXME(clang-tidy): unchecked operator[],
                                                    // consider .at(); non-constant array index
                if (NEXT < 0) {
                    invalid = true;
                    break;
                }
                cur = NEXT;
                if (trie.m_nodes[cur].m_sym !=
                    SYM_NONE) { // FIXME(clang-tidy): unchecked operator[], consider .at();
                                // non-constant array index
                    emitted_sym =
                        trie.m_nodes[cur].m_sym; // FIXME(clang-tidy): unchecked operator[],
                                                 // consider .at(); non-constant array index
                    cur = 0;
                }
            }

            // Invalid chunk (no such Huffman code) gets a sentinel; otherwise record the
            // destination row (allocating one if this node's new) plus whatever symbol emitted.
            const std::size_t SLOT = (static_cast<std::size_t>(ROW) * CHUNKS) + chunk;
            if (invalid) {
                table.m_entries[SLOT] = {
                    0xFF'FFU, 0xFF'FFU
                }; // FIXME(clang-tidy): unchecked operator[], consider .at(); non-constant
                   // array index
            } else {
                if (node_to_row[cur] < 0) { // FIXME(clang-tidy): unchecked operator[], consider
                                            // .at(); non-constant array index
                    alloc_row(cur);
                    queue[q_tail++] = cur; // FIXME(clang-tidy): unchecked operator[], consider
                                           // .at(); non-constant array index
                }
                const std::uint16_t SYM16 =
                    (emitted_sym == SYM_NONE) ? 0xFF'FEU : static_cast<std::uint16_t>(emitted_sym);
                table.m_entries[SLOT] = {
                    static_cast<std::uint16_t>(node_to_row[cur]), SYM16
                }; // FIXME(clang-tidy): unchecked operator[], consider .at(); non-constant
                   // array index
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
template<std::ranges::input_range R>
    requires std::convertible_to<std::ranges::range_value_t<R>, std::byte>
class HuffmanEncodeView
{
public:
    class Iterator
    {
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
         * @brief Attaches the iterator to `base` and primes the bit accumulator by pulling in
         * the first symbol's Huffman code.
         * @param base the input range to encode; iterator holds begin/end into it, not a copy.
         */
        explicit Iterator(R& base) :
            m_inner{std::ranges::begin(base)},
            m_end{std::ranges::end(base)}
        {
            advance();
        }

        /**
         * @brief Reads the next fully-buffered output byte off the top of the bit accumulator.
         * @return the byte.
         */
        [[nodiscard]] std::byte operator*() const noexcept
        {
            return std::byte{static_cast<unsigned char>(m_bits >> (m_shift - 8))};
        }

        /**
         * @brief Consumes the byte just read and pulls more Huffman-coded bits in behind it.
         * @return `*this`, advanced.
         */
        Iterator& operator++()
        {
            m_shift -= 8;
            advance();
            return *this;
        }

        /**
         * @brief Postfix increment — same motion as prefix, just discards the "old value"
         * nobody asked for.
         */
        void operator++(int)
        {
            ++*this;
        }

        /**
         * @brief Sentinel comparison — this is how range-for knows the view's exhausted.
         * @return true once every input symbol's been consumed and the final padded byte's been
         * emitted.
         */
        [[nodiscard]] bool operator==(std::default_sentinel_t /*unused*/) const noexcept
        {
            return m_done;
        }

    private:
        /**
         * @brief Pumps more Huffman-coded bits into the 64-bit MSB-first accumulator — either
         * the next input symbol's code, or (once input's exhausted) the EOS padding bits (RFC
         * 7541 §5.2: pad with the high-order bits of the EOS code, i.e. all 1s) to round out
         * the final byte.
         * @note Sets `m_done` once there's nothing left to emit — no more input and the
         * accumulator's fully drained (shift back to 0).
         */
        void advance()
        {
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
    explicit HuffmanEncodeView(R base) :
        m_base{std::move(base)}
    {
    }

    /** @brief Trivial dtor — `m_base` cleans up its own storage, no motion needed here. */
    ~HuffmanEncodeView() = default;

    /**
     * @brief Deleted — copying the base range could be arbitrarily expensive/wrong for
     * arbitrary `R`, so it's off the table.
     */
    HuffmanEncodeView(const HuffmanEncodeView&) = delete;
    /** @brief Deleted for the same reason as the copy ctor. */
    HuffmanEncodeView& operator=(const HuffmanEncodeView&) = delete;
    /** @brief Defaulted — moving just relocates `m_base`, nothing fancy. */
    HuffmanEncodeView(HuffmanEncodeView&&) = default;
    /** @brief Defaulted move-assign, same deal as the move ctor. */
    HuffmanEncodeView& operator=(HuffmanEncodeView&&) = default;

    /**
     * @brief Gets a fresh Iterator primed on `m_base` — starts the lazy encode.
     * @return an Iterator at the start.
     */
    [[nodiscard]] Iterator begin()
    {
        return Iterator{m_base};
    }

    /** @brief Gets the sentinel that marks "encoding's done". @return a default_sentinel_t. */
    [[nodiscard]] std::default_sentinel_t end() const noexcept
    {
        return {};
    }

private:
    R m_base;
};

template<std::ranges::input_range R>
HuffmanEncodeView(R&&) -> HuffmanEncodeView<std::views::all_t<R>>;

// HuffmanDecodeView
//
// Lazy char-producing range over a std::byte input sequence.
//
// Algorithm: W-bit chunk FSM, buffer-drain pattern.
template<int W, std::ranges::input_range R>
    requires DecodeWidth<W> && std::same_as<std::ranges::range_value_t<R>, std::byte>
class HuffmanDecodeView
{
public:
    static constexpr int CHUNKS = 1 << W;
    static constexpr int CHUNKS_PER_BYTE = 8 / W;
    static constexpr int CHUNK_MASK = CHUNKS - 1;

    // TODO: make constexpr when GCC supports it
    // Lazily built on first use (C++11 magic-statics, thread-safe) instead of a static-duration
    // member initialized at load time — build_table<W>() can throw, and this way the throw
    // happens on first real use instead of during static initialization, where it can't be
    // caught.
    [[nodiscard]] static const TransTable<W>& get_table()
    {
        static const TransTable<W> TABLE = build_table<W>();
        return TABLE;
    }

    class Iterator
    {
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
         * @brief Attaches the iterator to `base` and decodes the first output character to
         * prime it.
         * @param base the input byte range to Huffman-decode; iterator holds begin/end into it.
         */
        explicit Iterator(R& base) :
            m_inner{std::ranges::begin(base)},
            m_end{std::ranges::end(base)}
        {
            advance();
        }

        /**
         * @brief Gets the currently-decoded output character.
         * @return the last symbol emitted by advance().
         */
        [[nodiscard]] char operator*() const noexcept
        {
            return m_current;
        }

        /**
         * @brief Decodes the next output character.
         * @return `*this`, advanced past the one just emitted.
         */
        Iterator& operator++()
        {
            advance();
            return *this;
        }

        /**
         * @brief Postfix increment — same motion as prefix, discards the "old value" nobody
         * asked for.
         */
        void operator++(int)
        {
            ++*this;
        }

        /**
         * @brief Sentinel comparison — how range-for knows the decode's finished.
         * @return true once the FSM's fully drained the input with no more symbols to emit.
         */
        [[nodiscard]] bool operator==(std::default_sentinel_t /*unused*/) const noexcept
        {
            return m_done;
        }

    private:
        /**
         * @brief Drives the W-bit chunk FSM forward, table-lookup-decoding chunks until a real
         * symbol pops out (`sym < 256`), then stashes it in `m_current` and returns.
         * @note Runs the transition table (`TABLE`) chunk-by-chunk against the input bytes,
         * tracking `m_fsm` state across calls — this is the whole decode loop, one character at
         * a time, lazy.
         * @warning A malformed Huffman stream doesn't fail quietly: an invalid code (`sym ==
         * 0xFFFF`), an EOS symbol showing up mid-stream, or a truncated stream with too many
         * leftover padding bits (`m_padding_bits > 7`) all throw
         * `error::http::HuffmanDecodeError`. This is untrusted wire data — treat every throw as
         * a hostile-input signal, not a bug.
         */
        void advance()
        {
            // Loop chunk-by-chunk until a real symbol pops out or the stream runs dry.
            while (true) {
                // Input's exhausted — only a clean finish if we're byte-aligned and any
                // leftover padding is a valid EOS prefix (7 bits or fewer); anything else is a
                // cut-off stream.
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

                // Table lookup drives the FSM to its next state and (maybe) hands back a
                // symbol.
                const auto& [ns, sym] = get_table().m_entries
                                            [(static_cast<std::size_t>(m_fsm) * CHUNKS) +
                                             CHUNK]; // FIXME(clang-tidy): unchecked operator[],
                                                     // consider .at(); non-constant array index
                m_fsm = ns;

                // A real symbol (< 256) is the common case — stash it and return. Otherwise
                // it's either a hard decode error (invalid code, stray EOS) or just more
                // padding bits accumulating with no symbol yet, so keep looping.
                if (sym < 256U) [[likely]] {
                    m_current = static_cast<char>(sym);
                    m_padding_bits = 0;
                    return;
                } else if (sym == 0xFF'FFU) [[unlikely]] {
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
    explicit HuffmanDecodeView(R&& base) :
        m_base{std::move(base)}
    {
    }

    /** @brief Trivial dtor — `m_base` cleans up its own storage, no motion needed here. */
    ~HuffmanDecodeView() = default;

    /**
     * @brief Deleted — copying the base range could be arbitrarily expensive/wrong for
     * arbitrary `R`.
     */
    HuffmanDecodeView(const HuffmanDecodeView&) = delete;
    /** @brief Deleted for the same reason as the copy ctor. */
    HuffmanDecodeView& operator=(const HuffmanDecodeView&) = delete;
    /** @brief Defaulted — moving just relocates `m_base`, nothing fancy. */
    HuffmanDecodeView(HuffmanDecodeView&&) = default;
    /** @brief Defaulted move-assign, same deal as the move ctor. */
    HuffmanDecodeView& operator=(HuffmanDecodeView&&) = default;

    /**
     * @brief Gets a fresh Iterator primed on `m_base` — starts the lazy decode.
     * @return an Iterator at the start.
     */
    [[nodiscard]] Iterator begin()
    {
        return Iterator{m_base};
    }

    /** @brief Gets the sentinel that marks "decoding's done". @return a default_sentinel_t. */
    [[nodiscard]] std::default_sentinel_t end() const noexcept
    {
        return {};
    }

private:
    R m_base;
};

struct HuffmanEncodeAdaptor : std::ranges::range_adaptor_closure<HuffmanEncodeAdaptor>
{
    /**
     * @brief Pipe-adaptor call: turns `range | HuffmanEncodeAdaptor{}` into a lazy
     * HuffmanEncodeView over it. This is what makes `some_range | huffman::Huffman<>::encode()`
     * read clean, no cap.
     * @tparam R the viewable range type piped in.
     * @param range the char/byte range to encode.
     * @return a HuffmanEncodeView wrapping `range`.
     */
    template<std::ranges::viewable_range R>
        requires std::convertible_to<std::ranges::range_value_t<R>, std::byte>
    [[nodiscard]] auto operator()(R&& range) const
    {
        return HuffmanEncodeView<std::views::all_t<R>>{std::views::all(std::forward<R>(range))};
    }
};

template<int W = 4>
    requires DecodeWidth<W>
struct HuffmanDecodeAdaptor : std::ranges::range_adaptor_closure<HuffmanDecodeAdaptor<W>>
{
    /**
     * @brief Pipe-adaptor call: turns `range | HuffmanDecodeAdaptor<W>{}` into a lazy
     * HuffmanDecodeView over it.
     * @tparam R the viewable range type piped in.
     * @param range the `std::byte` range to decode.
     * @return a HuffmanDecodeView<W, ...> wrapping `range`.
     */
    template<std::ranges::viewable_range R>
        requires std::same_as<std::ranges::range_value_t<R>, std::byte>
    [[nodiscard]] auto operator()(R&& range) const
    {
        return HuffmanDecodeView<W, std::views::all_t<R>>{std::views::all(std::forward<R>(range))};
    }
};

template<int W = 4>
    requires DecodeWidth<W>
struct Huffman
{
    /**
     * @brief Gets a pipeable encode adaptor.
     * @return a fresh HuffmanEncodeAdaptor, stateless so any instance works.
     */
    [[nodiscard]] static HuffmanEncodeAdaptor encode() noexcept
    {
        return {};
    }

    /**
     * @brief Gets a pipeable decode adaptor at chunk-width `W`.
     * @return a fresh HuffmanDecodeAdaptor<W>, stateless.
     */
    [[nodiscard]] static HuffmanDecodeAdaptor<W> decode() noexcept
    {
        return {};
    }
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
        for (std::byte value: byte_view | Huffman<4>::encode()) {
            encoded.push_back(value);
        }
        expect(encoded.empty());

        std::string decoded;
        for (char character: encoded | Huffman<4>::decode()) {
            decoded += character;
        }
        expect(decoded == original);
    };

    "round-trips a short ASCII string"_test = [] {
        std::string original = "hello world";
        std::vector<std::byte> byte_view;
        for (char character: original) {
            byte_view.push_back(static_cast<std::byte>(character));
        }

        std::vector<std::byte> encoded;
        for (std::byte value: byte_view | Huffman<4>::encode()) {
            encoded.push_back(value);
        }

        std::string decoded;
        for (char character: encoded | Huffman<4>::decode()) {
            decoded += character;
        }
        expect(decoded == original);
    };

    "matches the RFC 7541 C.4.1 known encoding for \"www.example.com\""_test = [] {
        std::string original = "www.example.com";
        std::vector<std::byte> byte_view;
        for (char character: original) {
            byte_view.push_back(static_cast<std::byte>(character));
        }

        std::vector<std::byte> encoded;
        for (std::byte value: byte_view | Huffman<4>::encode()) {
            encoded.push_back(value);
        }

        std::vector<std::byte> expected{std::byte{0xF1}, std::byte{0xE3}, std::byte{0xC2},
                                        std::byte{0xE5}, std::byte{0xF2}, std::byte{0x3A},
                                        std::byte{0x6B}, std::byte{0xA0}, std::byte{0xAB},
                                        std::byte{0x90}, std::byte{0xF4}, std::byte{0xFF}};
        // Plain `==` on two std::vector<std::byte> forces boost::ut's failure-diagnostic
        // printer to instantiate operator<<(ostream&, std::byte), which doesn't exist —
        // wrapping in std::ranges::equal() keeps the comparison a plain bool instead.
        expect(std::ranges::equal(encoded, expected));
    };

    "decoding an all-ones stream throws HuffmanDecodeError"_test = [] {
        std::vector<std::byte> garbage{
            std::byte{0xFF}, std::byte{0xFF}, std::byte{0xFF}, std::byte{0xFF}
        };

        expect(throws<error::http::HuffmanDecodeError>([&] {
            std::string decoded;
            for (char character: garbage | Huffman<4>::decode()) {
                decoded += character;
            }
        }));
    };
};

} // namespace io::shared_codec::huffman::tests
#endif
