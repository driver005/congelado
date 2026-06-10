module serde.json;

@nogc nothrow:

import serde.core      : is_serializable;
import serde.converter : TomlParser;
import util.result     : Result;

// PORT-NOTE: C++ used rfl::json::write (reflect-cpp) for encode, and simdjson
// ondemand parser for decode. D port stubs both encode and decode.
// Run 2 will wire a @nogc-compatible JSON library (e.g. mir-ion or hand-rolled).

class Json {
  public:
    static immutable const(char)[] content_type = "application/json";

    // PORT-NOTE: C++ returned std::string; D returns borrowed const(char)[].
    // TODO: implement using @nogc JSON serializer in Run 2.
    static const(char)[] encode(T)(const ref T value) nothrow
            if (is_serializable!T) {
        return "{}"; // stub
    }

    // Non-serializable overload (plain types).
    static const(char)[] encode_plain(T)(const ref T value) nothrow {
        return "{}"; // stub
    }

    // PORT-NOTE: C++ returned std::expected<T,std::string>; D returns Result!(T, const(char)[]).
    // TODO: implement using @nogc JSON parser (simdjson D binding) in Run 2.
    static Result!(T, const(char)[]) decode(T)(const(char)[] data) nothrow
            if (is_serializable!T) {
        return Result!(T, const(char)[]).err("JSON decode not yet implemented"); // stub
    }

    // Decode a JSON array into a fixed caller-supplied slice.
    // PORT-NOTE: C++ returned std::vector<T>; D writes into caller-supplied out[].
    // Returns the number of decoded elements.
    static size_t decode_array(T)(const(char)[] data, T[] out_buf) nothrow
            if (is_serializable!T) {
        return 0; // stub
    }
}
