module io.shared.consts;
@nogc nothrow:

// io::shared namespace constants

enum size_t ENTRY_OVERHEAD = 32;

// PORT-NOTE: std::string literals -> static immutable const(char)[] for @nogc.
static immutable const(char)[] COOKIE_SEPARATOR = "; ";
static immutable const(char)[] VALUE_SEPARATOR  = ", ";
