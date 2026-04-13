export module io_server:consts;

import std;

export namespace transport::server {

inline constexpr std::uint16_t NO_CHILDREN = 0xFFFF;
inline constexpr std::size_t HANDLER_MASK = 0xFFFFFFFFFFFFFFFF;

} // namespace transport::server
