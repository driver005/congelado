module;

#include <sys/socket.h>

export module shared:leverage;

import std;

export namespace shared::leverage {

using CompletionCallback = std::move_only_function<void(int)>;

enum class IFlags : std::uint8_t {
    None = 0,
    FixedFile = (1 << 0),   // Use registered file descriptors
    IoDrain = (1 << 1),     // Wait for previous operations to complete
    IoLink = (1 << 2),      // Link this operation to the next one
    IoHardLink = (1 << 3),  // Stronger version of Link
    Async = (1 << 4),       // Force async execution
    BufferSelect = (1 << 5) // Use provided buffer ring
};

constexpr IFlags operator|(IFlags lhs, IFlags rhs) {
    return static_cast<IFlags>(static_cast<std::uint8_t>(lhs) | static_cast<std::uint8_t>(rhs));
}

constexpr bool operator&(IFlags lhs, IFlags rhs) {
    return static_cast<bool>(static_cast<std::uint8_t>(lhs) & static_cast<std::uint8_t>(rhs));
}

template <typename E>
concept AsyncExecutor =
    requires(E &e) {
        { e.run() } -> std::same_as<void>;
        { e.stop() } -> std::same_as<void>;
        { e.poll() } -> std::same_as<void>;
    } &&
    requires(E &e, int fd, CompletionCallback cb, IFlags iflags) {
        { e.close(fd, cb, iflags) } -> std::same_as<void>;
    } &&
    requires(E &e, int fd, const iovec *iovecs, unsigned nr, off_t offset, CompletionCallback cb, IFlags iflags) {
        { e.readv(fd, iovecs, nr, offset, cb, iflags) } -> std::same_as<void>;
        { e.writev(fd, iovecs, nr, offset, cb, iflags) } -> std::same_as<void>;
    } &&
    requires(E &e, int fd, void *buf, const void *cbuf, unsigned nbytes, off_t offset, CompletionCallback cb,
             IFlags iflags) {
        { e.read(fd, buf, nbytes, offset, cb, iflags) } -> std::same_as<void>;
        { e.write(fd, cbuf, nbytes, offset, cb, iflags) } -> std::same_as<void>;
    } &&
    requires(E &e, int fd, void *buf, const void *cbuf, unsigned nbytes, off_t offset, int buf_index,
             CompletionCallback cb, IFlags iflags) {
        { e.read_fixed(fd, buf, nbytes, offset, buf_index, cb, iflags) } -> std::same_as<void>;
        { e.write_fixed(fd, cbuf, nbytes, offset, buf_index, cb, iflags) } -> std::same_as<void>;
    } &&
    requires(E &e, int fd, sockaddr *addr, socklen_t *addrlen, int flags, CompletionCallback cb, IFlags iflags) {
        { e.accept(fd, addr, addrlen, flags, cb, iflags) } -> std::same_as<void>;
        { e.connect(fd, addr, *addrlen, cb, iflags) } -> std::same_as<void>;
    } &&
    requires(E &e, int fd, void *buf, const void *cbuf, unsigned nbytes, int flags, CompletionCallback cb,
             IFlags iflags) {
        { e.recv(fd, buf, nbytes, flags, cb, iflags) } -> std::same_as<void>;
        { e.send(fd, cbuf, nbytes, flags, cb, iflags) } -> std::same_as<void>;
    } &&
    requires(E &e, int dfd, const char *path, int flags, mode_t mode, CompletionCallback cb, IFlags iflags) {
        { e.openat(dfd, path, flags, mode, cb, iflags) } -> std::same_as<void>;
        { e.unlinkat(dfd, path, flags, cb, iflags) } -> std::same_as<void>;
    } && requires(E &e, std::span<const int> fds, std::span<const iovec> iovecs) {
        { e.register_files(fds) } -> std::same_as<void>;
        { e.register_buffers(iovecs) } -> std::same_as<void>;
        { e.unregister_files() } -> std::same_as<int>;
        { e.unregister_buffers() } -> std::same_as<int>;
    };

} // namespace shared::leverage
