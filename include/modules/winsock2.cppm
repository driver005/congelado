module;
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
export module modules.winsock2;

export {
    // Core types
    using ::addrinfo;
    using ::BOOL;
    using ::DWORD;
    using ::in_addr;
    using ::ip_mreq;
    using ::sockaddr;
    using ::sockaddr_in;
    using ::sockaddr_storage;
    using ::SOCKET;
    using ::u_long;
    using ::WSADATA;

    // Lifecycle
    using ::WSACleanup;
    using ::WSAGetLastError;
    using ::WSAStartup;

    // Socket API
    using ::accept;
    using ::bind;
    using ::closesocket;
    using ::connect;
    using ::getsockopt;
    using ::ioctlsocket;
    using ::listen;
    using ::recv;
    using ::recvfrom;
    using ::send;
    using ::sendto;
    using ::setsockopt;
    using ::shutdown;
    using ::socket;

    // Address conversion
    using ::freeaddrinfo;
    using ::getaddrinfo;
    using ::InetNtopA;
    using ::InetPtonA;

    // Byte order
    using ::htonl;
    using ::htons;
    using ::ntohl;
    using ::ntohs;

    // ssize_t is not defined by Winsock; define a compatible alias
    using ssize_t = int;
}
