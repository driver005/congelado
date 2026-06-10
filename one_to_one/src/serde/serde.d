module serde.serde;

@nogc nothrow:

// Umbrella re-export of all serde sub-modules.

public import serde.core;
public import serde.converter;
public import serde.json;
public import serde.toml;
public import serde.cache;
public import serde.sql;

import serde.core : is_serializable, is_any_format;
import serde.json : Json;
import serde.toml : Toml;
import util.result : Result;

// ─── SerBase!Fmts ─────────────────────────────────────────────────────────────
// Runtime wire-format dispatch. Reads the Accept / Content-Type header value
// and dispatches to a format class satisfying is_format!(F,T).
//
// To add a new wire format: define a class satisfying is_format!(F,T) and append
// it to the Ser alias at the bottom of this file.
//
// PORT-NOTE: C++ used a variadic template pack over format types with fold-expression
// dispatch. D port uses a fixed 2-format dispatch (Json, Toml).
// IMPROVEMENTS: generalise to a format registry array in Run 3.

// PORT-NOTE: C++ returned std::vector<std::byte>; D uses caller-supplied out buffer.
// TODO: wire actual encode in Run 2.

class SerBase(Fmts...) {
  public:
    // serialize: encode T according to accept header, write into out_buf.
    // Returns number of bytes written.
    static size_t serialize(T)(const(char)[] accept, const ref T value,
                               ubyte[] out_buf) nothrow
            if (is_serializable!T) {
        // PORT-NOTE: C++ dispatched via fold-expression; D stubs as JSON default.
        auto encoded = Json.encode!T(value);
        size_t n = encoded.length < out_buf.length ? encoded.length : out_buf.length;
        out_buf[0 .. n] = cast(const(ubyte)[]) encoded[0 .. n];
        return n;
    }

    // serialize_error: write {"error":"<message>"} into out_buf.
    static size_t serialize_error(const(char)[] accept,
                                  const(char)[] message,
                                  ubyte[] out_buf) nothrow {
        // PORT-NOTE: C++ used std::format(R"({{"error":"{}"}}", message)
        // D stub: copy message verbatim.
        enum prefix = `{"error":"`;
        enum suffix = `"}`;
        size_t pos = 0;
        foreach (ch; cast(const(ubyte)[]) prefix) {
            if (pos >= out_buf.length) break;
            out_buf[pos++] = ch;
        }
        foreach (ch; cast(const(ubyte)[]) message) {
            if (pos >= out_buf.length) break;
            out_buf[pos++] = ch;
        }
        foreach (ch; cast(const(ubyte)[]) suffix) {
            if (pos >= out_buf.length) break;
            out_buf[pos++] = ch;
        }
        return pos;
    }

    // serialize_raw: copy data into out_buf verbatim.
    static size_t serialize_raw(const(char)[] accept,
                                const(char)[] data,
                                ubyte[] out_buf) nothrow {
        size_t n = data.length < out_buf.length ? data.length : out_buf.length;
        out_buf[0 .. n] = cast(const(ubyte)[]) data[0 .. n];
        return n;
    }

    // deserialize: decode T from content_type/data.
    static Result!(T, const(char)[]) deserialize(T)(const(char)[] content_type_,
                                                    const(char)[] data) nothrow
            if (is_serializable!T) {
        // PORT-NOTE: C++ dispatched via fold-expression over Fmts.
        // D stub: only JSON supported; TOML stub deferred to Run 2.
        if (content_type_ == Toml.content_type)
            return Toml.decode!T(data);
        return Json.decode!T(data);
    }
}

// Default Ser = SerBase!(Json, Toml).
alias Ser = SerBase!(Json, Toml);
