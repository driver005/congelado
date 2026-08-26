module;

#include <sys/socket.h>

export module shared:leverage;

import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace shared::leverage {

using CompletionCallback = std::move_only_function<void(int)>;

enum class IFlags : std::uint8_t
{
    NONE = 0,
    FIXED_FILE = (1 << 0),   // Use registered file descriptors
    IO_DRAIN = (1 << 1),     // Wait for previous operations to complete
    IO_LINK = (1 << 2),      // Link this operation to the next one
    IO_HARD_LINK = (1 << 3), // Stronger version of Link
    ASYNC = (1 << 4),        // Force async execution
    BUFFER_SELECT = (1 << 5) // Use provided buffer ring
};

constexpr IFlags operator|(IFlags lhs, IFlags rhs)
{
    return static_cast<IFlags>(static_cast<std::uint8_t>(lhs) | static_cast<std::uint8_t>(rhs));
}

constexpr bool operator&(IFlags lhs, IFlags rhs)
{
    return static_cast<bool>(static_cast<std::uint8_t>(lhs) & static_cast<std::uint8_t>(rhs));
}

template<typename E>
concept AsyncExecutor =
    requires(E& executor) {
        { executor.run() } -> std::same_as<void>;
        { executor.stop() } -> std::same_as<void>;
        { executor.poll() } -> std::same_as<void>;
    } &&
    requires(E& executor, int descriptor, CompletionCallback callback, IFlags iflags) {
        { executor.close(descriptor, std::move(callback), iflags) } -> std::same_as<void>;
    } &&
    requires(
        E& executor,
        int descriptor,
        const iovec* iovecs,
        unsigned count,
        off_t offset,
        CompletionCallback callback,
        IFlags iflags
    ) {
        {
            executor.readv(descriptor, iovecs, count, offset, std::move(callback), iflags)
        } -> std::same_as<void>;
        {
            executor.writev(descriptor, iovecs, count, offset, std::move(callback), iflags)
        } -> std::same_as<void>;
    } &&
    requires(
        E& executor,
        int descriptor,
        void* buf,
        const void* cbuf,
        unsigned nbytes,
        off_t offset,
        CompletionCallback callback,
        IFlags iflags
    ) {
        {
            executor.read(descriptor, buf, nbytes, offset, std::move(callback), iflags)
        } -> std::same_as<void>;
        {
            executor.write(descriptor, cbuf, nbytes, offset, std::move(callback), iflags)
        } -> std::same_as<void>;
    } &&
    requires(
        E& executor,
        int descriptor,
        void* buf,
        const void* cbuf,
        unsigned nbytes,
        off_t offset,
        int buf_index,
        CompletionCallback callback,
        IFlags iflags
    ) {
        {
            executor.read_fixed(
                descriptor, buf, nbytes, offset, buf_index, std::move(callback), iflags
            )
        } -> std::same_as<void>;
        {
            executor.write_fixed(
                descriptor, cbuf, nbytes, offset, buf_index, std::move(callback), iflags
            )
        } -> std::same_as<void>;
    } &&
    requires(
        E& executor,
        int descriptor,
        sockaddr* addr,
        socklen_t* addrlen,
        int flags,
        CompletionCallback callback,
        IFlags iflags
    ) {
        {
            executor.accept(descriptor, addr, addrlen, flags, std::move(callback), iflags)
        } -> std::same_as<void>;
        {
            executor.connect(descriptor, addr, *addrlen, std::move(callback), iflags)
        } -> std::same_as<void>;
    } &&
    requires(
        E& executor,
        int descriptor,
        void* buf,
        const void* cbuf,
        unsigned nbytes,
        int flags,
        CompletionCallback callback,
        IFlags iflags
    ) {
        {
            executor.recv(descriptor, buf, nbytes, flags, std::move(callback), iflags)
        } -> std::same_as<void>;
        {
            executor.send(descriptor, cbuf, nbytes, flags, std::move(callback), iflags)
        } -> std::same_as<void>;
    } &&
    requires(
        E& executor,
        int dfd,
        const char* path,
        int flags,
        mode_t mode,
        CompletionCallback callback,
        IFlags iflags
    ) {
        {
            executor.openat(dfd, path, flags, mode, std::move(callback), iflags)
        } -> std::same_as<void>;
        { executor.unlinkat(dfd, path, flags, std::move(callback), iflags) } -> std::same_as<void>;
    } &&
    requires(E& executor, std::span<const int> fds, std::span<const iovec> iovecs) {
        { executor.register_files(fds) } -> std::same_as<void>;
        { executor.register_buffers(iovecs) } -> std::same_as<void>;
        { executor.unregister_files() } -> std::same_as<int>;
        { executor.unregister_buffers() } -> std::same_as<int>;
    };

} // namespace shared::leverage

