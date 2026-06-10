module serde.cache;

@nogc nothrow:

import serde.core : is_connectable, Serializable, FieldOptions;
import serde.json : Json;

// PORT-NOTE: C++ used template metaprogramming over FieldDesc tuples to find the
// primary-key field and format cache keys. D port stubs the field iteration
// (Run 2 will wire it once Serializable!T.fields() is fully implemented).

// ─── field_value_to_string stub ───────────────────────────────────────────────

const(char)[] field_value_to_string(VT)(const ref VT value) nothrow {
    // TODO: convert rfl_type → string in Run 2.
    return ""; // stub
}

// ─── Cache ────────────────────────────────────────────────────────────────────

class Cache {
  public:
    // PORT-NOTE: C++ applied template over FieldDesc tuple to find primary_key field.
    // D stub: callers must supply their own pk serialization for now.
    // TODO: wire Serializable!T.fields() iteration in Run 2.
    static const(char)[] pk_string(T)(const ref T value) nothrow
            if (is_connectable!T) {
        return ""; // stub — iterate fields to find primary_key in Run 2
    }

    static const(char)[] cache_key(T)(const ref T value) nothrow
            if (is_connectable!T) {
        // PORT-NOTE: format("table_name:pk_string") — C++ used std::format.
        // D stub returns empty string; Run 2 will use snprintf into a stack buffer.
        return ""; // stub
    }

    static const(char)[] cache_key_by_pk(T)(const(char)[] pk_value) nothrow
            if (is_connectable!T) {
        return ""; // stub
    }

    static const(char)[] cache_value(T)(const ref T value) nothrow
            if (is_connectable!T) {
        return Json.encode!T(value);
    }
}
