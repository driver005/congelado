module;
#define UUID_SYSTEM_GENERATOR
#include <uuid.h>

export module model:identifiers;

import std;

export namespace model {

using WorkflowId    = uuids::uuid;
using TaskId        = uuids::uuid;
using ExecutionId   = uuids::uuid;
using CorrelationId = uuids::uuid;

inline ExecutionId generate_id() {
    static uuids::uuid_system_generator gen{};
    return gen();
}

} // namespace model
