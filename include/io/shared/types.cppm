export module io_shared:types;

import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace io::shared {

enum class Role : std::uint8_t
{
    SENDER = 0,
    RECEIVER = 1
};

} // namespace io::shared

#ifdef CONGELADO_TEST
namespace io::shared::tests {
using namespace boost::ut;

suite<"Role"> role_suite = [] {
    "SENDER and RECEIVER have their expected underlying values"_test = [] {
        expect(std::to_underlying(Role::SENDER) == 0);
        expect(std::to_underlying(Role::RECEIVER) == 1);
    };

    "SENDER and RECEIVER are distinct"_test = [] {
        expect(Role::SENDER != Role::RECEIVER);
    };
};

} // namespace io::shared::tests
#endif
