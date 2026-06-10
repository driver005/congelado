module modules.netdb;
@nogc nothrow:

// Re-export address resolution types and functions from POSIX headers,
// mirroring the C++ module that wraps <netdb.h>.

public import core.sys.posix.netdb :
    addrinfo,
    freeaddrinfo,
    gai_strerror,
    getaddrinfo,
    getnameinfo;
