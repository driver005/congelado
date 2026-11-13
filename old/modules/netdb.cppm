module;
#include <netdb.h>
export module modules.netdb;

export {
    using ::addrinfo;
    using ::freeaddrinfo;
    using ::gai_strerror;
    using ::getaddrinfo;
    using ::getnameinfo;
}
