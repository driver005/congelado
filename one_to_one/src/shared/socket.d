module shared.socket;
@nogc nothrow:

import core.sys.posix.sys.socket : sockaddr, socklen_t;
import core.sys.posix.sys.uio : iovec;
import shared.leverage : CompletionCallback, IFlags;

// AsyncSocket concept — expressed as a D template constraint.
// Mirrors the C++ shared:socket AsyncSocket concept which requires run/stop/poll,
// close, read/write, accept/connect, recv/send, and buffer registration ops.
//
// Note: the openat/unlinkat block is commented out in the original C++ source;
// that state is preserved here.

template AsyncSocket(E) {
    enum bool AsyncSocket = is(typeof({
        E* e;

        // run / stop / poll
        e.run();
        e.stop();
        e.poll();

        // close (socket-level: no fd param, unlike AsyncExecutor)
        CompletionCallback cb;
        IFlags iflags;
        e.close(cb, iflags);

        // read / write (socket-level: no fd param)
        void* buf;
        const(void)* cbuf;
        e.read(buf, 0u, 0, cb, iflags);
        e.write(cbuf, 0u, 0, cb, iflags);

        // accept / connect
        sockaddr* addr;
        socklen_t addrlen;
        e.accept(addr, &addrlen, 0, cb, iflags);
        e.connect(addr, addrlen, cb, iflags);

        // recv / send
        e.recv(buf, 0u, 0, cb, iflags);
        e.send(cbuf, 0u, 0, cb, iflags);

        // requires(E &e, const char *path, int flags, mode_t mode, ...) {
        //     { e.openat(path, flags, mode, cb, iflags) } -> std::same_as<void>;
        //     { e.unlinkat(path, flags, cb, iflags) } -> std::same_as<void>;
        // } &&

        // register_files / register_buffers / unregister_*
        const(int)[] fds;
        const(iovec)[] iovecs;
        e.register_files(fds);
        e.register_buffers(iovecs);
        int _uf = e.unregister_files();
        int _ub = e.unregister_buffers();
    }));
}
