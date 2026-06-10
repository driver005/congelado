module io.error.base;
@nogc nothrow:

import core.stdc.stdio : fprintf, stderr;
import core.stdc.stdlib : abort;

// OpenSSL error string via extern(C)
extern(C) ulong ERR_get_error();
extern(C) void  ERR_error_string_n(ulong e, char* buf, size_t len);

// PORT-NOTE: C++ handle_error was a template over use_exception/abort_on_error/Fn.
// D port collapses the three orthogonal axes into three overloaded free functions:
//   handle_error(msg)                — logs to stderr
//   handle_error(msg, cb, ctx)       — invokes callback; cb may not be null
//   handle_error_abort(msg)          — logs to stderr + aborts

void handle_error(const(char)[] message) {
    fprintf(stderr, "Default Log: %.*s\n", cast(int) message.length, message.ptr);
}

alias ErrorCallback = void function(const(char)[], void*) @nogc nothrow;

void handle_error(const(char)[] message, ErrorCallback callback, void* ctx) {
    callback(message, ctx);
}

void handle_error_abort(const(char)[] message) {
    fprintf(stderr, "Default Log: %.*s\n", cast(int) message.length, message.ptr);
    abort();
}

// PORT-NOTE: TlsError was a std::runtime_error subclass; exceptions are gone.
// Converted to a plain struct that holds a static char[256+256] message buffer.
// Callers that previously caught TlsError should check the returned TlsError
// value (non-empty message means error).
struct TlsError {
    char[512] message;  // PORT-NOTE: fixed-size buf: ctx prefix (up to 255) + ": " + ERR_error_string (256)
    size_t    message_length;

    static TlsError make(const(char)[] ctx) @nogc nothrow {
        TlsError err;
        char[256] ssl_buf;
        ERR_error_string_n(ERR_get_error(), ssl_buf.ptr, ssl_buf.sizeof);

        // Build: ctx + ": " + ssl_buf (truncate to fit)
        size_t out_pos = 0;
        foreach (c; ctx) {
            if (out_pos >= err.message.length - 3) break;
            err.message[out_pos++] = c;
        }
        if (out_pos + 2 < err.message.length) {
            err.message[out_pos++] = ':';
            err.message[out_pos++] = ' ';
        }
        size_t i = 0;
        while (ssl_buf[i] != '\0' && out_pos < err.message.length - 1) {
            err.message[out_pos++] = ssl_buf[i++];
        }
        err.message[out_pos] = '\0';
        err.message_length = out_pos;
        return err;
    }

    bool is_error() const {
        return message_length > 0;
    }

    const(char)[] msg() const {
        return message[0 .. message_length];
    }
}
