module;

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#endif

export module io_base_socket:consts;

import std;

export namespace transport::base::socket {

inline constexpr bool DEBUG = false;

#if defined(_WIN32)
constexpr bool is_windows = true;
inline constexpr int SHUT_RDWR = SD_BOTH;
#else
constexpr bool is_windows = false;
inline constexpr int INVALID_SOCKET = -1;
inline constexpr int SOCKET_ERROR = -1;
#endif

} // namespace transport::base::socket
