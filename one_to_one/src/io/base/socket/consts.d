module io.base.socket.consts;
@nogc nothrow:

static immutable bool DEBUG = false;

version (Windows) {
    static immutable bool is_windows = true;
    // PORT-NOTE: SD_BOTH = 2 on Windows (from winsock2)
    static immutable int SHUT_RDWR = 2;  // SD_BOTH
} else {
    static immutable bool is_windows = false;
    static immutable int INVALID_SOCKET = -1;
    static immutable int SOCKET_ERROR   = -1;
}
