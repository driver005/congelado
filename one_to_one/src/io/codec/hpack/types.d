module io.codec.hpack.types;
@nogc nothrow:

import io.codec.hpack.consts;

// Some header that are recommended
private static immutable const(char)[][4] NEVER_INDEXED = [
    "authorization",
    "proxy-authorization",
    "cookie",
    "set-cookie",
];

enum EncodePolicy : ubyte {
    WithIndexing,
    WithoutIndexing,
    NeverIndexed,
}

EncodePolicy policy_for(const(char)[] name) pure {
    foreach (s; NEVER_INDEXED)
        if (name == s)
            return EncodePolicy.NeverIndexed;
    return EncodePolicy.WithIndexing;
}
