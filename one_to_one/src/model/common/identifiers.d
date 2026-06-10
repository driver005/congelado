module model.common.identifiers;

@nogc nothrow:

// PORT-NOTE: C++ used uuids::uuid (stduuid library) for all ID types.
// D port represents UUIDs as a 16-byte struct for @nogc compatibility.
// generate_id() uses /dev/urandom via getrandom(2) — caller must guard concurrency.

extern(C) int getrandom(void* buf, size_t buflen, uint flags) @nogc nothrow;

struct Uuid {
    ubyte[16] data;

    bool opEquals(const ref Uuid other) const {
        return data == other.data;
    }

    // Returns true if all bytes are zero (nil UUID).
    bool is_nil() const {
        foreach (b; data)
            if (b != 0) return false;
        return true;
    }
}

// Callers that invoke methods on returned UUIDs (e.g. .is_nil(), opEquals)
// must import model.common.identifiers.
alias WorkflowId    = Uuid;
alias TaskId        = Uuid;
alias ExecutionId   = Uuid;
alias CorrelationId = Uuid;

// uuid_system_generator uses the OS PRNG (re-entrant on Linux/macOS);
// operator() is not guaranteed thread-safe — guard externally if needed.
ExecutionId generate_id() {
    ExecutionId id;
    // PORT-NOTE: C++ used uuids::uuid_system_generator (OS PRNG).
    // D uses getrandom(2) (Linux) directly — no libstdc++ dependency.
    getrandom(id.data.ptr, 16, 0);
    // Set version 4 and variant bits per RFC 4122.
    id.data[6] = cast(ubyte)((id.data[6] & 0x0F) | 0x40);
    id.data[8] = cast(ubyte)((id.data[8] & 0x3F) | 0x80);
    return id;
}
