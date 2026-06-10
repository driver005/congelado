module util.alloc;
@nogc nothrow:

import core.stdc.stdlib : malloc, free;
import core.lifetime : emplace;

T make(T, Args...)(auto ref Args args) if (is(T == class))
{
    enum size = __traits(classInstanceSize, T);
    void* mem = malloc(size);
    if (mem is null) assert(0, "oom");
    return emplace!T(mem[0 .. size], args);
}

void dispose(T)(ref T obj) if (is(T == class))
{
    if (obj is null) return;
    destroy!false(obj);
    free(cast(void*) obj);
    obj = null;
}
