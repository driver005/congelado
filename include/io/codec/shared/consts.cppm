export module io_codec_shared:consts;

import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace io::shared_codec {

constexpr std::size_t PRIME = 31;

constexpr std::size_t ENTRY_OVERHEAD = 32;

constexpr std::size_t DEFAULT_MAX_TABLE_SIZE = 4096;

// On 64-bit SIZE_MAX = 0xFFFF'FFFF'FFFF'FFFF — same sentinel trick, two tag bits stolen from the top, index lives in
// the remaining 62 bits.
constexpr std::size_t SIZE_MAX = 0xFFFF'FFFF'FFFF'FFFF;

} // namespace codec::shared

#ifdef CONGELADO_TEST
namespace io::shared_codec::tests {
using namespace boost::ut;

suite<"shared_codec_consts"> shared_codec_consts_suite = [] {
    "SIZE_MAX is the all-bits-set sentinel for std::size_t"_test = [] {
        expect(SIZE_MAX == std::numeric_limits<std::size_t>::max());
    };
    "ENTRY_OVERHEAD and DEFAULT_MAX_TABLE_SIZE hold their documented values"_test = [] {
        expect(ENTRY_OVERHEAD == 32);
        expect(DEFAULT_MAX_TABLE_SIZE == 4096);
    };
};

} // namespace io::shared_codec::tests
#endif
