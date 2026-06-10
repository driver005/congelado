module modules.winsock2;
@nogc nothrow:

// Re-export Winsock2 types and functions for Windows builds, mirroring the C++
// module that wraps <winsock2.h> and <ws2tcpip.h>.
// On non-Windows platforms this module is empty (POSIX sockets are used instead).

version (Windows) {

    public import core.sys.windows.winsock2 :
        // Core types
        addrinfo,
        BOOL,
        DWORD,
        in_addr,
        ip_mreq,
        sockaddr,
        sockaddr_in,
        sockaddr_storage,
        SOCKET,
        u_long,
        WSADATA,

        // Lifecycle
        WSACleanup,
        WSAGetLastError,
        WSAStartup,

        // Socket API
        accept,
        bind,
        closesocket,
        connect,
        getsockopt,
        ioctlsocket,
        listen,
        recv,
        recvfrom,
        send,
        sendto,
        setsockopt,
        shutdown,
        socket,

        // Address conversion
        freeaddrinfo,
        getaddrinfo,
        InetNtopA,
        InetPtonA,

        // Byte order
        htonl,
        htons,
        ntohl,
        ntohs;

    // #pragma comment(lib, "Ws2_32.lib") — link via dub.sdl instead
    // ssize_t is not defined by Winsock; define a compatible alias
    // using ssize_t = int;  // kept as comment, define at use-site if needed

} // version (Windows)
