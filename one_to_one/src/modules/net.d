module modules.net;
@nogc nothrow:

// Re-export address types and byte-order/conversion functions from POSIX headers,
// mirroring the C++ module that wraps <arpa/inet.h>, <netinet/in.h>, <netinet/tcp.h>.

// address types
public import core.sys.posix.netinet.in_ :
    in_addr,
    in_addr_t,
    in_port_t,
    ip_mreq,
    sockaddr_in;

// byte order
public import core.sys.posix.arpa.inet :
    htonl,
    htons,
    ntohl,
    ntohs,
    // address string conversion
    inet_addr,
    inet_ntoa,
    inet_ntop,
    inet_pton;
