export module shared:huffman;
import std;
import error;
import :types;

namespace codec::shared::huffman {

inline constexpr std::uint32_t SYM_COUNT = 257;
inline constexpr std::uint32_t SYM_EOS = 256;
inline constexpr std::uint32_t SYM_NONE = 0xFFFF'FFFEu;
inline constexpr std::uint32_t SYM_INVALID = 0xFFFF'FFFFu;

// Upper bounds for compile-time arrays.
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
    int child[2] = {-1, -1};
    std::uint32_t sym = SYM_NONE;
};

struct Trie {
    std::array<TrieNode, TRIE_CAP> nodes{};
    std::size_t size = 1;
};

consteval Trie build_trie() {
    Trie t;
    for (std::uint32_t sym = 0; sym < SYM_COUNT; ++sym) {
        const auto [bits, len] = CODES[sym];
        int cur = 0;
        for (int i = len - 1; i >= 0; --i) {
            const int b = (bits >> i) & 1;
            if (t.nodes[cur].child[b] < 0) {
                t.nodes[cur].child[b] = static_cast<int>(t.size++);
            }
            cur = t.nodes[cur].child[b];
        }
        t.nodes[cur].sym = sym;
    }
    return t;
}


template <int W>
struct TransTable {
    static constexpr int CHUNK_COUNT = 1 << W;
    std::array<std::pair<std::uint16_t, std::uint16_t>, TABLE_CAP> entries{};
    std::uint16_t row_count = 0;
};

template <int W>
consteval TransTable<W> build_table() {
    constexpr int CHUNKS = TransTable<W>::CHUNK_COUNT;
    constexpr Trie trie = build_trie();
    TransTable<W> table;

    std::array<int, TRIE_CAP> node_to_row;
    node_to_row.fill(-1);
    std::array<int, TRIE_CAP> queue{};
    int q_head = 0, q_tail = 0;

    auto alloc_row = [&](int node) -> int {
        const int row = static_cast<int>(table.row_count++);
        node_to_row[node] = row;
        return row;
    };

    alloc_row(0);
    queue[q_tail++] = 0;

    while (q_head < q_tail) {
        const int start_node = queue[q_head++];
        const int row = node_to_row[start_node];

        for (int chunk = 0; chunk < CHUNKS; ++chunk) {
            int cur = start_node;
            std::uint32_t emitted_sym = SYM_NONE;
            bool invalid = false;

            for (int i = W - 1; i >= 0; --i) {
                const int bit = (chunk >> i) & 1;
                const int next = trie.nodes[cur].child[bit];
                if (next < 0) {
                    invalid = true;
                    break;
                }
                cur = next;
                if (trie.nodes[cur].sym != SYM_NONE) {
                    emitted_sym = trie.nodes[cur].sym;
                    cur = 0;
                }
            }

            const std::size_t slot = static_cast<std::size_t>(row) * CHUNKS + chunk;
            if (invalid) {
                table.entries[slot] = {0xFFFFu, 0xFFFFu};
            } else {
                if (node_to_row[cur] < 0) {
                    alloc_row(cur);
                    queue[q_tail++] = cur;
                }
                std::uint16_t sym16 = (emitted_sym == SYM_NONE) ? 0xFFFEu : static_cast<std::uint16_t>(emitted_sym);
                table.entries[slot] = {static_cast<std::uint16_t>(node_to_row[cur]), sym16};
            }
        }
    }
    return table;
}

} // namespace codec::shared::huffman

export namespace codec::shared::huffman {

template <int W = 4>
    requires DecodeWidth<W>
class Huffman {

  public:
    Huffman() = default;

    template <std::output_iterator<std::uint8_t> Out>
    void encode(std::string_view input, Out out) const noexcept {
        std::uint64_t buf = 0;
        int bits = 0;

        for (const unsigned char chr : input) {
            const auto &[code_bits, code_len] = CODES[chr];
            buf = (buf << code_len) | code_bits;
            bits += code_len;

            while (bits >= 8) {
                bits -= 8;
                *out++ = static_cast<std::uint8_t>(buf >> bits);
            }
        }
        if (bits > 0) {
            *out++ = static_cast<std::uint8_t>((buf << (8 - bits)) | (0xFF >> bits));
        }
    }

    [[nodiscard]] std::string decode(std::span<const std::uint8_t> data) const {
        if (data.empty())
            return {};

        std::string result;
        result.reserve(data.size() + (data.size() >> 1));

        std::uint32_t state = 0;
        int padding_bits = 0;

        const std::uint8_t *ptr = data.data();
        const std::uint8_t *end = ptr + data.size();

        while (ptr < end - 1) {
            unsigned int temp = *ptr++;
            for (int i = 0; i < CHUNKS_PER_BYTE; ++i) {
                const int chunk = (temp >> (8 - W)) & CHUNK_MASK;
                apply_step(state, chunk, result, padding_bits, false);
                temp <<= W;
            }
        }

        unsigned int last_byte = *ptr;
        for (int i = 0; i < CHUNKS_PER_BYTE; ++i) {
            const int chunk = (last_byte >> (8 - W)) & CHUNK_MASK;
            apply_step(state, chunk, result, padding_bits, true);
            last_byte <<= W;
        }

        if (padding_bits > 7) [[unlikely]]
            throw error::http::HuffmanDecodeError{"padding exceeds 7 bits"};

        if (state != 0) [[unlikely]]
            throw error::http::HuffmanDecodeError{"stream ends mid-symbol"};

        return result;
    }

  private:
    static constexpr int CHUNKS = 1 << W;
    static constexpr auto TABLE = build_table<W>();
    static constexpr int CHUNKS_PER_BYTE = 8 / W;
    static constexpr int CHUNK_MASK = CHUNKS - 1;

    inline void apply_step(std::uint32_t &state, int chunk, std::string &result, int &padding_bits,
                           bool is_last) const {
        const auto &entry = TABLE.entries[(state << W) | chunk];
        state = entry.first;
        const std::uint16_t sym = entry.second;

        if (sym < 256) [[likely]] {
            result.push_back(static_cast<char>(sym));
            padding_bits = 0;
        } else if (sym == 0xFFFEu) [[likely]] {
            if (is_last)
                padding_bits += W;
        } else if (sym == 256) {
            throw error::http::HuffmanDecodeError{"EOS found"};
        } else [[unlikely]] {
            throw error::http::HuffmanDecodeError{"invalid code"};
        }
    }
};

} // namespace codec::shared::huffman
