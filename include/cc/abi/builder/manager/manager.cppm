module;

#include "c/extern/manager/manager.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"
#include "c/intern/tf_bool.h"

export module cc_abi_builder_manager;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;
import cc_abi_builder_worker;


export namespace ice::builder {

// Abstract base class for a worker-manager backend — pure interface, zero C-ABI/TF_* knowledge,
// mirrors ice::builder::Builder's role. A backend implements this directly and
// registers a factory function pointer into ice::sonic::RegistrationRuntime under
// type="manager".
class WorkerManager
{
public:
    virtual ~WorkerManager() = default;


    // Takes ownership of an in-process worker. Cross-plugin C-ABI callers can't support this —
    // there's no way to move a C++ Worker object across a .so boundary — see
    // ice::sonic::Manager::add_worker's documented limitation.
    virtual std::expected<void, ice::Status>
    add_worker(std::unique_ptr<ice::builder::Worker> worker) = 0;

    virtual std::expected<void, ice::Status>
    spawn(const ice::String& spec_json) = 0;

    virtual std::expected<bool, ice::Status>
    start(const ice::String& worker_id) = 0;

    virtual std::expected<bool, ice::Status>
    stop(const ice::String& worker_id) = 0;

    virtual std::expected<ice::String, ice::Status> list() = 0;

    virtual ice::String get_name() const = 0;

    TF_WorkerManager* get_generic_vtable() {
        static TF_WorkerManager vtable = {
            .struct_size = sizeof(TF_WorkerManager),
            .destroy = [](void* ctx) {
                delete ctx_as<WorkerManager>(ctx);
            },
            .get_name = [](void* ctx, TF_String* out) {
                auto* self = ctx_as<WorkerManager>(ctx);
                auto name = self->get_name();
                name.to_c(out);
            },.spawn = [](void* ctx, const TF_TString* spec_json, TF_Status* status) {
                auto* self = ctx_as<WorkerManager>(ctx);
                auto res = self->spawn(ice::String::create(spec_json));
                if (!res) res.error().to_c(status);
            },
            .start = [](void* ctx, const TF_TString* worker_id, TF_Status* status) -> TF_Bool {
                auto* self = ctx_as<WorkerManager>(ctx);
                auto res = self->start(ice::String::create(worker_id));
                if (!res) {
                    res.error().to_c(status);
                    return 0;
                }
                return *res ? 1 : 0;
            },
            .stop = [](void* ctx, const TF_TString* worker_id, TF_Status* status) -> TF_Bool {
                auto* self = ctx_as<WorkerManager>(ctx);
                auto res = self->stop(ice::String::create(worker_id));
                if (!res) {
                    res.error().to_c(status);
                    return 0;
                }
                return *res ? 1 : 0;
            },
            .list = [](void* ctx, TF_String* out_list_json, TF_Status* status) {
                auto* self = ctx_as<WorkerManager>(ctx);
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
