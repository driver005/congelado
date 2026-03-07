export module consts;

import std;

export inline constexpr std::size_t BLOCK_SIZE = 64;

export inline constexpr std::size_t REFS_MASK = ~static_cast<std::size_t>(0) >> 1;
export inline constexpr std::size_t SHOULD_BE_ON_LIST = ~REFS_MASK;
