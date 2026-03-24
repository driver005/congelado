export module hpack:consts;

import std;

export namespace codec::hpack {

constexpr std::size_t ENTRY_OVERHEAD = 32;

constexpr std::size_t DEFAULT_MAX_TABLE_SIZE = 4096;

constexpr std::size_t STATIC_TABLE_SIZE = 61;
} // namespace codec::hpack
