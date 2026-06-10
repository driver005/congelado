module shared.types;
@nogc nothrow:

// Concepts/constraints for byte-range readers and writers, mirroring the C++
// shared:types partition which defines ByteRangeReader, ByteRangeWriter,
// ByteIteratorReader, ByteIteratorWriter.
//
// D does not have C++ range concepts. The template constraints below encode the
// same structural requirements using D's `__traits` and static-if idioms.

// PORT-NOTE: C++ concepts mapped to D template constraints (if(...)).
// ByteRangeReader  — a forward range whose element type is ubyte (std::byte → ubyte)
// ByteRangeWriter  — a forward output range of ubyte
// ByteIteratorReader — a forward iterator whose value type is ubyte
// ByteIteratorWriter — a forward output iterator for ubyte
//
// In D there is no standard range-concept library; the constraints are expressed
// inline at each template instantiation site. The four aliases below give consumers
// a named hook for documentation and potential enforcement via static assert.

template ByteRangeReader(R) {
    // R must be an input range whose element type is ubyte
    enum bool ByteRangeReader = is(typeof({
        R r = R.init;
        ubyte b = r.front;
        r.popFront();
        bool e = r.empty;
    }));
}

template ByteRangeWriter(R) {
    // R must be an output range that accepts ubyte
    enum bool ByteRangeWriter = is(typeof({
        R r = R.init;
        ubyte b;
        r.front = b;
        r.popFront();
        bool e = r.empty;
    }));
}

template ByteIteratorReader(It) {
    // It must be a forward iterator whose dereferenced value is ubyte
    enum bool ByteIteratorReader = is(typeof({
        It it = It.init;
        ubyte b = *it;
        ++it;
    }));
}

template ByteIteratorWriter(It) {
    // It must be a forward output iterator that accepts ubyte
    enum bool ByteIteratorWriter = is(typeof({
        It it = It.init;
        *it = ubyte(0);
        ++it;
    }));
}
