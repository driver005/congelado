module interfaces.cache;
@nogc nothrow:

import shared.flow : QueryReadFn;

extern(C++) interface ICache {
    const(char)[] backend_name() const;
    bool required() const { return false; }

    void get(const(char)[] key, QueryReadFn result);
    void set(const(char)[] key, const(char)[] value, QueryReadFn result);
    void remove(const(char)[] key, QueryReadFn result);
}
