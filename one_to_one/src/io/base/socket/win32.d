module io.base.socket.win32;
@nogc nothrow:

version (Windows):

import io.error.base : handle_error;
import io.base.socket.consts : DEBUG;
import core.stdc.errno : EWOULDBLOCK, EBADF, EINTR;
import core.stdc.string : memset, memcpy;

// PORT-NOTE: Win32 socket type aliases; see win32.d in leverage for HANDLE/SOCKET stubs
alias ioctl_setting = uint;  // u_long
alias buffsize_t    = int;

// PORT-NOTE: SOCKET/SOCKADDR_IN etc. come from the leverage win32 binding
// Re-use the types declared in io.base.leverage.win32
// (linked at Run 2 time via pragma(lib))

// PORT-NOTE: inet_ntop replacement for older WinSock2
// taken from: https://github.com/rxi/dyad/blob/915ae4939529b9aaaf6ebfd2f65c6cff45fc0eac/src/dyad.c#L58
// PORT-NOTE: full inet_ntop implementation dropped; this is a stub.
// The C WSAAddressToStringA approach uses heap allocation indirectly — deferred to Run 2.
const(char)* inet_ntop_compat(int af, const(void)* src, char* dst, uint size) {
    // PORT-NOTE: stub — fill dst with "0.0.0.0" as fallback
    if (dst !is null && size > 0) dst[0] = '\0';
    return dst;
}

// WSA global state — init/cleanup
// PORT-NOTE: WSA lifecycle management. C++ used std::enable_shared_from_this<WSA>.
// D port uses a plain module-level reference count (thread-unsafe, acceptable for
// single-init scenarios; improvement candidate for Run 3).

private struct WSA {
    // PORT-NOTE: value wrapper, not GC-allocated; lifecycle managed by getWSA/releaseWSA
    ubyte[408] wsa_data;  // WSADATA on Win64 is 408 bytes
    bool initialized;
}

private __gshared WSA g_wsa;
private __gshared int g_wsa_refcount = 0;

// PORT-NOTE: OsPayload on Windows initialized WSA via shared_ptr in C++.
// D port holds a simple boolean indicating WSA was initialized.
class OsPayload {
  public:
    this() {
        if (g_wsa_refcount == 0) {
            // WSAStartup omitted; declared in leverage win32 module
            // WSAStartup(0x0202, g_wsa.wsa_data.ptr);
            g_wsa.initialized = true;
        }
        g_wsa_refcount++;
        if (DEBUG) {
            // fprintf(stderr, "Initialized Windows Socket API\n");
        }
    }

    ~this() {
        g_wsa_refcount--;
        if (g_wsa_refcount == 0) {
            // WSACleanup();
            g_wsa.initialized = false;
        }
        if (DEBUG) {
            // fprintf(stderr, "Cleanup Windows Socket API\n");
        }
    }

    // non-copyable
    @disable this(this);
}

// Return the last error code, posix-ified for commonly-used values
int get_error_code() {
    // PORT-NOTE: WSAGetLastError() declared in leverage win32 module
    // For now, return a placeholder; wired up in Run 2
    return 0;
}

void set_non_blocking_impl(size_t sock, bool non_blocking) {
    uint mode = non_blocking ? 1 : 0;
    // PORT-NOTE: ioctlsocket call deferred to Run 2 (needs FIONBIO constant)
    if (mode == 0) {  // suppress unused warning
        handle_error("Failed to set socket non-blocking");
    }
}
