module serde.toml;

@nogc nothrow:

import serde.core      : is_serializable;
import serde.converter : TomlParser;
import util.result     : Result;

// PORT-NOTE: C++ used rfl::toml::write (reflect-cpp) for encode, and toml++ for decode.
// D port stubs both. Run 2 will integrate a D TOML library.

class Toml {
  public:
    static immutable const(char)[] content_type = "application/toml";

    // PORT-NOTE: C++ returned std::string; D stubs.
    // TODO: implement using @nogc TOML serializer in Run 2.
    static const(char)[] encode(T)(const ref T value) nothrow
            if (is_serializable!T) {
        return ""; // stub
    }

    static const(char)[] encode_plain(T)(const ref T value) nothrow {
        return ""; // stub
    }

    // PORT-NOTE: C++ caught toml::parse_error and returned std::unexpected.
    // D stub returns an error.
    // TODO: implement using @nogc TOML parser in Run 2.
    static Result!(T, const(char)[]) decode(T)(const(char)[] data) nothrow
            if (is_serializable!T) {
        return Result!(T, const(char)[]).err("TOML decode not yet implemented"); // stub
    }
}

// model::Toml — convenience wrapper that calls serde::TomlParser::from_toml_impl.
// PORT-NOTE: C++ lived in a separate namespace block in the same file.
class ModelToml {
  public:
    // PORT-NOTE: from_toml delegates to serde::TomlParser::from_toml_impl.
    // TODO: implement in Run 2 once TomlParser stub is filled.
    static Result!(bool, const(char)[]) from_toml(T)(const(void)* table, ref T obj) nothrow
            if (is_serializable!T) {
        return TomlParser.from_toml_impl!T(table, obj);
    }
}
