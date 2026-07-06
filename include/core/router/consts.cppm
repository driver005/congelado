export module core_router:consts;

import std;

export namespace core::router {

inline constexpr std::uint16_t NO_CHILDREN = 0xFFFF;
inline constexpr std::size_t HANDLER_MASK = 0xFFFFFFFFFFFFFFFF;

} // namespace core::router
