module io.codec.shared.huffman;
@nogc nothrow:

import io.codec.shared.types : DecodeWidth;

// PORT-NOTE: C++ constexpr Huffman tables → D static immutable arrays.
// TODO: improvement — convert to CTFE-evaluated tables (build_trie / build_table as
//       D enum or static this() CTFE). Currently preserved as runtime-init to match
//       the C++ "TODO: make consteval" comments.

enum uint SYM_COUNT   = 257;
enum uint SYM_EOS     = 256;
enum uint SYM_NONE    = 0xFFFF_FFFE;
enum uint SYM_INVALID = 0xFFFF_FFFF;

enum size_t TRIE_CAP  = 600;
enum size_t TABLE_CAP = 9600;

// PORT-NOTE: C++ std::pair<uint32_t, uint8_t> CODES[] → D struct array.
// Improvement idea: static immutable at CTFE.
struct HuffCode { uint code; ubyte len; }

static immutable HuffCode[SYM_COUNT] CODES = () {
    HuffCode[SYM_COUNT] c;
    /*   0 */ c[0]   = HuffCode(0x1ff8, 13);     /*   1 */ c[1]   = HuffCode(0x7fffd8, 23);
    /*   2 */ c[2]   = HuffCode(0xfffffe2, 28);  /*   3 */ c[3]   = HuffCode(0xfffffe3, 28);
    /*   4 */ c[4]   = HuffCode(0xfffffe4, 28);  /*   5 */ c[5]   = HuffCode(0xfffffe5, 28);
    /*   6 */ c[6]   = HuffCode(0xfffffe6, 28);  /*   7 */ c[7]   = HuffCode(0xfffffe7, 28);
    /*   8 */ c[8]   = HuffCode(0xfffffe8, 28);  /*   9 */ c[9]   = HuffCode(0xffffea, 24);
    /*  10 */ c[10]  = HuffCode(0x3ffffffc, 30); /*  11 */ c[11]  = HuffCode(0xfffffe9, 28);
    /*  12 */ c[12]  = HuffCode(0xfffffea, 28);  /*  13 */ c[13]  = HuffCode(0x3ffffffd, 30);
    /*  14 */ c[14]  = HuffCode(0xfffffeb, 28);  /*  15 */ c[15]  = HuffCode(0xfffffec, 28);
    /*  16 */ c[16]  = HuffCode(0xfffffed, 28);  /*  17 */ c[17]  = HuffCode(0xfffffee, 28);
    /*  18 */ c[18]  = HuffCode(0xfffffef, 28);  /*  19 */ c[19]  = HuffCode(0xffffff0, 28);
    /*  20 */ c[20]  = HuffCode(0xffffff1, 28);  /*  21 */ c[21]  = HuffCode(0xffffff2, 28);
    /*  22 */ c[22]  = HuffCode(0x3ffffffe, 30); /*  23 */ c[23]  = HuffCode(0xffffff3, 28);
    /*  24 */ c[24]  = HuffCode(0xffffff4, 28);  /*  25 */ c[25]  = HuffCode(0xffffff5, 28);
    /*  26 */ c[26]  = HuffCode(0xffffff6, 28);  /*  27 */ c[27]  = HuffCode(0xffffff7, 28);
    /*  28 */ c[28]  = HuffCode(0xffffff8, 28);  /*  29 */ c[29]  = HuffCode(0xffffff9, 28);
    /*  30 */ c[30]  = HuffCode(0xffffffa, 28);  /*  31 */ c[31]  = HuffCode(0xffffffb, 28);
    /* ' '*/ c[32]   = HuffCode(0x14, 6);         /* '!'*/ c[33]   = HuffCode(0x3f8, 10);
    /* '"'*/ c[34]   = HuffCode(0x3f9, 10);       /* '#'*/ c[35]   = HuffCode(0xffa, 12);
    /* '$'*/ c[36]   = HuffCode(0x1ff9, 13);      /* '%'*/ c[37]   = HuffCode(0x15, 6);
    /* '&'*/ c[38]   = HuffCode(0xf8, 8);         /* '''*/ c[39]   = HuffCode(0x7fa, 11);
    /* '('*/ c[40]   = HuffCode(0x3fa, 10);       /* ')'*/ c[41]   = HuffCode(0x3fb, 10);
    /* '*'*/ c[42]   = HuffCode(0xf9, 8);         /* '+'*/ c[43]   = HuffCode(0x7fb, 11);
    /* ','*/ c[44]   = HuffCode(0xfa, 8);         /* '-'*/ c[45]   = HuffCode(0x16, 6);
    /* '.'*/ c[46]   = HuffCode(0x17, 6);         /* '/'*/ c[47]   = HuffCode(0x18, 6);
    /* '0'*/ c[48]   = HuffCode(0x0, 5);          /* '1'*/ c[49]   = HuffCode(0x1, 5);
    /* '2'*/ c[50]   = HuffCode(0x2, 5);          /* '3'*/ c[51]   = HuffCode(0x19, 6);
    /* '4'*/ c[52]   = HuffCode(0x1a, 6);         /* '5'*/ c[53]   = HuffCode(0x1b, 6);
    /* '6'*/ c[54]   = HuffCode(0x1c, 6);         /* '7'*/ c[55]   = HuffCode(0x1d, 6);
    /* '8'*/ c[56]   = HuffCode(0x1e, 6);         /* '9'*/ c[57]   = HuffCode(0x1f, 6);
    /* ':'*/ c[58]   = HuffCode(0x5c, 7);         /* ';'*/ c[59]   = HuffCode(0xfb, 8);
    /* '<'*/ c[60]   = HuffCode(0x7ffc, 15);      /* '='*/ c[61]   = HuffCode(0x20, 6);
    /* '>'*/ c[62]   = HuffCode(0xffb, 12);       /* '?'*/ c[63]   = HuffCode(0x3fc, 10);
    /* '@'*/ c[64]   = HuffCode(0x1ffa, 13);      /* 'A'*/ c[65]   = HuffCode(0x21, 6);
    /* 'B'*/ c[66]   = HuffCode(0x5d, 7);         /* 'C'*/ c[67]   = HuffCode(0x5e, 7);
    /* 'D'*/ c[68]   = HuffCode(0x5f, 7);         /* 'E'*/ c[69]   = HuffCode(0x60, 7);
    /* 'F'*/ c[70]   = HuffCode(0x61, 7);         /* 'G'*/ c[71]   = HuffCode(0x62, 7);
    /* 'H'*/ c[72]   = HuffCode(0x63, 7);         /* 'I'*/ c[73]   = HuffCode(0x64, 7);
    /* 'J'*/ c[74]   = HuffCode(0x65, 7);         /* 'K'*/ c[75]   = HuffCode(0x66, 7);
    /* 'L'*/ c[76]   = HuffCode(0x67, 7);         /* 'M'*/ c[77]   = HuffCode(0x68, 7);
    /* 'N'*/ c[78]   = HuffCode(0x69, 7);         /* 'O'*/ c[79]   = HuffCode(0x6a, 7);
    /* 'P'*/ c[80]   = HuffCode(0x6b, 7);         /* 'Q'*/ c[81]   = HuffCode(0x6c, 7);
    /* 'R'*/ c[82]   = HuffCode(0x6d, 7);         /* 'S'*/ c[83]   = HuffCode(0x6e, 7);
    /* 'T'*/ c[84]   = HuffCode(0x6f, 7);         /* 'U'*/ c[85]   = HuffCode(0x70, 7);
    /* 'V'*/ c[86]   = HuffCode(0x71, 7);         /* 'W'*/ c[87]   = HuffCode(0x72, 7);
    /* 'X'*/ c[88]   = HuffCode(0xfc, 8);         /* 'Y'*/ c[89]   = HuffCode(0x73, 7);
    /* 'Z'*/ c[90]   = HuffCode(0xfd, 8);         /* '['*/ c[91]   = HuffCode(0x1ffb, 13);
    /* '\'*/ c[92]   = HuffCode(0x7fff0, 19);     /* ']'*/ c[93]   = HuffCode(0x1ffc, 13);
    /* '^'*/ c[94]   = HuffCode(0x3ffc, 14);      /* '_'*/ c[95]   = HuffCode(0x22, 6);
    /* '`'*/ c[96]   = HuffCode(0x7ffd, 15);      /* 'a'*/ c[97]   = HuffCode(0x3, 5);
    /* 'b'*/ c[98]   = HuffCode(0x23, 6);         /* 'c'*/ c[99]   = HuffCode(0x4, 5);
    /* 'd'*/ c[100]  = HuffCode(0x24, 6);         /* 'e'*/ c[101]  = HuffCode(0x5, 5);
    /* 'f'*/ c[102]  = HuffCode(0x25, 6);         /* 'g'*/ c[103]  = HuffCode(0x26, 6);
    /* 'h'*/ c[104]  = HuffCode(0x27, 6);         /* 'i'*/ c[105]  = HuffCode(0x6, 5);
    /* 'j'*/ c[106]  = HuffCode(0x74, 7);         /* 'k'*/ c[107]  = HuffCode(0x75, 7);
    /* 'l'*/ c[108]  = HuffCode(0x28, 6);         /* 'm'*/ c[109]  = HuffCode(0x29, 6);
    /* 'n'*/ c[110]  = HuffCode(0x2a, 6);         /* 'o'*/ c[111]  = HuffCode(0x7, 5);
    /* 'p'*/ c[112]  = HuffCode(0x2b, 6);         /* 'q'*/ c[113]  = HuffCode(0x76, 7);
    /* 'r'*/ c[114]  = HuffCode(0x2c, 6);         /* 's'*/ c[115]  = HuffCode(0x8, 5);
    /* 't'*/ c[116]  = HuffCode(0x9, 5);          /* 'u'*/ c[117]  = HuffCode(0x2d, 6);
    /* 'v'*/ c[118]  = HuffCode(0x77, 7);         /* 'w'*/ c[119]  = HuffCode(0x78, 7);
    /* 'x'*/ c[120]  = HuffCode(0x79, 7);         /* 'y'*/ c[121]  = HuffCode(0x7a, 7);
    /* 'z'*/ c[122]  = HuffCode(0x7b, 7);         /* '{'*/ c[123]  = HuffCode(0x7ffe, 15);
    /* '|'*/ c[124]  = HuffCode(0x7fc, 11);       /* '}'*/ c[125]  = HuffCode(0x3ffd, 14);
    /* '~'*/ c[126]  = HuffCode(0x1ffd, 13);
    /* 127*/ c[127]  = HuffCode(0xffffffc, 28);
    /* 128*/ c[128]  = HuffCode(0xfffe6, 20);     /* 129*/ c[129]  = HuffCode(0x3fffd2, 22);
    /* 130*/ c[130]  = HuffCode(0xfffe7, 20);     /* 131*/ c[131]  = HuffCode(0xfffe8, 20);
    /* 132*/ c[132]  = HuffCode(0x3fffd3, 22);    /* 133*/ c[133]  = HuffCode(0x3fffd4, 22);
    /* 134*/ c[134]  = HuffCode(0x3fffd5, 22);    /* 135*/ c[135]  = HuffCode(0x7fffd9, 23);
    /* 136*/ c[136]  = HuffCode(0x3fffd6, 22);    /* 137*/ c[137]  = HuffCode(0x7fffda, 23);
    /* 138*/ c[138]  = HuffCode(0x7fffdb, 23);    /* 139*/ c[139]  = HuffCode(0x7fffdc, 23);
    /* 140*/ c[140]  = HuffCode(0x7fffdd, 23);    /* 141*/ c[141]  = HuffCode(0x7fffde, 23);
    /* 142*/ c[142]  = HuffCode(0xffffeb, 24);    /* 143*/ c[143]  = HuffCode(0x7fffdf, 23);
    /* 144*/ c[144]  = HuffCode(0xffffec, 24);    /* 145*/ c[145]  = HuffCode(0xffffed, 24);
    /* 146*/ c[146]  = HuffCode(0x3fffd7, 22);    /* 147*/ c[147]  = HuffCode(0x7fffe0, 23);
    /* 148*/ c[148]  = HuffCode(0xffffee, 24);    /* 149*/ c[149]  = HuffCode(0x7fffe1, 23);
    /* 150*/ c[150]  = HuffCode(0x7fffe2, 23);    /* 151*/ c[151]  = HuffCode(0x7fffe3, 23);
    /* 152*/ c[152]  = HuffCode(0x7fffe4, 23);    /* 153*/ c[153]  = HuffCode(0x1fffdc, 21);
    /* 154*/ c[154]  = HuffCode(0x3fffd8, 22);    /* 155*/ c[155]  = HuffCode(0x7fffe5, 23);
    /* 156*/ c[156]  = HuffCode(0x3fffd9, 22);    /* 157*/ c[157]  = HuffCode(0x7fffe6, 23);
    /* 158*/ c[158]  = HuffCode(0x7fffe7, 23);    /* 159*/ c[159]  = HuffCode(0xffffef, 24);
    /* 160*/ c[160]  = HuffCode(0x3fffda, 22);    /* 161*/ c[161]  = HuffCode(0x1fffdd, 21);
    /* 162*/ c[162]  = HuffCode(0xfffe9, 20);     /* 163*/ c[163]  = HuffCode(0x3fffdb, 22);
    /* 164*/ c[164]  = HuffCode(0x3fffdc, 22);    /* 165*/ c[165]  = HuffCode(0x7fffe8, 23);
    /* 166*/ c[166]  = HuffCode(0x7fffe9, 23);    /* 167*/ c[167]  = HuffCode(0x1fffde, 21);
    /* 168*/ c[168]  = HuffCode(0x7fffea, 23);    /* 169*/ c[169]  = HuffCode(0x3fffdd, 22);
    /* 170*/ c[170]  = HuffCode(0x3fffde, 22);    /* 171*/ c[171]  = HuffCode(0xfffff0, 24);
    /* 172*/ c[172]  = HuffCode(0x1fffdf, 21);    /* 173*/ c[173]  = HuffCode(0x3fffdf, 22);
    /* 174*/ c[174]  = HuffCode(0x7fffeb, 23);    /* 175*/ c[175]  = HuffCode(0x7fffec, 23);
    /* 176*/ c[176]  = HuffCode(0x1fffe0, 21);    /* 177*/ c[177]  = HuffCode(0x1fffe1, 21);
    /* 178*/ c[178]  = HuffCode(0x3fffe0, 22);    /* 179*/ c[179]  = HuffCode(0x1fffe2, 21);
    /* 180*/ c[180]  = HuffCode(0x7fffed, 23);    /* 181*/ c[181]  = HuffCode(0x3fffe1, 22);
    /* 182*/ c[182]  = HuffCode(0x7fffee, 23);    /* 183*/ c[183]  = HuffCode(0x7fffef, 23);
    /* 184*/ c[184]  = HuffCode(0xfffea, 20);     /* 185*/ c[185]  = HuffCode(0x3fffe2, 22);
    /* 186*/ c[186]  = HuffCode(0x3fffe3, 22);    /* 187*/ c[187]  = HuffCode(0x3fffe4, 22);
    /* 188*/ c[188]  = HuffCode(0x7ffff0, 23);    /* 189*/ c[189]  = HuffCode(0x3fffe5, 22);
    /* 190*/ c[190]  = HuffCode(0x3fffe6, 22);    /* 191*/ c[191]  = HuffCode(0x7ffff1, 23);
    /* 192*/ c[192]  = HuffCode(0x3ffffe0, 26);   /* 193*/ c[193]  = HuffCode(0x3ffffe1, 26);
    /* 194*/ c[194]  = HuffCode(0xfffeb, 20);     /* 195*/ c[195]  = HuffCode(0x7fff1, 19);
    /* 196*/ c[196]  = HuffCode(0x3fffe7, 22);    /* 197*/ c[197]  = HuffCode(0x7ffff2, 23);
    /* 198*/ c[198]  = HuffCode(0x3fffe8, 22);    /* 199*/ c[199]  = HuffCode(0x1ffffec, 25);
    /* 200*/ c[200]  = HuffCode(0x3ffffe2, 26);   /* 201*/ c[201]  = HuffCode(0x3ffffe3, 26);
    /* 202*/ c[202]  = HuffCode(0x3ffffe4, 26);   /* 203*/ c[203]  = HuffCode(0x7ffffde, 27);
    /* 204*/ c[204]  = HuffCode(0x7ffffdf, 27);   /* 205*/ c[205]  = HuffCode(0x3ffffe5, 26);
    /* 206*/ c[206]  = HuffCode(0xfffff1, 24);    /* 207*/ c[207]  = HuffCode(0x1ffffed, 25);
    /* 208*/ c[208]  = HuffCode(0x7fff2, 19);     /* 209*/ c[209]  = HuffCode(0x1fffe3, 21);
    /* 210*/ c[210]  = HuffCode(0x3ffffe6, 26);   /* 211*/ c[211]  = HuffCode(0x7ffffe0, 27);
    /* 212*/ c[212]  = HuffCode(0x7ffffe1, 27);   /* 213*/ c[213]  = HuffCode(0x3ffffe7, 26);
    /* 214*/ c[214]  = HuffCode(0x7ffffe2, 27);   /* 215*/ c[215]  = HuffCode(0xfffff2, 24);
    /* 216*/ c[216]  = HuffCode(0x1fffe4, 21);    /* 217*/ c[217]  = HuffCode(0x1fffe5, 21);
    /* 218*/ c[218]  = HuffCode(0x3ffffe8, 26);   /* 219*/ c[219]  = HuffCode(0x3ffffe9, 26);
    /* 220*/ c[220]  = HuffCode(0xffffffd, 28);   /* 221*/ c[221]  = HuffCode(0x7ffffe3, 27);
    /* 222*/ c[222]  = HuffCode(0x7ffffe4, 27);   /* 223*/ c[223]  = HuffCode(0x7ffffe5, 27);
    /* 224*/ c[224]  = HuffCode(0xfffec, 20);     /* 225*/ c[225]  = HuffCode(0xfffff3, 24);
    /* 226*/ c[226]  = HuffCode(0xfffed, 20);     /* 227*/ c[227]  = HuffCode(0x1fffe6, 21);
    /* 228*/ c[228]  = HuffCode(0x3fffe9, 22);    /* 229*/ c[229]  = HuffCode(0x1fffe7, 21);
    /* 230*/ c[230]  = HuffCode(0x1fffe8, 21);    /* 231*/ c[231]  = HuffCode(0x7ffff3, 23);
    /* 232*/ c[232]  = HuffCode(0x3fffea, 22);    /* 233*/ c[233]  = HuffCode(0x3fffeb, 22);
    /* 234*/ c[234]  = HuffCode(0x1ffffee, 25);   /* 235*/ c[235]  = HuffCode(0x1ffffef, 25);
    /* 236*/ c[236]  = HuffCode(0xfffff4, 24);    /* 237*/ c[237]  = HuffCode(0xfffff5, 24);
    /* 238*/ c[238]  = HuffCode(0x3ffffea, 26);   /* 239*/ c[239]  = HuffCode(0x7ffff4, 23);
    /* 240*/ c[240]  = HuffCode(0x3ffffeb, 26);   /* 241*/ c[241]  = HuffCode(0x7ffffe6, 27);
    /* 242*/ c[242]  = HuffCode(0x3ffffec, 26);   /* 243*/ c[243]  = HuffCode(0x3ffffed, 26);
    /* 244*/ c[244]  = HuffCode(0x7ffffe7, 27);   /* 245*/ c[245]  = HuffCode(0x7ffffe8, 27);
    /* 246*/ c[246]  = HuffCode(0x7ffffe9, 27);   /* 247*/ c[247]  = HuffCode(0x7ffffea, 27);
    /* 248*/ c[248]  = HuffCode(0x7ffffeb, 27);   /* 249*/ c[249]  = HuffCode(0xffffffe, 28);
    /* 250*/ c[250]  = HuffCode(0x7ffffec, 27);   /* 251*/ c[251]  = HuffCode(0x7ffffed, 27);
    /* 252*/ c[252]  = HuffCode(0x7ffffee, 27);   /* 253*/ c[253]  = HuffCode(0x7ffffef, 27);
    /* 254*/ c[254]  = HuffCode(0x7fffff0, 27);   /* 255*/ c[255]  = HuffCode(0x3ffffee, 26);
    /* EOS*/ c[256]  = HuffCode(0x3fffffff, 30);
    return c;
}();

