module serde.converter;

@nogc nothrow:

import serde.core;

// PORT-NOTE: C++ used rfl.hpp NamedTuple machinery, simdjson, and toml++ to perform
// field-by-field serialization. D port stubs the FieldConverter<VT> template with
// a minimal interface that Run 2 will flesh out with actual JSON/TOML libraries.
//
// Key mappings:
//   FieldConverter<uuids::uuid>          → FieldConverter!Uuid
//   FieldConverter<std::chrono::tp>      → FieldConverter!long (Unix epoch ms)
//   FieldConverter<std::optional<T>>     → FieldConverter!(Optional!T)
//   FieldConverter<std::vector<string>>  → FieldConverter!(const(char)[][])
//   FieldConverter<std::unordered_map>   → FieldConverter!(KvEntry[])
//
// TomlParser::from_toml_impl is preserved as a stub.

import model.common.identifiers : Uuid;
import util.optional            : Optional;
import util.result              : Result;

// ─── FieldConverter primary template (stub) ───────────────────────────────────

struct FieldConverter(VT) {
    // PORT-NOTE: from_simdjson / from_toml / to_rfl / from_rfl stubbed.
    // Run 2 will wire actual JSON/TOML parsing.
    static const(char)[] to_string(const ref VT v) nothrow { return ""; }
}

// ─── FieldConverter!Uuid ──────────────────────────────────────────────────────

struct FieldConverter_Uuid {
    alias rfl_type = const(char)[];

    // PORT-NOTE: C++ converted to/from uuids::uuid string; D uses Uuid.data bytes.
    static const(char)[] to_rfl(const ref Uuid v) nothrow { return ""; /* TODO: uuid→string */ }
    static Uuid from_rfl(const(char)[] s) nothrow { return Uuid.init; /* TODO: string→uuid */ }
}

// ─── FieldConverter!long (Unix epoch ms) ─────────────────────────────────────

struct FieldConverter_Long {
    alias rfl_type = long;

    static long to_rfl(long v) nothrow { return v; }
    static long from_rfl(long v) nothrow { return v; }
}

// ─── FieldConverter!(const(char)[]) ──────────────────────────────────────────

struct FieldConverter_String {
    alias rfl_type = const(char)[];

    static const(char)[] to_rfl(const(char)[] v) nothrow { return v; }
    static const(char)[] from_rfl(const(char)[] v) nothrow { return v; }
}

// ─── TomlParser ───────────────────────────────────────────────────────────────

class TomlParser {
  public:
    // PORT-NOTE: C++ used toml::table + recursive field extraction.
    // D port is a stub; Run 2 integrates a D TOML library.
    // TODO: implement from_toml_impl using @nogc TOML parser in Run 2.
    static Result!(bool, const(char)[]) from_toml_impl(T)(const(void)* table, ref T obj) nothrow {
        return Result!(bool, const(char)[]).ok(true); // stub
    }
}

// ─── build_named_tuple / apply_named_tuple_to stubs ───────────────────────────
// PORT-NOTE: C++ used rfl::NamedTuple + fold expressions over field descriptors.
// D equivalent will require a Phobos-free reflection library in Run 2.
// These are preserved as documentation stubs only.
//
// template build_named_tuple(T, Fields...) { ... }
// template apply_named_tuple_to(T, NT, Fields...) { ... }
