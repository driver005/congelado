module interfaces.io;
@nogc nothrow:

// IoCallback equivalent: a callable accepting (size_t, Status).
// D port: use a fn+ctx pair to stay @nogc (move_only_function not available).
// PORT-NOTE: C++ used std::move_only_function<void(size_t, Status)>; D uses fn+ctx pair.
struct IoCallback(Status) {
    // PORT-NOTE: value wrapper, exempt from classes-only rule
    void function(void* ctx, size_t bytes, Status status) @nogc nothrow fn;
    void* ctx;
}

// SYNC

// Concept equivalents expressed as template constraints (D does not have C++ concepts).
// PORT-NOTE: C++ concepts translated to template constraint comments only;
// concrete implementations are validated at usage sites.

// SyncSendable!T: T must have sync_send(const(ubyte)*, size_t, Args...) returning pair!(size_t, Status)
// SyncReceivable!T: T must have sync_receive(Out, size_t, size_t offset, Args...) returning pair!(size_t, Status)
// SyncClose!T: T must have sync_close() returning void
// SyncGetter!T: T must have get_fd() returning void

// IoSyncSend!T = SyncSendable && SyncGetter && SyncClose
// IoSyncReceive!T = SyncReceivable && SyncGetter && SyncClose
// IoSyncOps!T = SyncSendable && SyncReceivable && SyncGetter && SyncClose

// ASYNC

// AsyncSendable!T: T must have async_send(const(ubyte)*, size_t, IoCallback!Status, Args...) returning void
// AsyncReceivable!T: T must have async_receive(Out, size_t, IoCallback!Status, Args...) returning void
// AsyncClose!T: T must have async_close() returning void
// AsyncGetter!T: T must have get_fd(), attach(), detach() returning void

// IoAsyncSend!T = AsyncSendable && AsyncGetter && AsyncClose
// IoAsyncReceive!T = AsyncReceivable && AsyncGetter && AsyncClose
// IoAsyncOps!T = AsyncSendable && AsyncReceivable && AsyncGetter && AsyncClose

// Buf  – must satisfy SendBuffer  (e.g. ubyte[N])
// Out  – must satisfy ReceiveOutput (e.g. ubyte*, output range)
// Args – trailing extras forwarded to every sub-concept (e.g. AddressInfo*, ubyte)
// IoOps!T = AsyncSendable && SyncSendable && SyncReceivable && AsyncReceivable
