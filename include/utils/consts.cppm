export module consts;

import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export inline constexpr std::size_t BLOCK_SIZE = 64;

export inline constexpr std::size_t REFS_MASK = ~static_cast<std::size_t>(0) >> 1;
export inline constexpr std::size_t SHOULD_BE_ON_LIST = ~REFS_MASK;

#ifdef CONGELADO_TEST
namespace {
using namespace boost::ut;

suite<"consts"> consts_suite = [] {
    "REFS_MASK and SHOULD_BE_ON_LIST partition every bit exactly once"_test = [] {
        expect((REFS_MASK & SHOULD_BE_ON_LIST) == 0);
        expect((REFS_MASK | SHOULD_BE_ON_LIST) == ~static_cast<std::size_t>(0));
    };
    "SHOULD_BE_ON_LIST is exactly the top bit"_test = [] {
        expect(SHOULD_BE_ON_LIST == (static_cast<std::size_t>(1) << (sizeof(std::size_t) * 8 - 1)));
    };
};

} // namespace
#endif
