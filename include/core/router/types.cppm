export module core_router:utils;

import std;

export namespace core::router {

enum class EdgeKind : std::uint8_t { PATH = 0, PARAM = 1, WILD = 2 };

constexpr auto split_path(std::string_view path) noexcept {
    return path | std::views::split('/') | std::views::transform([](auto &&rng) {
               return std::string_view(std::ranges::begin(rng), std::ranges::end(rng));
           }) |
           std::views::filter([](std::string_view segment) { return !segment.empty(); });
}

constexpr std::uint32_t fnv1a(std::string_view text) noexcept {
    // seed value the rolling hash starts from before any bytes get mixed in
    std::uint32_t hash = 5381U;
    // fold every byte of the string into the hash, one at a time — bet, this feeds the sibling
    // probe order in RouterNode::find_child(), so the mixing order here is load-bearing
    for (char chr : text) {
        hash = (hash * 33U) ^ static_cast<std::uint8_t>(chr);
    }
    return hash;
}

} // namespace core::router