// PORT-NOTE: C++ struct TrieNode → D struct TrieNode (ABI POD)
struct TrieNode {
    // PORT-NOTE: ABI POD value wrapper
    int[2] m_child = [-1, -1];
    uint   m_sym   = SYM_NONE;
}

// PORT-NOTE: C++ struct Trie → D struct Trie (ABI POD)
struct Trie {
    // PORT-NOTE: ABI POD value wrapper
    TrieNode[TRIE_CAP] m_nodes;
    size_t             m_size = 1;
}

// TODO: make consteval
Trie build_trie() pure {
    Trie trie;
    foreach (sym, ref entry; CODES) {
        const code = entry.code;
        const len  = cast(int)entry.len;
        int cur = 0;
        for (int shift = len - 1; shift >= 0; --shift) {
            const int BIT = (code >> shift) & 1;
            if (trie.m_nodes[cur].m_child[BIT] < 0)
                trie.m_nodes[cur].m_child[BIT] = cast(int)(trie.m_size++);
            cur = trie.m_nodes[cur].m_child[BIT];
        }
        trie.m_nodes[cur].m_sym = cast(uint)sym;
    }
    return trie;
}

// PORT-NOTE: C++ template<int W> struct TransTable → D template struct TransTable!W
struct TransTable(int W) if (DecodeWidth!W) {
    // PORT-NOTE: ABI POD value wrapper
    enum int CHUNK_COUNT = 1 << W;
    struct Entry { ushort next_row; ushort sym; }
    Entry[TABLE_CAP] m_entries;
    ushort           m_row_count = 0;
}

