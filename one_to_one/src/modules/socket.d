module modules.socket;
@nogc nothrow:

// Re-export socket types and functions from POSIX headers, mirroring the C++
// module that wraps <sys/socket.h> and <sys/types.h>.

public import core.sys.posix.sys.socket :
    sa_family_t,
    sockaddr,
    sockaddr_storage,
    socklen_t,
    accept,
    bind,
    connect,
    getpeername,
    getsockname,
    getsockopt,
    listen,
    recv,
    recvfrom,
    send,
    sendto,
    setsockopt,
    shutdown,
    socket;

public import core.sys.posix.sys.types : ssize_t;
