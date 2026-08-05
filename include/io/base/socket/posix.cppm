module;

#include <cerrno>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>

export module io_base_socket:posix;

import std;
import io_error;
import core_events;
import core_logger;

export namespace io::base::socket {

using ioctl_setting = int;
using buffsize_t = size_t;

using SOCKET = int;
using SOCKADDR_IN = sockaddr_in;
using SOCKADDR = sockaddr;
using IN_ADDR = in_addr;

inline int closesocket(SOCKET socket_descriptor) { return close(socket_descriptor); }

template <typename... Params>
inline int ioctlsocket(int socket_descriptor, int request, Params &&...params) {
    return ioctl(socket_descriptor, request, std::forward<Params>(params)...);
}

using OsPayload = std::monostate;

inline int get_error_code() { return errno; }


inline void set_non_blocking_impl(SOCKET socket, bool non_blocking) {
    // grab the current flag word first, bet — can't toggle O_NONBLOCK without it. -1 means the
    // fcntl itself failed, logged and fallen through rather than bailing, since flags still
    // holds something usable (garbage, but fcntl won't touch it further below without a value)
    int flags = fcntl(socket, F_GETFL, 0);
    if (flags == -1) {
        core::logger::error("io/posix", "get flags failed");
        core::events::publish("io.posix.get_flags_failed", {{"fd", std::to_string(socket)}});
    }

    // set or clear the non-blocking bit depending on what the caller asked for
    if (non_blocking) {
        flags |= O_NONBLOCK;
    } else {
        flags &= ~O_NONBLOCK;
    }

    // write the modified flags back — this is the call that actually takes effect
    if (fcntl(socket, F_SETFL, flags) < 0) {  // NOLINT(cppcoreguidelines-pro-type-vararg) — fcntl is a POSIX vararg API, no safe C++ alternative
        core::logger::error("io/posix", "set non-blocking failed");
        core::events::publish("io.posix.set_non_blocking_failed", {{"fd", std::to_string(socket)}});
    }

    core::logger::debug("io/posix", "fd {} non-blocking={}", socket, non_blocking);
}

} // namespace io::base::socket
