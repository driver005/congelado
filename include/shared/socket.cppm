module;

#include <sys/socket.h>

export module shared:socket;

import std;
import :leverage;

export namespace shared::socket {


template <typename E>
concept AsyncSocket =
    requires(E &e) {
        { e.run() } -> std::same_as<void>;
        { e.stop() } -> std::same_as<void>;
        { e.poll() } -> std::same_as<void>;
    } &&
    requires(E &e, leverage::CompletionCallback cb, leverage::IFlags iflags) {
        { e.close(cb, iflags) } -> std::same_as<void>;
    } &&
    requires(E &e, void *buf, const void *cbuf, unsigned nbytes, off_t offset, leverage::CompletionCallback cb,
             leverage::IFlags iflags) {
        { e.read(buf, nbytes, offset, cb, iflags) } -> std::same_as<void>;
        { e.write(cbuf, nbytes, offset, cb, iflags) } -> std::same_as<void>;
    } &&
    requires(E &e, sockaddr *addr, socklen_t *addrlen, int flags, leverage::CompletionCallback cb,
             leverage::IFlags iflags) {
        { e.accept(addr, addrlen, flags, cb, iflags) } -> std::same_as<void>;
        { e.connect(addr, *addrlen, cb, iflags) } -> std::same_as<void>;
    } &&
    requires(E &e, void *buf, const void *cbuf, unsigned nbytes, int flags, leverage::CompletionCallback cb,
             leverage::IFlags iflags) {
        { e.recv(buf, nbytes, flags, cb, iflags) } -> std::same_as<void>;
        { e.send(cbuf, nbytes, flags, cb, iflags) } -> std::same_as<void>;
    } &&
    // requires(E &e, const char *path, int flags, mode_t mode, leverage::CompletionCallback cb, leverage::IFlags
    // iflags) {
    //     { e.openat(path, flags, mode, cb, iflags) } -> std::same_as<void>;
    //     { e.unlinkat(path, flags, cb, iflags) } -> std::same_as<void>;
    // } &&
    requires(E &e, std::span<const int> fds, std::span<const iovec> iovecs) {
        { e.register_files(fds) } -> std::same_as<void>;
        { e.register_buffers(iovecs) } -> std::same_as<void>;
        { e.unregister_files() } -> std::same_as<int>;
        { e.unregister_buffers() } -> std::same_as<int>;
    };


} // namespace shared::socket
