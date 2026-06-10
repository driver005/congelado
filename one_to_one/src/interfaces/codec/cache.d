module interfaces.codec.cache;
@nogc nothrow:

// PORT-NOTE: C++ virtual interface class -> D template class (not interface because
// ICacheCodec is parameterized on T; D interfaces cannot be templates).
class ICacheCodec(T) {
    abstract char[] key(ref const T value) const;
    abstract char[] encode(ref const T value) const;

    abstract void decode(const(char)[] value, ref T out_) const;
}

// CacheCodec concept -> D template constraint.
template CacheCodec(Codec, T) {
    enum bool CacheCodec = is(Codec : ICacheCodec!T);
}
