module;

#include <sys/socket.h>

export module shared:socket;

import std;
import :leverage;

export namespace shared::socket {


template <typename E>
concept AsyncSocket =
    requires(E &engine) {
        { engine.run() } -> std::same_as<void>;
        { engine.stop() } -> std::same_as<void>;
        { engine.poll() } -> std::same_as<void>;
    } &&
    requires(E &engine, leverage::CompletionCallback callback, leverage::IFlags iflags) {
        { engine.close(callback, iflags) } -> std::same_as<void>;
    } &&
    requires(E &engine, void *buf, const void *cbuf, unsigned nbytes, off_t offset, leverage::CompletionCallback callback,
             leverage::IFlags iflags) {
        { engine.read(buf, nbytes, offset, callback, iflags) } -> std::same_as<void>;
        { engine.write(cbuf, nbytes, offset, callback, iflags) } -> std::same_as<void>;
    } &&
    requires(E &engine, sockaddr *addr, socklen_t *addrlen, int flags, leverage::CompletionCallback callback,
             leverage::IFlags iflags) {
        { engine.accept(addr, addrlen, flags, callback, iflags) } -> std::same_as<void>;
        { engine.connect(addr, *addrlen, callback, iflags) } -> std::same_as<void>;
    } &&
    requires(E &engine, void *buf, const void *cbuf, unsigned nbytes, int flags, leverage::CompletionCallback callback,
             leverage::IFlags iflags) {
        { engine.recv(buf, nbytes, flags, callback, iflags) } -> std::same_as<void>;
        { engine.send(cbuf, nbytes, flags, callback, iflags) } -> std::same_as<void>;
    } &&
    // requires(E &engine, const char *path, int flags, mode_t mode, leverage::CompletionCallback callback, leverage::IFlags
    // iflags) {
    //     { engine.openat(path, flags, mode, callback, iflags) } -> std::same_as<void>;
    //     { engine.unlinkat(path, flags, callback, iflags) } -> std::same_as<void>;
    // } &&
    requires(E &engine, std::span<const int> fds, std::span<const iovec> iovecs) {
        { engine.register_files(fds) } -> std::same_as<void>;
        { engine.register_buffers(iovecs) } -> std::same_as<void>;
        { engine.unregister_files() } -> std::same_as<int>;
        { engine.unregister_buffers() } -> std::same_as<int>;
    };


} // namespace shared::socket
