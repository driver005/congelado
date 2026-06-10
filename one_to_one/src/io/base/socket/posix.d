module io.base.socket.posix;
@nogc nothrow:

version (Posix):

import io.error.base : handle_error;
import core.sys.posix.sys.socket : sockaddr, sockaddr_in, in_addr;
import core.sys.posix.netinet.in_ : sockaddr_in6;
import core.sys.posix.unistd : close;
import core.sys.posix.fcntl : fcntl, F_GETFL, F_SETFL, O_NONBLOCK;
import core.sys.posix.sys.ioctl : ioctl;
import core.stdc.errno : errno;

// PORT-NOTE: ABI POD type aliases matching the C++ using declarations
alias ioctl_setting = int;
alias buffsize_t    = size_t;

alias SOCKET    = int;
alias SOCKADDR_IN = sockaddr_in;
alias SOCKADDR    = sockaddr;
alias IN_ADDR     = in_addr;

int closesocket(SOCKET sock) {
    return close(sock);
}

int ioctlsocket(int fd, int request, ...) {
    // PORT-NOTE: variadic forwarding to ioctl; callers pass a single int* or u_long*
    // D variadics are not @nogc — callers should call ioctl directly for @nogc paths.
    // This shim is preserved for compatibility with socket.d call sites.
    return 0;  // PORT-NOTE: stub; see TODO in socket.d
}

// PORT-NOTE: OsPayload = std::monostate on POSIX → empty struct
struct OsPayload {}

int get_error_code() { return errno; }

// PORT-NOTE: core.logger calls use the shared logger interface
// Import the shared logger module
import shared_.logger : error, debug_ = debug;

void set_non_blocking_impl(SOCKET sock, bool non_blocking) {
    int flags = fcntl(sock, F_GETFL, 0);
    if (flags == -1) {
        error("io/posix", "get flags failed");
    }

    if (non_blocking) {
        flags |= O_NONBLOCK;
    } else {
        flags &= ~O_NONBLOCK;
    }

    if (fcntl(sock, F_SETFL, flags) < 0) {
        error("io/posix", "set non-blocking failed");
    }

    // core::logger::debug("io/posix", "fd {} non-blocking={}", socket, non_blocking);
    debug_("io/posix", "set non-blocking");
}
