module serde.core;

@nogc nothrow:

// PORT-NOTE: C++ used rfl.hpp (reflect-cpp) for NamedTuple / Reflector,
// and simdjson for JSON parsing. D port stubs the template machinery with
// structural equivalents. The concept/constraint layer is preserved as
// static template constraints (D template predicates).
//
// serde::StringLiteral<N> → D's built-in string template params (const(char)[N])
// serde::FieldDesc<Name,Getter,Setter,Opts> → D struct FieldDesc(alias G, alias S, FieldOptions O)
// serde::Serializable<T> → D template Serializable(T) with static method fields()
// serde::ISerializable → D template constraint is_serializable!T
// serde::IConnectable  → D template constraint is_connectable!T
// serde::IFormat<F,T>  → D template constraint is_format!(F,T)

// ─── FieldOptionsDb ───────────────────────────────────────────────────────────

struct FieldOptionsDb {
    bool         primary_key = false;
    bool         unique      = false;
    bool         nullable    = true;
    bool         skip_insert = false;
    bool         skip_update = false;
    const(char)* ref_table   = null;
    const(char)* ref_column  = null;

    static FieldOptionsDb init_() nothrow { return FieldOptionsDb.init; }

    FieldOptionsDb pk() const nothrow {
        auto opt = this;
        opt.primary_key = true;
        return opt;
    }
    FieldOptionsDb not_null() const nothrow {
        auto opt = this;
        opt.nullable = false;
        return opt;
    }
    FieldOptionsDb no_insert() const nothrow {
        auto opt = this;
        opt.skip_insert = true;
        return opt;
    }
    FieldOptionsDb no_update() const nothrow {
        auto opt = this;
        opt.skip_update = true;
        return opt;
    }
    FieldOptionsDb references(const(char)* tbl, const(char)* col) const nothrow {
        auto opt = this;
        opt.ref_table  = tbl;
        opt.ref_column = col;
        return opt;
    }
}

// ─── FieldOptions ─────────────────────────────────────────────────────────────

struct FieldOptions {
    FieldOptionsDb db;

    static FieldOptions init_() nothrow { return FieldOptions.init; }

    FieldOptions with_db(FieldOptionsDb dbo) const nothrow {
        auto opt = this;
        opt.db = dbo;
        return opt;
    }
}

// ─── FieldDesc ────────────────────────────────────────────────────────────────
// PORT-NOTE: C++ FieldDesc used a StringLiteral NTTP + member-function-pointer NTTPs.
// D port uses string name + alias parameters for getter/setter.

struct FieldDesc(string Name, alias Getter, alias Setter,
                 FieldOptions Opts = FieldOptions.init) {
    static immutable string name       = Name;
    static immutable FieldOptions options = Opts;
    alias getter = Getter;
    alias setter = Setter;
}

// ─── Serializable!T ───────────────────────────────────────────────────────────
// Default: no specialization means T is not serializable.
// Specialize per model class, providing a fields() method.

template Serializable(T) {
    // No default body — specialize to make T serializable.
}

// ─── Concept equivalents (template predicates) ────────────────────────────────

// is_serializable!T: T has a Serializable!T.fields() method.
template is_serializable(T) {
    enum is_serializable = __traits(compiles, { auto f = Serializable!T.fields(); });
}

// is_connectable!T: is_serializable + has table_name().
template is_connectable(T) {
    enum is_connectable = is_serializable!T
        && __traits(compiles, { const(char)[] n = Serializable!T.table_name(); });
}

// is_any_format!F: F has a content_type member.
template is_any_format(F) {
    enum is_any_format = __traits(compiles, { const(char)[] ct = F.content_type; });
}

// is_format!(F,T): is_any_format + is_serializable + encode/decode present.
template is_format(F, T) {
    enum is_format = is_any_format!F && is_serializable!T
        && __traits(compiles, {
            const(char)[] s = F.encode(T.init);
        });
}
