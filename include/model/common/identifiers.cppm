module;
#define UUID_SYSTEM_GENERATOR
#include <uuid.h>

export module model:identifiers;

import std;

export namespace model {

using WorkflowId = uuids::uuid;
using TaskId = uuids::uuid;
using ExecutionId = uuids::uuid;
using CorrelationId = uuids::uuid;

// Callers that invoke methods on returned UUIDs (e.g. .is_nil(), operator!=)
// must #define UUID_SYSTEM_GENERATOR and #include <uuid.h> before import model;
inline ExecutionId generate_id() {
    // uuid_system_generator uses the OS PRNG (re-entrant on Linux/macOS);
    // operator() is not guaranteed thread-safe — guard externally if needed.
    static uuids::uuid_system_generator gen{};
    return gen();
}

} // namespace model
