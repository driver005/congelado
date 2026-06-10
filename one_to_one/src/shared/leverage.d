module shared.leverage;
@nogc nothrow:

import core.sys.posix.sys.socket : sockaddr, socklen_t;
import core.sys.posix.sys.uio : iovec;

// PORT-NOTE: std::move_only_function<void(int)> → function pointer + context void*.
// @nogc forbids delegates with GC closures; use fn+ctx pair at call sites.
alias CompletionCallback = void function(void* ctx, int result) @nogc nothrow;

enum IFlags : ubyte {
    None         = 0,
    FixedFile    = (1 << 0),  // Use registered file descriptors
    IoDrain      = (1 << 1),  // Wait for previous operations to complete
    IoLink       = (1 << 2),  // Link this operation to the next one
    IoHardLink   = (1 << 3),  // Stronger version of Link
    Async        = (1 << 4),  // Force async execution
    BufferSelect = (1 << 5),  // Use provided buffer ring
}

IFlags opOr(IFlags lhs, IFlags rhs) @nogc nothrow {
    return cast(IFlags)(cast(ubyte)lhs | cast(ubyte)rhs);
}

bool opAnd(IFlags lhs, IFlags rhs) @nogc nothrow {
    return (cast(ubyte)lhs & cast(ubyte)rhs) != 0;
}

// AsyncExecutor concept — expressed as a D template constraint.
// An AsyncExecutor E must satisfy all method requirements below.
template AsyncExecutor(E) {
    enum bool AsyncExecutor = is(typeof({
        E* e;
        // run / stop / poll
        e.run();
        e.stop();
        e.poll();
        // close
        CompletionCallback cb;
        IFlags iflags;
        e.close(0, cb, iflags);
        // readv / writev
        iovec iovecs_arr;
        e.readv(0, &iovecs_arr, 1u, 0, cb, iflags);
        e.writev(0, &iovecs_arr, 1u, 0, cb, iflags);
        // read / write
        void* buf;
        const(void)* cbuf;
        e.read(0, buf, 0u, 0, cb, iflags);
        e.write(0, cbuf, 0u, 0, cb, iflags);
        // read_fixed / write_fixed
        e.read_fixed(0, buf, 0u, 0, 0, cb, iflags);
        e.write_fixed(0, cbuf, 0u, 0, 0, cb, iflags);
        // accept / connect
        sockaddr* addr;
        socklen_t addrlen;
        e.accept(0, addr, &addrlen, 0, cb, iflags);
        e.connect(0, addr, addrlen, cb, iflags);
        // recv / send
        e.recv(0, buf, 0u, 0, cb, iflags);
        e.send(0, cbuf, 0u, 0, cb, iflags);
        // openat / unlinkat
        e.openat(0, cast(const(char)*)null, 0, 0, cb, iflags);
        e.unlinkat(0, cast(const(char)*)null, 0, cb, iflags);
        // register_files / register_buffers / unregister_*
        const(int)[] fds;
        const(iovec)[] iovecs_span;
        e.register_files(fds);
        e.register_buffers(iovecs_span);
        int _uf = e.unregister_files();
        int _ub = e.unregister_buffers();
    }));
}
