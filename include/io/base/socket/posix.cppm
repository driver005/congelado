module;

#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>

export module io_base_socket:posix;

import std;
import io_error;
import core_logger;

export namespace io::base::socket {

using ioctl_setting = int;
using buffsize_t = size_t;

using SOCKET = int;
using SOCKADDR_IN = sockaddr_in;
using SOCKADDR = sockaddr;
using IN_ADDR = in_addr;

inline int closesocket(SOCKET in) { return close(in); }

template <typename... Params>
inline int ioctlsocket(int fd, int request, Params &&...params) {
    return ioctl(fd, request, params...);
}

using OsPayload = std::monostate;

inline int get_error_code() { return errno; }


inline void set_non_blocking_impl(SOCKET socket, bool non_blocking) {
    int flags = fcntl(socket, F_GETFL, 0);
    if (flags == -1) {
        core::logger::error("io/posix", "get flags failed");
    }

    if (non_blocking) {
        flags |= O_NONBLOCK;
    } else {
        flags &= ~O_NONBLOCK;
    }

    if (fcntl(socket, F_SETFL, flags) < 0) {
        core::logger::error("io/posix", "set non-blocking failed");
    }

    core::logger::debug("io/posix", "fd {} non-blocking={}", socket, non_blocking);
}

} // namespace io::base::socket
