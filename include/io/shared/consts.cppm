export module io_shared:consts;

import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace io::shared {

inline constexpr std::size_t ENTRY_OVERHEAD = 32;

inline constexpr std::string COOKIE_SEPARATOR = "; ";

inline constexpr std::string VALUE_SEPARATOR = ", ";

} // namespace io::shared

#ifdef CONGELADO_TEST
namespace io::shared::tests {
using namespace boost::ut;

suite<"shared_consts"> shared_consts_suite = [] {
    "ENTRY_OVERHEAD is 32 bytes"_test = [] {
        expect(ENTRY_OVERHEAD == 32);
    };

    "COOKIE_SEPARATOR is a semicolon-space"_test = [] {
        expect(COOKIE_SEPARATOR == "; ");
    };

    "VALUE_SEPARATOR is a comma-space"_test = [] {
        expect(VALUE_SEPARATOR == ", ");
    };

    "COOKIE_SEPARATOR and VALUE_SEPARATOR are distinct"_test = [] {
        expect(COOKIE_SEPARATOR != VALUE_SEPARATOR);
    };
};

} // namespace io::shared::tests
#endif
