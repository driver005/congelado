module shared.flow;
@nogc nothrow:

import utils.buffering.reader : BufferReader;
import utils.buffering.node   : BufferNode;
import shared.handler : HandlerController;

// PORT-NOTE: std::move_only_function<void(BufferReader&)> etc. → fn+ctx pairs under
// @nogc. Each callback alias below pairs a function pointer with an implicit void*
// context at the call site.
//
// ReadCallback  — invoked when data arrives; receives a BufferReader ref
// SendCallback  — invoked to send; takes ownership of a BufferNode (moved)
// CloseCallback — invoked on connection close; no arguments
// ErrorCallback — invoked on error; (errno, sub-error-code)
// CompletionCallback — generic int result callback
// QueryReadFn   — invoked with a string_view (const char[]) query key

alias ReadCallback       = void function(void* ctx, ref BufferReader reader)    @nogc nothrow;
alias SendCallback       = void function(void* ctx, BufferNode node)            @nogc nothrow;
alias CloseCallback      = void function(void* ctx)                             @nogc nothrow;
alias ErrorCallback      = void function(void* ctx, int err, int sub_err)       @nogc nothrow;
alias CompletionCallback = void function(void* ctx, int result)                 @nogc nothrow;
alias QueryReadFn        = void function(void* ctx, const(char)[] key)          @nogc nothrow;

// FlowLayer concept — T must be constructible from (SendCallback, CloseCallback)
// and expose an on_read() method returning a ReadCallback.
template FlowLayer(T) {
    enum bool FlowLayer = is(typeof({
        SendCallback  send;
        CloseCallback close;
        T* layer = null; // construction checked at instantiation
        ReadCallback cb = layer.on_read();
    }));
}

// FlowBase concept — T must satisfy HandlerController(Controller), be constructible
// from (ReadCallback&&, Leverager&, Controller), and expose on_send(fd) → SendCallback.
template FlowBase(T, Controller, Leverager) {
    enum bool FlowBase = HandlerController!Controller && is(typeof({
        T* t;
        int fd;
        SendCallback cb = t.on_send(fd);
    }));
}
