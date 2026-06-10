module interfaces.codec.db;
@nogc nothrow:

// PORT-NOTE: C++ virtual interface class -> D template class (not interface because
// IDbCodec is parameterized on T; D interfaces cannot be templates).
class IDbCodec(T) {
    abstract char[] encode_query(ref const T value) const;
    abstract char[] encode_insert(ref const T value) const;
    abstract char[] encode_update(ref const T value) const;
    abstract char[] encode_remove(ref const T value) const;

    abstract void decode(const(char)[] result, ref T out_) const;
}

// DbCodec concept -> D template constraint.
template DbCodec(Codec, T) {
    enum bool DbCodec = is(Codec : IDbCodec!T);
}
