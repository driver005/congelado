export module core_contract:consts;

import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::contract {

inline constexpr std::uint64_t BIAS_FLAG = 1ULL << 63;

}

#ifdef CONGELADO_TEST
namespace core::contract::tests {
using namespace boost::ut;

suite<"contract_consts"> contract_consts_suite = [] {
    "BIAS_FLAG is the sign bit of a 64-bit value, and only that bit"_test = [] {
        expect(BIAS_FLAG == 0x8000000000000000ULL);
        expect((BIAS_FLAG & (BIAS_FLAG - 1)) == 0);
    };
};

} // namespace core::contract::tests
#endif
