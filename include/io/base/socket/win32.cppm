module;

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <cerrno>
#include <winsock2.h>
#include <ws2tcpip.h>

export module io_base_socket:win32;

import std;
import io_error;
import :consts;

export namespace io::base::socket {

using ioctl_setting = u_long;
using buffsize_t = int;

// Handle WinSock2/Windows Socket API initialization and cleanup
#pragma comment(lib, "Ws2_32.lib")

// taken from:
// https://github.com/rxi/dyad/blob/915ae4939529b9aaaf6ebfd2f65c6cff45fc0eac/src/dyad.c#L58
inline const char* inet_ntop(int af, const void* src, char* dst, socklen_t size)
{
    union
    {
        struct sockaddr sa;
        struct sockaddr_in sai;
        struct sockaddr_in6 sai6;
    } addr;

    int res;
    // zero the whole union first so whichever member we don't touch below stays clean
    memset(&addr, 0, sizeof(addr));
    addr.sa.sa_family = (unsigned short)af;
    // copy the raw address bytes into the right union member depending on v4 vs v6
    if (af == AF_INET6) {
        memcpy(&addr.sai6.sin6_addr, src, sizeof(addr.sai6.sin6_addr));
    } else {
        memcpy(&addr.sai.sin_addr, src, sizeof(addr.sai.sin_addr));
    }
    // let WinSock cook — it does the actual address-to-string formatting, no native inet_ntop
    // here
    res = WSAAddressToStringA(&addr.sa, sizeof(addr), 0, dst, reinterpret_cast<LPDWORD>(&size));
    if (res != 0) {
        return NULL;
    }
    return dst;
}

namespace win32_specific {
    /// Forward declare the object that will permit to manage the WSAStartup/Cleanup
    /// automatically
    struct WSA;

    /// Enclose the global pointer in this namespace. Only use this inside a shared_ptr
    namespace internal_state {
        inline WSA* global_WSA = nullptr;
    }

    /// WSA object. Only to be constructed with std::make_shared()
    struct WSA : std::enable_shared_from_this<WSA>
    {
        // For safety, only initialize Windows Socket API once, and delete it once
        /// Prevent copy construct
        WSA(const WSA&) = delete;
        /// Prevent copy assignment
        WSA& operator=(const WSA&) = delete;
        /// Prevent moving
        WSA(WSA&&) = delete;
        /// Prevent move assignment
        WSA& operator=(WSA&&) = delete;

        /// data storage
        WSADATA wsa_data;

        /// Startup
        WSA() :
            wsa_data{}
        {
            // fire off WSAStartup once — a non-zero status means the whole Windows Socket API
            // is unusable, so translate the code into something readable before panicking
            if (const auto status = WSAStartup(MAKEWORD(2, 2), &wsa_data); status != 0) {
                std::string error_message;
                switch (
                    status) // https://docs.microsoft.com/en-us/windows/win32/api/winsock/nf-winsock-wsastartup#return-value
                {
                    default:
                        error_message = "Unknown error happened.";
                        break;
                    case WSASYSNOTREADY:
                        error_message = "The underlying network subsystem is not ready for "
                                        "network communication.";
                        break;
                    case WSAVERNOTSUPPORTED: // unlikely, we specify 2.2!
                        error_message = " The version of Windows Sockets support requested "
                                        "(2.2)" // we know here the version was 2.2, add that to
                                                // the error message copied from MSDN
                                        " is not provided by this particular Windows Sockets "
                                        "implementation. ";
                        break;
                    case WSAEINPROGRESS:
                        error_message = "A blocking Windows Sockets 1.1 operation is in progress.";
                        break;
                    case WSAEPROCLIM:
                        error_message = "A limit on the number of tasks supported by the "
                                        "Windows Sockets implementation has been reached.";
                        break;
                    case WSAEFAULT: // unlikely, if this ctor is running, wsa_data is part of
                                    // this object's "stack" data
                        error_message = "The lpWSAData parameter is not a valid pointer.";
                        break;
                }

                error::handle_error(error_message);
            }
            // debug-only trace, compiled away entirely when DEBUG is false
            if constexpr (DEBUG) {
                std::cerr << "Initialized Windows Socket API\n";
            }
        }

        /// Cleanup
        ~WSA()
        {
            // release WinSock and clear the global raw pointer so getWSA() knows to recreate it
            WSACleanup();
            internal_state::global_WSA = nullptr;
            if constexpr (DEBUG) {
                std::cerr << "Cleanup Windows Socket API\n";
            }
        }

        /// get the shared pointer
        std::shared_ptr<WSA> getPtr()
        {
            return shared_from_this();
        }
    };

    /// Get-or-create the global pointer
    inline std::shared_ptr<WSA> getWSA()
    {
        // If it has been created already:
        if (internal_state::global_WSA) {
            return internal_state::global_WSA
                ->getPtr(); // fetch the smart pointer from the naked pointer
        }

        // Create in wsa
        auto wsa = std::make_shared<WSA>();

        // Save the raw address in the global state
        internal_state::global_WSA = wsa.get();

        // Return the smart pointer
        return wsa;
    }
} // namespace win32_specific

class OsPayload
{
    std::shared_ptr<win32_specific::WSA> wsa_ptr;

public:
    /**
     * @brief Grabs (or lazily creates) the process-wide WSA singleton via `getWSA()` — bet, the
     * whole point of this class is being a `[[no_unique_address]]` member that keeps WinSock
     * alive for as long as at least one socket referencing it is around. On posix this type is
     * just an empty `std::monostate`, so this ctor only exists on win32.
     */
    OsPayload() :
        wsa_ptr{win32_specific::getWSA()}
    {
    }

    /** @brief Move ctor — defaulted, just moves the shared_ptr ref to the WSA singleton. */
    OsPayload(OsPayload&&) noexcept = default;
    /** @brief Move assignment — defaulted, mirrors the move ctor. */
    OsPayload& operator=(OsPayload&&) noexcept = default;

    /** @brief Deleted — copying would be harmless (it's just a shared_ptr bump) but there's
     * lowkey no use case for it here, sockets move, they don't get duplicated payloads. */
    OsPayload(const OsPayload&) = delete;
    /** @brief Deleted — mirrors the copy ctor. */
    OsPayload& operator=(const OsPayload&) = delete;
};

/// Return the last error code
inline int get_error_code()
{
    const auto error = WSAGetLastError();

    // We need to posixify the values that we are actually using inside this header.
    switch (error) {
        case WSAEWOULDBLOCK:
            return EWOULDBLOCK;
        case WSAEBADF:
            return EBADF;
        case WSAEINTR:
            return EINTR;
        default:
            return error;
    }
}

inline void set_non_blocking_impl(SOCKET socket, bool non_blocking)
{
    // FIONBIO takes a u_long in/out mode flag rather than a bitmask like posix's O_NONBLOCK
    u_long mode = non_blocking ? 1 : 0;
    if (ioctlsocket(socket, FIONBIO, &mode) < 0) {
        error::handle_error("Failed to set socket non-blocking");
    }
}

} // namespace io::base::socket
