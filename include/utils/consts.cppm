module;

#include <cstddef>

export module constants;

export inline constexpr std::size_t BLOCK_SIZE = 64;

// For your AtomicList logic
export inline constexpr std::size_t SHOULD_BE_ON_LIST = 1ULL << 63;
export inline constexpr std::size_t REFS_MASK = SHOULD_BE_ON_LIST - 1;

// Networking / gRPC defaults
export inline constexpr int DEFAULT_PORT = 50051;
export inline constexpr auto DEFAULT_ADDRESS = "0.0.0.0";
