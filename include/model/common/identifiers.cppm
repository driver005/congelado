module;
#define UUID_SYSTEM_GENERATOR
#include <uuid.h>

export module model:identifiers;

import std;

template <>
struct std::formatter<uuids::uuid> {
    constexpr auto parse(std::format_parse_context &ctx) const { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const uuids::uuid &uuid, FormatContext &ctx) const {
        return std::format_to(ctx.out(), "{}", uuids::to_string(uuid));
    }
};

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
