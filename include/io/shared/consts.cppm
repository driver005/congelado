export module io_shared:consts;

import std;

export namespace io::shared {

inline constexpr std::size_t ENTRY_OVERHEAD = 32;

inline constexpr std::string COOKIE_SEPARATOR = "; ";

inline constexpr std::string VALUE_SEPARATOR = ", ";

} // namespace io::shared
