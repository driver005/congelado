export module core_router:consts;

import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::router {

inline constexpr std::uint16_t NO_CHILDREN = 0xFF'FF;
inline constexpr std::size_t HANDLER_MASK = 0xFF'FF'FF'FF'FF'FF'FF'FF;

} // namespace core::router

#ifdef CONGELADO_TEST
namespace core::router::tests {
using namespace boost::ut;

suite<"router_consts"> router_consts_suite = [] {
    "NO_CHILDREN and HANDLER_MASK are all-bits-set sentinels for their width"_test = [] {
        expect(NO_CHILDREN == 0xFF'FF);
        expect(HANDLER_MASK == 0xFF'FF'FF'FF'FF'FF'FF'FFULL);
    };
};

} // namespace core::router::tests
#endif
