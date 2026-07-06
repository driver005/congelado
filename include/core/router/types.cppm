export module core_router:utils;

import std;

export namespace core::router {

enum class EdgeKind : std::uint8_t { Path = 0, Param = 1, Wild = 2 };

constexpr auto split_path(std::string_view path) noexcept {
    return path | std::views::split('/') | std::views::transform([](auto &&rng) {
               return std::string_view(std::ranges::begin(rng), std::ranges::end(rng));
           }) |
           std::views::filter([](std::string_view sv) { return !sv.empty(); });
}

constexpr std::uint32_t fnv1a(std::string_view s) noexcept {
    std::uint32_t hash = 5381U;
    for (char chr : s) {
        hash = (hash * 33U) ^ static_cast<std::uint8_t>(chr);
    }
    return hash;
}

} // namespace core::router
