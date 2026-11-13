module;

#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

export module modules.net;

export {
    // address types
    using ::in_addr;
    using ::in_addr_t;
    using ::in_port_t;
    using ::ip_mreq;
    using ::sockaddr_in;

    // byte order
    using ::htonl;
    using ::htons;
    using ::ntohl;
    using ::ntohs;

    // address string conversion
    using ::inet_addr;
    using ::inet_ntoa;
    using ::inet_ntop;
    using ::inet_pton;
}
