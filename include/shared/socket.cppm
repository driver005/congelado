module;

#include <sys/socket.h>

export module shared:socket;

import std;
import :leverage;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace shared::socket {


template <typename E>
concept AsyncSocket =
    requires(E &engine) {
        { engine.run() } -> std::same_as<void>;
        { engine.stop() } -> std::same_as<void>;
        { engine.poll() } -> std::same_as<void>;
    } &&
    requires(E &engine, leverage::CompletionCallback callback, leverage::IFlags iflags) {
        { engine.close(std::move(callback), iflags) } -> std::same_as<void>;
    } &&
    requires(E &engine, void *buf, const void *cbuf, unsigned nbytes, off_t offset, leverage::CompletionCallback callback,
             leverage::IFlags iflags) {
        { engine.read(buf, nbytes, offset, std::move(callback), iflags) } -> std::same_as<void>;
        { engine.write(cbuf, nbytes, offset, std::move(callback), iflags) } -> std::same_as<void>;
    } &&
    requires(E &engine, sockaddr *addr, socklen_t *addrlen, int flags, leverage::CompletionCallback callback,
             leverage::IFlags iflags) {
        { engine.accept(addr, addrlen, flags, std::move(callback), iflags) } -> std::same_as<void>;
        { engine.connect(addr, *addrlen, std::move(callback), iflags) } -> std::same_as<void>;
    } &&
    requires(E &engine, void *buf, const void *cbuf, unsigned nbytes, int flags, leverage::CompletionCallback callback,
             leverage::IFlags iflags) {
        { engine.recv(buf, nbytes, flags, std::move(callback), iflags) } -> std::same_as<void>;
        { engine.send(cbuf, nbytes, flags, std::move(callback), iflags) } -> std::same_as<void>;
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

#ifdef CONGELADO_TEST
namespace shared::socket::tests {

// Satisfies AsyncSocket in full — every required member, correct signature/return type.
class FullMockEngine {
  public:
    void run() {}
    void stop() {}
    void poll() {}
    void close(leverage::CompletionCallback, leverage::IFlags) {}
    void read(void *, unsigned, off_t, leverage::CompletionCallback, leverage::IFlags) {}
    void write(const void *, unsigned, off_t, leverage::CompletionCallback, leverage::IFlags) {}
    void accept(sockaddr *, socklen_t *, int, leverage::CompletionCallback, leverage::IFlags) {}
    void connect(sockaddr *, socklen_t, leverage::CompletionCallback, leverage::IFlags) {}
    void recv(void *, unsigned, int, leverage::CompletionCallback, leverage::IFlags) {}
    void send(const void *, unsigned, int, leverage::CompletionCallback, leverage::IFlags) {}
    void register_files(std::span<const int>) {}
    void register_buffers(std::span<const iovec>) {}
    int unregister_files() { return 0; }
    int unregister_buffers() { return 0; }
};

// Missing every I/O member — only the lifecycle trio is present.
class PartialMockEngine {
  public:
    void run() {}
    void stop() {}
    void poll() {}
};

using namespace boost::ut;

suite<"AsyncSocket concept"> async_socket_concept_suite = [] {
    "a type implementing every required member satisfies AsyncSocket"_test = [] {
        expect(AsyncSocket<FullMockEngine>);
    };

    "a type missing the I/O members does not satisfy AsyncSocket"_test = [] {
        expect(!AsyncSocket<PartialMockEngine>);
    };
};

} // namespace shared::socket::tests
#endif
