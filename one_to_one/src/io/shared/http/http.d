module io.shared.http.http;
@nogc nothrow:

// Umbrella re-export for io.shared.http.*
public import io.shared.http.header;
public import io.shared.http.types;

// Protocol namespace tag — mirrors C++ io::shared::http::Protocol.
// In C++ this was a class with nested using-aliases; in D a simple
// namespace-struct (no behaviour, just a type tag for generic code).
// PORT-NOTE: ABI POD value wrapper, exempt from class-only rule.
struct Protocol {
    alias Header = HeaderEntry;
    alias TokenType = Token;
}
