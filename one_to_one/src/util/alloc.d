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

// Struct overload: allocate a struct on the heap via malloc, return pointer.
T* make_struct(T, Args...)(auto ref Args args) if (is(T == struct))
{
    T* mem = cast(T*) malloc(T.sizeof);
    if (mem is null) assert(0, "oom");
    import core.lifetime : emplace;
    emplace!T(mem, args);
    return mem;
}

void dispose_struct(T)(ref T* obj) if (is(T == struct))
{
    if (obj is null) return;
    destroy!false(*obj);
    free(cast(void*) obj);
    obj = null;
}