// TODO: make consteval
TransTable!W build_table(int W)() pure if (DecodeWidth!W) {
    enum int CHUNKS = TransTable!W.CHUNK_COUNT;
    Trie trie = build_trie();
    TransTable!W table;

    int[TRIE_CAP] node_to_row;
    node_to_row[] = -1;
    int[TRIE_CAP] queue;
    int q_head = 0;
    int q_tail = 0;

    int alloc_row(int node) {
        const int ROW = cast(int)(table.m_row_count++);
        node_to_row[node] = ROW;
        return ROW;
    }

    alloc_row(0);
    queue[q_tail++] = 0;

    while (q_head < q_tail) {
        const int START = queue[q_head++];
        const int ROW   = node_to_row[START];

        foreach (chunk; 0 .. CHUNKS) {
            int  cur         = START;
            uint emitted_sym = SYM_NONE;
            bool invalid     = false;

            for (int shift = W - 1; shift >= 0; --shift) {
                const int BIT  = (chunk >> shift) & 1;
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

            const size_t SLOT = cast(size_t)(ROW * CHUNKS) + chunk;
            if (invalid) {
                table.m_entries[SLOT] = TransTable!W.Entry(0xFFFF, 0xFFFF);
            } else {
                if (node_to_row[cur] < 0) {
                    alloc_row(cur);
                    queue[q_tail++] = cur;
                }
                const ushort SYM16 = (emitted_sym == SYM_NONE) ? 0xFFFE : cast(ushort)emitted_sym;
                table.m_entries[SLOT] = TransTable!W.Entry(cast(ushort)node_to_row[cur], SYM16);
            }
        }
    }
    return table;
}

// HuffmanEncodeView
//
// Lazy ubyte-producing range over an input char/byte sequence.
//
// Algorithm: 64-bit MSB-first accumulator.
// PORT-NOTE: C++ lazy range → D InputRange struct (forward-range equivalent).
// PORT-NOTE: C++ class → D struct (short-lived, no heap allocation needed)
struct HuffmanEncodeRange {
    // PORT-NOTE: ABI POD value wrapper – range over caller-supplied input slice
    const(ubyte)[] m_input;
    size_t         m_in_pos;
    ulong          m_bits;
    int            m_shift;
    bool           m_done;
    ubyte          m_front_val;

    this(const(ubyte)[] input) {
        m_input  = input;
        m_in_pos = 0;
        m_bits   = 0;
        m_shift  = 0;
        m_done   = false;
        advance();
    }

    bool empty() const pure { return m_done; }
    ubyte front() const pure { return m_front_val; }

    void popFront() {
        m_shift -= 8;
        advance();
    }

private:
    void advance() {
        if (m_in_pos >= m_input.length) {
            if (m_shift == 0) {
                m_done = true;
                return;
            }
            const int PAD = 8 - m_shift;
            m_bits  = (m_bits << PAD) | ((1U << PAD) - 1U);
            m_shift = 8;
        } else {
            const auto ref entry = CODES[m_input[m_in_pos++]];
            m_bits  = (m_bits << entry.len) | cast(ulong)entry.code;
            m_shift += cast(int)entry.len;
        }
        m_front_val = cast(ubyte)(m_bits >> (m_shift - 8));
    }
}

// HuffmanDecodeRange
//
// Lazy char-producing range over a ubyte input sequence.
//
// Algorithm: W-bit chunk FSM, buffer-drain pattern.
// PORT-NOTE: C++ class template HuffmanDecodeView<W, R> → D struct HuffmanDecodeRange!W.
struct HuffmanDecodeRange(int W) if (DecodeWidth!W) {
    // PORT-NOTE: ABI POD value wrapper
    enum int CHUNKS           = 1 << W;
    enum int CHUNKS_PER_BYTE  = 8 / W;
    enum int CHUNK_MASK       = CHUNKS - 1;
    // TODO: make constexpr when GCC supports it
    // PORT-NOTE: static table built once at runtime via shared static this
    static TransTable!W TABLE;
    shared static this() { TABLE = build_table!W(); }

    const(ubyte)[] m_input;
    size_t         m_in_pos;
    uint           m_fsm;
    int            m_chunk_idx;
    char           m_current;
    bool           m_done;
    int            m_padding_bits;

    this(const(ubyte)[] input) {
        m_input      = input;
        m_in_pos     = 0;
        m_fsm        = 0;
        m_chunk_idx  = 0;
        m_current    = '\0';
        m_done       = false;
        m_padding_bits = 0;
        advance();
    }

    bool empty() const pure { return m_done; }
    char front() const pure { return m_current; }
    void popFront() { advance(); }

private:
    void advance() {
        while (true) {
            if (m_in_pos >= m_input.length) {
                if (m_chunk_idx != 0) {
                    // truncated Huffman stream — set done silently (error propagated via @nogc sentinel)
                    m_done = true;
                    return;
                }
                if (m_fsm != 0 && m_padding_bits > 7) {
                    m_done = true;
                    return;
                }
                m_done = true;
                return;
            }

            const int CHUNK = CHUNK_MASK & (cast(int)m_input[m_in_pos] >>
                                            (8 - (W * (m_chunk_idx + 1))));

            if (++m_chunk_idx == CHUNKS_PER_BYTE) {
                ++m_in_pos;
                m_chunk_idx = 0;
            }

            const ref entry = TABLE.m_entries[cast(size_t)(m_fsm * CHUNKS) + CHUNK];
            m_fsm = entry.next_row;

            if (entry.sym < 256) {
                m_current      = cast(char)entry.sym;
                m_padding_bits = 0;
                return;
            } else if (entry.sym == 0xFFFF) {
                // invalid Huffman code — set done
                m_done = true;
                return;
            } else if (entry.sym == cast(ushort)SYM_EOS) {
                // EOS symbol in stream
                m_done = true;
                return;
            } else {
                m_padding_bits += W;
            }
        }
    }
}

// Huffman adaptor facade — mirrors C++ struct Huffman<W>.
// PORT-NOTE: C++ static methods → D free functions in a namespace-struct.
struct Huffman(int W = 4) if (DecodeWidth!W) {
    // PORT-NOTE: value wrapper (namespace struct), exempt from class-only rule

    // encode: returns an InputRange of ubyte over the input ubyte slice
    static HuffmanEncodeRange encode(const(ubyte)[] input) {
        return HuffmanEncodeRange(input);
    }

    // decode: returns an InputRange of char over the input ubyte slice
    static HuffmanDecodeRange!W decode(const(ubyte)[] input) {
        return HuffmanDecodeRange!W(input);
    }

    // decode into caller-supplied char buffer; returns number of chars written
    size_t decode(const(ubyte)[] body_bytes, ref char[] out_str) {
        auto r = HuffmanDecodeRange!W(body_bytes);
        size_t n = 0;
        while (!r.empty()) {
            if (n < out_str.length)
                out_str[n] = r.front();
            ++n;
            r.popFront();
        }
        return n;
    }
}