#ifdef CONGELADO_TEST
namespace shared::leverage::tests {

// Satisfies AsyncExecutor in full — every required member, correct signature/return type.
class FullMockExecutor
{
public:
    void run() {}

    void stop() {}

    void poll() {}

    void close(int, CompletionCallback, IFlags) {}

    void readv(int, const iovec*, unsigned, off_t, CompletionCallback, IFlags) {}

    void writev(int, const iovec*, unsigned, off_t, CompletionCallback, IFlags) {}

    void read(int, void*, unsigned, off_t, CompletionCallback, IFlags) {}

    void write(int, const void*, unsigned, off_t, CompletionCallback, IFlags) {}

    void read_fixed(int, void*, unsigned, off_t, int, CompletionCallback, IFlags) {}

    void write_fixed(int, const void*, unsigned, off_t, int, CompletionCallback, IFlags) {}

    void accept(int, sockaddr*, socklen_t*, int, CompletionCallback, IFlags) {}

    void connect(int, sockaddr*, socklen_t, CompletionCallback, IFlags) {}

    void recv(int, void*, unsigned, int, CompletionCallback, IFlags) {}

    void send(int, const void*, unsigned, int, CompletionCallback, IFlags) {}

    void openat(int, const char*, int, mode_t, CompletionCallback, IFlags) {}

    void unlinkat(int, const char*, int, CompletionCallback, IFlags) {}

    void register_files(std::span<const int>) {}

    void register_buffers(std::span<const iovec>) {}

    int unregister_files()
    {
        return 0;
    }

    int unregister_buffers()
    {
        return 0;
    }
};

// Missing every I/O member — only the lifecycle trio is present.
class PartialMockExecutor
{
public:
    void run() {}

    void stop() {}

    void poll() {}
};

using namespace boost::ut;

suite<"IFlags bitwise operators"> iflags_suite = [] {
    "operator| combines two distinct flags"_test = [] {
        auto combined = IFlags::FIXED_FILE | IFlags::IO_DRAIN;
        expect(combined & IFlags::FIXED_FILE);
        expect(combined & IFlags::IO_DRAIN);
        expect(!(combined & IFlags::IO_LINK));
    };

    "operator& is false when neither operand shares a bit"_test = [] {
        expect(!(IFlags::FIXED_FILE & IFlags::IO_DRAIN));
    };

    "NONE has no bits set"_test = [] {
        expect(!(IFlags::NONE & IFlags::FIXED_FILE));
        expect(!(IFlags::NONE & IFlags::ASYNC));
    };

    "operator| is associative/idempotent when combining the same flag twice"_test = [] {
        auto combined = IFlags::ASYNC | IFlags::ASYNC;
        expect(combined & IFlags::ASYNC);
    };
};

suite<"AsyncExecutor concept"> async_executor_concept_suite = [] {
    "a type implementing every required member satisfies AsyncExecutor"_test = [] {
        expect(AsyncExecutor<FullMockExecutor>);
    };

    "a type missing the I/O members does not satisfy AsyncExecutor"_test = [] {
        expect(!AsyncExecutor<PartialMockExecutor>);
    };
};

} // namespace shared::leverage::tests
#endif
