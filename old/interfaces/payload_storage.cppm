export module interfaces:payload_storage;

import std;
import shared;

export namespace interfaces {

/// @brief Which payload this call is storing/reading — mirrors Conductor's own four categories
/// (workflow-level vs task-level, input vs output).
enum class PayloadType : std::uint8_t
{
    WORKFLOW_INPUT,
    WORKFLOW_OUTPUT,
    TASK_INPUT,
    TASK_OUTPUT,
};

/// @brief Externalized payload storage for input/output blobs too large to keep inline in a
/// WorkflowExecution/TaskInstance's own flat data map — write it here instead, keep just the
/// returned reference in the row. Same "opaque string in, opaque string out" idiom `IDatabase`/
/// `ISearchProvider` already use.
/// @warning Interface + a working local-disk default exist; NOT yet wired into any actual
/// input/output write path (spawn_with_def, submit_result, etc.) — doing that well needs a real
/// size-threshold policy decision (what counts as "too big," which of the four PayloadTypes
/// actually get checked) this pass deliberately didn't guess at, same reasoning DO_WHILE/
/// FORK_JOIN_DYNAMIC's orchestrator wiring was deferred for in Phase 3. A real integration is
/// the next thing to build here, not a silently-forgotten gap.
class IExternalPayloadStorage
{
public:
    virtual ~IExternalPayloadStorage() = default;
    IExternalPayloadStorage() = default;
    IExternalPayloadStorage(const IExternalPayloadStorage&) = delete;
    IExternalPayloadStorage& operator=(const IExternalPayloadStorage&) = delete;
    IExternalPayloadStorage(IExternalPayloadStorage&&) = delete;
    IExternalPayloadStorage& operator=(IExternalPayloadStorage&&) = delete;

    /**
     * @brief Writes a payload out to external storage.
     * @param type which category of payload this is.
     * @param data the raw payload bytes (already serialized by the caller).
     * @param callback gets the stored payload's reference path/URI on success, `""` on failure
     * — that reference is what a caller stashes in the owning row instead of the payload
     * itself.
     */
    virtual void
    write(PayloadType type, std::string_view data, shared::QueryReadFn&& callback) noexcept = 0;
    /**
     * @brief Reads a payload back by the reference `write()` returned.
     * @param reference the reference path/URI returned by a prior `write()`.
     * @param callback gets the raw payload bytes on success, `""` on failure or if `reference`
     * doesn't resolve to anything.
     */
    virtual void read(std::string_view reference, shared::QueryReadFn&& callback) noexcept = 0;
};

} // namespace interfaces
