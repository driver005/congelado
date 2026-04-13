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

export namespace transport::base::socket {

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
        error::handle_error("Failed to get socket flags");
    }

    if (non_blocking) {
        flags |= O_NONBLOCK;
    } else {
        flags &= ~O_NONBLOCK;
    }

    if (fcntl(socket, F_SETFL, flags) < 0) {
        error::handle_error("Failed to set socket non-blocking");
    }
}

} // namespace transport::base::socket
