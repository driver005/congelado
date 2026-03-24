export module shared:consts;

import std;

export namespace codec::shared {

constexpr std::size_t ENTRY_OVERHEAD = 32;

constexpr std::size_t DEFAULT_MAX_TABLE_SIZE = 4096;

// On 64-bit SIZE_MAX = 0xFFFF'FFFF'FFFF'FFFF — same sentinel trick, two tag bits stolen from the top, index lives in
// the remaining 62 bits.
constexpr std::size_t SIZE_MAX = 0xFFFF'FFFF'FFFF'FFFF;

} // namespace codec::shared
