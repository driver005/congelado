module;

#include "c/extern/manager/manager.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"

export module cc_abi_builder_manager;

import std;
import cc_abi_builder_worker;
import cc_abi_primitives;
import cc_abi_sonic_intern;

export namespace ice::builder {

// Abstract base class for a worker-manager backend — pure interface, zero C-ABI/TF_* knowledge,
// mirrors ice::builder::Generator's role. A backend implements this directly and
// registers a factory function pointer into ice::sonic::Registration under
// type="manager".
class WorkerManager
{
public:
    // Recover the WorkerManager instance from the opaque void* context slot that every
    // C vtable callback receives.  Named accessor so the cast intent is explicit
    // at the call site and the static_cast appears exactly once, here.
    static WorkerManager* create(void* ctx) noexcept
    {
        return static_cast<WorkerManager*>(ctx);
    }

    virtual ~WorkerManager() = default;


    // Takes ownership of an in-process worker. Cross-plugin C-ABI callers can't support this —
    // there's no way to move a C++ Worker object across a .so boundary — see
    // ice::sonic::WorkerManager::add_worker's documented limitation.
    [[nodiscard]] virtual std::expected<void, ice::Status>
    add_worker(std::unique_ptr<ice::builder::Worker> worker) noexcept = 0;

    [[nodiscard]] virtual std::expected<void, ice::Status> spawn(const ice::String& spec_json) noexcept = 0;

    [[nodiscard]] virtual std::expected<bool, ice::Status> start(const ice::String& worker_id) noexcept = 0;

    [[nodiscard]] virtual std::expected<bool, ice::Status> stop(const ice::String& worker_id) noexcept = 0;

    [[nodiscard]] virtual std::expected<ice::String, ice::Status> list() noexcept = 0;

    virtual ice::String get_name() const noexcept = 0;

    static TF_WorkerManager* get_generic_vtable()
    {
        static TF_WorkerManager vtable = {
            .struct_size = TF_WORKER_MANAGER_STRUCT_SIZE,
            .destroy =
                [](void* plugin_context) noexcept
            {
                delete WorkerManager::create(plugin_context);
            },
            .get_name =
                [](void* plugin_context, TF_String* out) noexcept
            {
                auto* self = WorkerManager::create(plugin_context);
                auto name = self->get_name();
                name.to_c(out);
            },
            .spawn =
                [](void* plugin_context, const TF_TString* spec_json, TF_Status* status) noexcept
            {
                auto* self = WorkerManager::create(plugin_context);
                auto res = self->spawn(ice::String::create(spec_json));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .start =
                [](void* plugin_context, const TF_TString* worker_id, TF_Status* status) noexcept -> bool
            {
                auto* self = WorkerManager::create(plugin_context);
                auto res = self->start(ice::String::create(worker_id));
                if (!res) {
                    res.error().to_c(status);
                    return 0;
                }
                return *res ? 1 : 0;
            },
            .stop =
                [](void* plugin_context, const TF_TString* worker_id, TF_Status* status) noexcept -> bool
            {
                auto* self = WorkerManager::create(plugin_context);
                auto res = self->stop(ice::String::create(worker_id));
                if (!res) {
                    res.error().to_c(status);
                    return 0;
                }
                return *res ? 1 : 0;
            },
            .list =
                [](void* plugin_context, TF_String* out_list_json, TF_Status* status) noexcept
            {
                auto* self = WorkerManager::create(plugin_context);
                auto res = self->list();
                if (!res) {
                    res.error().to_c(status);
                    return;
                }
                res->to_c(out_list_json);
            }
        };
        return &vtable;
    }
};

} // namespace ice::builder
