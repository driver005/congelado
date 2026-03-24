module;

#include <sys/socket.h>
#include <sys/types.h>

export module modules.socket;

export {
    using ::sa_family_t;
    using ::sockaddr;
    using ::sockaddr_storage;
    using ::socklen_t;
    using ::ssize_t;

    using ::accept;
    using ::bind;
    using ::connect;
    using ::getpeername;
    using ::getsockname;
    using ::getsockopt;
    using ::listen;
    using ::recv;
    using ::recvfrom;
    using ::send;
    using ::sendto;
    using ::setsockopt;
    using ::shutdown;
    using ::socket;
}
