export module core_router:utils;

import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

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

#ifdef CONGELADO_TEST
namespace core::router::tests {
using namespace boost::ut;

suite<"router_utils_split_path"> split_path_suite = [] {
    "splits a multi-segment path"_test = [] {
        std::vector<std::string_view> segments;
        for (auto segment : split_path("/users/42/posts")) {
            segments.emplace_back(segment);
        }

        expect(segments.size() == 3);
        expect(segments[0] == "users");
        expect(segments[1] == "42");
        expect(segments[2] == "posts");
    };

    "collapses consecutive slashes, ignoring empty segments"_test = [] {
        std::vector<std::string_view> segments;
        for (auto segment : split_path("//foo//bar/")) {
            segments.emplace_back(segment);
        }

        expect(segments.size() == 2);
        expect(segments[0] == "foo");
        expect(segments[1] == "bar");
    };

    "empty path yields no segments"_test = [] {
        std::vector<std::string_view> segments;
        for (auto segment : split_path("")) {
            segments.emplace_back(segment);
        }

        expect(segments.empty());
    };
};

suite<"router_utils_fnv1a"> fnv1a_suite = [] {
    "empty string hashes to the seed value"_test = [] {
        expect(fnv1a("") == 5381U);
    };

    "known strings hash to their expected fnv1a-variant value"_test = [] {
        expect(fnv1a("a") == 177604U);
        expect(fnv1a("users") == 183638951U);
        expect(fnv1a(":id") == 193366962U);
    };

    "hashing is deterministic"_test = [] {
        expect(fnv1a("same-input") == fnv1a("same-input"));
    };
};

} // namespace core::router::tests
#endif
