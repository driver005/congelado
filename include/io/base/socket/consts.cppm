module;

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#endif

export module io_base_socket:consts;

import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace io::base::socket {

inline constexpr bool DEBUG = false;

#ifdef _WIN32
constexpr bool IS_WINDOWS = true;
inline constexpr int SHUT_RDWR = SD_BOTH;
#else
constexpr bool IS_WINDOWS = false;
inline constexpr int INVALID_SOCKET = -1;
inline constexpr int SOCKET_ERROR = -1;
#endif

} // namespace io::base::socket

#ifdef CONGELADO_TEST
namespace io::base::socket::tests {
using namespace boost::ut;

suite<"socket_consts"> socket_consts_suite = [] {
    "DEBUG is off by default"_test = [] { expect(not DEBUG); };

    "IS_WINDOWS matches the platform this was compiled for"_test = [] {
#ifdef _WIN32
        expect(IS_WINDOWS);
#else
        expect(not IS_WINDOWS);
#endif
    };

#ifndef _WIN32
    "INVALID_SOCKET and SOCKET_ERROR are both -1 on posix"_test = [] {
        expect(INVALID_SOCKET == -1);
        expect(SOCKET_ERROR == -1);
    };
#endif
};

} // namespace io::base::socket::tests
#endif
