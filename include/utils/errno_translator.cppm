module;

#include <cerrno>

export module utils_errno_translator;

import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace utils {

class ErrnoTranslator {
  public:
    /**
     * @brief Maps a raw POSIX errno-style code to a short, human-readable explanation of what
     * actually happened — meant to be dropped straight into a log line so a bare numeric code
     * (e.g. `send error: 32`) doesn't force a manual `errno.h` lookup to understand.
     * @note Only covers codes this codebase's socket/IO layer actually surfaces (see
     * `io::base::socket::Socket::get_error_code()` and its posix.cppm/win32.cppm backends) —
     * not the full `errno.h` table. On Windows, only `EWOULDBLOCK`/`EBADF`/`EINTR` are
     * posixified by win32.cppm's `get_error_code()`; any other raw `WSAE*` code lands in the
     * `default` case here.
     * @param code the errno value to describe. `0` is handled explicitly since
     * `SSL_ERROR_SYSCALL` can hand it back too (see socket.cppm's `map_receive_ssl_error()`),
     * where it doesn't mean "no error" the way a bare `errno == 0` normally would.
     * @return a static string view describing what `code` means, with enough context to explain
     * the failure without needing the surrounding log lines; `"Unknown error"` for anything not
     * explicitly handled below.
     */
    [[nodiscard]] static std::string_view describe_errno(int code) noexcept {
        switch (code) {
        case 0:
            return "No errno set — for a plain syscall failure this would mean success, but for "
                   "SSL_ERROR_SYSCALL specifically it means the peer closed the TCP connection "
                   "without a clean TLS shutdown (no close_notify), so libssl has nothing more "
                   "specific to report";
        case EPIPE:
            return "Broken pipe — the peer already closed its end of the connection, so this "
                   "write landed on a dead socket";
        case ECONNRESET:
            return "Connection reset by peer — the remote side tore the connection down abruptly "
                   "(TCP RST), typically after a crash, timeout, or forceful close on their end";
        case ECONNREFUSED:
            return "Connection refused — nothing was listening on the target address/port, or a "
                   "firewall actively rejected the connection attempt";
        case ETIMEDOUT:
            return "Connection timed out — the peer never responded within the allotted time";
        case ENOTCONN:
            return "Socket is not connected — an operation that requires an established "
                   "connection was attempted on a socket that isn't connected";
        case EAGAIN:
#if EWOULDBLOCK != EAGAIN
        case EWOULDBLOCK:
#endif
            return "Resource temporarily unavailable — the operation would have blocked on a "
                   "non-blocking socket; not a real failure, just come back later";
        case EBADF:
            return "Bad file descriptor — the socket handle is invalid, already closed, or not a "
                   "socket at all";
        case EINTR:
            return "Interrupted system call — a signal arrived and interrupted the operation "
                   "before it could complete";
        case EINVAL:
            return "Invalid argument — the socket or one of the call's parameters is in a state "
                   "that doesn't support this operation";
        case ENOMEM:
            return "Out of memory — the kernel couldn't allocate the resources this operation "
                   "needed";
        case EACCES:
            return "Permission denied — insufficient privileges for this socket operation "
                   "(e.g. binding a privileged port without the right capability)";
        case EMFILE:
            return "Too many open files — this process has hit its own file descriptor limit";
        case ENFILE:
            return "Too many open files in system — the system-wide file descriptor limit has "
                   "been reached";
        case EADDRINUSE:
            return "Address already in use — another socket is already bound to this address "
                   "and port";
        case EADDRNOTAVAIL:
            return "Address not available — the requested local address isn't assigned to any "
                   "interface on this machine";
        case ENETUNREACH:
            return "Network unreachable — no route exists to the destination network";
        case EHOSTUNREACH:
            return "Host unreachable — no route exists to the destination host";
        case ENOBUFS:
            return "No buffer space available — the kernel ran out of network buffer memory for "
                   "this operation";
        case EPROTOTYPE:
            return "Protocol wrong type for socket — the socket's type doesn't match the "
                   "protocol being requested on it";
        case EOPNOTSUPP:
            return "Operation not supported — this operation isn't supported on this socket type";
        case ENOSPC:
            return "No space left on device";
        case EIO:
            return "I/O error — a low-level hardware or driver failure occurred";
        default:
            return "Unknown error";
        }
    }
};

} // namespace utils

#ifdef CONGELADO_TEST
namespace utils::tests {
using namespace boost::ut;

suite<"ErrnoTranslator"> errno_translator_suite = [] {
    "known codes map to their specific description"_test = [] {
        expect(ErrnoTranslator::describe_errno(EPIPE).starts_with("Broken pipe"));
        expect(ErrnoTranslator::describe_errno(ECONNRESET).starts_with("Connection reset"));
        expect(ErrnoTranslator::describe_errno(ETIMEDOUT).starts_with("Connection timed out"));
    };
    "0 is treated as the SSL_ERROR_SYSCALL special case, not success"_test = [] {
        expect(ErrnoTranslator::describe_errno(0).starts_with("No errno set"));
    };
    "unmapped codes fall back to Unknown error"_test = [] {
        expect(ErrnoTranslator::describe_errno(-999999) == "Unknown error");
    };
};

} // namespace utils::tests
#endif
