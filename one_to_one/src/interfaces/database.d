module interfaces.database;
@nogc nothrow:

import shared.flow : QueryReadFn;

extern(C++) interface IDatabase {
    const(char)[] backend_name() const;
    bool required() const { return false; }

    void query(const(char)[] payload, QueryReadFn result);
    void insert(const(char)[] payload, QueryReadFn result);
    void update(const(char)[] payload, QueryReadFn result);
    void remove(const(char)[] payload, QueryReadFn result);
}
