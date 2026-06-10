module io.codec.shared.consts;
@nogc nothrow:

// io::shared_codec namespace constants

enum size_t PRIME = 31;

enum size_t ENTRY_OVERHEAD = 32;

enum size_t DEFAULT_MAX_TABLE_SIZE = 4096;

// On 64-bit SIZE_MAX = 0xFFFF'FFFF'FFFF'FFFF — same sentinel trick, two tag bits stolen from the top, index lives in
// the remaining 62 bits.
enum size_t SIZE_MAX = 0xFFFF_FFFF_FFFF_FFFF;
