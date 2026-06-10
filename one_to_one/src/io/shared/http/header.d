module io.shared.http.header;
@nogc nothrow:

import io.shared.consts  : ENTRY_OVERHEAD;
import io.shared.http.types : Token, token_to_string;
import util.alloc : make, dispose;

// PORT-NOTE: C++ template HeaderField<bool IsStatic> split into two D structs:
//   HeaderFieldStatic  (IsStatic=true)  — name is a Token enum value
//   HeaderField        (IsStatic=false) — name is a heap-allocated const(char)[]
// Both are ABI-POD value types; callers wrap in pointers via make!/dispose.

// PORT-NOTE: C++ variant<shared_ptr<HeaderField<true>>, shared_ptr<HeaderField<false>>>
// → D tagged union HeaderEntry. Heap ownership signalled by PORT-NOTE below.

// Static header field: name is a Token; value is a string_view-length slice.
struct HeaderFieldStatic {
    // PORT-NOTE: ABI POD, exempt from class-only rule (value wrapper)
    Token        m_name;
    const(char)[] m_value;

    const(char)[] get_name_str() const pure { return token_to_string(m_name); }
    Token         get_name()  const pure { return m_name; }
    const(char)[] get_value() const pure { return m_value; }

    size_t size() const pure {
        return Token.sizeof + m_value.length + ENTRY_OVERHEAD;
    }

    bool is_empty() const pure { return m_value.length == 0; }

    void set_value(const(char)[] value) { m_value = value; }

    bool opEquals(ref const HeaderFieldStatic other) const pure {
        return m_name == other.m_name && m_value == other.m_value;
    }
}

// Dynamic header field: name is an owned heap string.
struct HeaderField {
    // PORT-NOTE: ABI POD, exempt from class-only rule (value wrapper)
    const(char)[] m_name;
    const(char)[] m_value;

    const(char)[] get_name()  const pure { return m_name; }
    const(char)[] get_value() const pure { return m_value; }

    size_t size() const pure {
        return m_name.length + m_value.length + ENTRY_OVERHEAD;
    }

    bool is_empty() const pure { return m_value.length == 0; }

    // Setters
    // PORT-NOTE: C++ throws on empty name → D returns false (no exceptions).
    bool set_name(const(char)[] name) {
        if (name.length == 0) return false;
        m_name = name;
        return true;
    }

    void set_value(const(char)[] value) { m_value = value; }

    bool opEquals(ref const HeaderField other) const pure {
        return m_name == other.m_name && m_value == other.m_value;
    }
}

// Export a common type for header entries, which can be either static or dynamic.
// PORT-NOTE: C++ std::variant<shared_ptr<HeaderField<true>>, shared_ptr<HeaderField<false>>>
// → D tagged union with raw pointers. Ownership: callers allocate via make!/dispose.
enum HeaderEntryKind : ubyte { Static = 0, Dynamic = 1 }

struct HeaderEntry {
    // PORT-NOTE: ABI POD, exempt from class-only rule (value wrapper)
    HeaderEntryKind kind;
    union {
        HeaderFieldStatic* static_field;
        HeaderField*       dynamic_field;
    }
}
