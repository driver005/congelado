export module core_server:consts;

import std;

export namespace core::server {

inline constexpr std::uint16_t NO_CHILDREN = 0xFFFF;
inline constexpr std::size_t HANDLER_MASK = 0xFFFFFFFFFFFFFFFF;

} // namespace core::server
