module;
#define UUID_SYSTEM_GENERATOR
#include <uuid.h>

export module model:identifiers;

import std;

template <>
struct std::formatter<uuids::uuid> {
    /**
     * @brief No format-spec support here — parses nothing and just hands the context back
     * untouched. `{}` is the only spec this understands.
     * @param ctx the format parse context positioned at the start of the (empty) format-spec.
     * @return an iterator to where parsing left off, i.e. ctx.begin() unchanged.
     */
    static constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

    /**
     * @brief Formats a uuid the same way uuids::to_string() does — this is the whole reason
     * std::format("{}", some_uuid) just works, no cap, no extra ceremony needed.
     * @tparam FormatContext the format context type, deduced by the std::format machinery.
     * @param uuid the uuid to stringify.
     * @param ctx the format context to write the formatted output into.
     * @return an iterator to the end of the written output.
     */
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
