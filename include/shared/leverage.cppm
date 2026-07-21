module;

#include <sys/socket.h>

export module shared:leverage;

import std;

export namespace shared::leverage {

using CompletionCallback = std::move_only_function<void(int)>;

enum class IFlags : std::uint8_t {
    NONE = 0,
    FIXED_FILE = (1 << 0),   // Use registered file descriptors
    IO_DRAIN = (1 << 1),     // Wait for previous operations to complete
    IO_LINK = (1 << 2),      // Link this operation to the next one
    IO_HARD_LINK = (1 << 3), // Stronger version of Link
    ASYNC = (1 << 4),        // Force async execution
    BUFFER_SELECT = (1 << 5) // Use provided buffer ring
};

constexpr IFlags operator|(IFlags lhs, IFlags rhs) {
    return static_cast<IFlags>(static_cast<std::uint8_t>(lhs) | static_cast<std::uint8_t>(rhs));
}

constexpr bool operator&(IFlags lhs, IFlags rhs) {
    return static_cast<bool>(static_cast<std::uint8_t>(lhs) & static_cast<std::uint8_t>(rhs));
}

template <typename E>
concept AsyncExecutor =
    requires(E &executor) {
        { executor.run() } -> std::same_as<void>;
        { executor.stop() } -> std::same_as<void>;
        { executor.poll() } -> std::same_as<void>;
    } &&
    requires(E &executor, int descriptor, CompletionCallback callback, IFlags iflags) {
        { executor.close(descriptor, callback, iflags) } -> std::same_as<void>;
    } &&
    requires(E &executor, int descriptor, const iovec *iovecs, unsigned count, off_t offset, CompletionCallback callback, IFlags iflags) {
        { executor.readv(descriptor, iovecs, count, offset, callback, iflags) } -> std::same_as<void>;
        { executor.writev(descriptor, iovecs, count, offset, callback, iflags) } -> std::same_as<void>;
    } &&
    requires(E &executor, int descriptor, void *buf, const void *cbuf, unsigned nbytes, off_t offset, CompletionCallback callback,
             IFlags iflags) {
        { executor.read(descriptor, buf, nbytes, offset, callback, iflags) } -> std::same_as<void>;
        { executor.write(descriptor, cbuf, nbytes, offset, callback, iflags) } -> std::same_as<void>;
    } &&
    requires(E &executor, int descriptor, void *buf, const void *cbuf, unsigned nbytes, off_t offset, int buf_index,
             CompletionCallback callback, IFlags iflags) {
        { executor.read_fixed(descriptor, buf, nbytes, offset, buf_index, callback, iflags) } -> std::same_as<void>;
        { executor.write_fixed(descriptor, cbuf, nbytes, offset, buf_index, callback, iflags) } -> std::same_as<void>;
    } &&
    requires(E &executor, int descriptor, sockaddr *addr, socklen_t *addrlen, int flags, CompletionCallback callback, IFlags iflags) {
        { executor.accept(descriptor, addr, addrlen, flags, callback, iflags) } -> std::same_as<void>;
        { executor.connect(descriptor, addr, *addrlen, callback, iflags) } -> std::same_as<void>;
    } &&
    requires(E &executor, int descriptor, void *buf, const void *cbuf, unsigned nbytes, int flags, CompletionCallback callback,
             IFlags iflags) {
        { executor.recv(descriptor, buf, nbytes, flags, callback, iflags) } -> std::same_as<void>;
        { executor.send(descriptor, cbuf, nbytes, flags, callback, iflags) } -> std::same_as<void>;
    } &&
    requires(E &executor, int dfd, const char *path, int flags, mode_t mode, CompletionCallback callback, IFlags iflags) {
        { executor.openat(dfd, path, flags, mode, callback, iflags) } -> std::same_as<void>;
        { executor.unlinkat(dfd, path, flags, callback, iflags) } -> std::same_as<void>;
    } && requires(E &executor, std::span<const int> fds, std::span<const iovec> iovecs) {
        { executor.register_files(fds) } -> std::same_as<void>;
        { executor.register_buffers(iovecs) } -> std::same_as<void>;
        { executor.unregister_files() } -> std::same_as<int>;
        { executor.unregister_buffers() } -> std::same_as<int>;
    };

} // namespace shared::leverage
