module;

#include "c/extern/orchestrator/orchestrator.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"
#include "c/intern/tf_bool.h"

export module cc_abi_builder_orchestrator;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;
import cc_abi_builder_worker;


export namespace ice::builder {

// Abstract base class for a workflow-orchestrator backend — pure interface, zero C-ABI/TF_*
// knowledge, mirrors ice::builder::Builder's role. A backend implements this
// directly and registers a factory function pointer into ice::sonic::RegistrationRuntime under
// type="orchestrator". Reuses ice::builder::CompletionFn — task completion is reported
// the same way a worker's own async operations are.
class Orchestrator
{
public:
    virtual ~Orchestrator() = default;


    virtual std::expected<void, ice::Status> start_workflow(
        const ice::String& def_name,
        const ice::String& variables_json,
        ice::builder::CompletionFn completion,
        void* cb_user_data
    ) = 0;

    virtual std::expected<void, ice::Status> pause(
        const ice::String& exec_id, ice::builder::CompletionFn completion,
        void* cb_user_data
    ) = 0;

    virtual std::expected<void, ice::Status> resume(
        const ice::String& exec_id, ice::builder::CompletionFn completion,
        void* cb_user_data
    ) = 0;

    virtual std::expected<void, ice::Status> terminate(
        const ice::String& exec_id, ice::builder::CompletionFn completion,
        void* cb_user_data
    ) = 0;

    virtual std::expected<void, ice::Status> complete_task(
        const ice::String& exec_id,
        const ice::String& node_ref,
        bool success,
        const ice::String& output_json,
        ice::builder::CompletionFn completion,
        void* cb_user_data
    ) = 0;

    virtual ice::String get_name() const = 0;

    TF_Orchestrator* get_generic_vtable() {
        static TF_Orchestrator vtable = {
            .struct_size = sizeof(TF_Orchestrator),
            .destroy = [](void* ctx) {
                delete ctx_as<Orchestrator>(ctx);
            },
            .get_name = [](void* ctx, TF_String* out) {
                auto* self = ctx_as<Orchestrator>(ctx);
                auto name = self->get_name();
                name.to_c(out);
            },.start_workflow = [](void* ctx, const TF_TString* def_name, const TF_TString* variables_json, TF_Worker_CompletionFn completion, void* cb_user_data, TF_Status* status) {
                auto* self = ctx_as<Orchestrator>(ctx);
                auto res = self->start_workflow(
                    ice::String::create(def_name),
                    ice::String::create(variables_json),
                    ice::builder::completion_from_c(completion), cb_user_data
                );
                if (!res) res.error().to_c(status);
            },
            .pause = [](void* ctx, const TF_TString* exec_id, TF_Worker_CompletionFn completion, void* cb_user_data, TF_Status* status) {
                auto* self = ctx_as<Orchestrator>(ctx);
                auto res = self->pause(
                    ice::String::create(exec_id),
                    ice::builder::completion_from_c(completion), cb_user_data
                );
                if (!res) res.error().to_c(status);
            },
            .resume = [](void* ctx, const TF_TString* exec_id, TF_Worker_CompletionFn completion, void* cb_user_data, TF_Status* status) {
                auto* self = ctx_as<Orchestrator>(ctx);
                auto res = self->resume(
                    ice::String::create(exec_id),
                    ice::builder::completion_from_c(completion), cb_user_data
                );
                if (!res) res.error().to_c(status);
            },
            .terminate = [](void* ctx, const TF_TString* exec_id, TF_Worker_CompletionFn completion, void* cb_user_data, TF_Status* status) {
                auto* self = ctx_as<Orchestrator>(ctx);
                auto res = self->terminate(
                    ice::String::create(exec_id),
                    ice::builder::completion_from_c(completion), cb_user_data
                );
                if (!res) res.error().to_c(status);
            },
            .complete_task = [](void* ctx, const TF_TString* exec_id, const TF_TString* node_ref, TF_Bool success, const TF_TString* output_json, TF_Worker_CompletionFn completion, void* cb_user_data, TF_Status* status) {
                auto* self = ctx_as<Orchestrator>(ctx);
                auto res = self->complete_task(
                    ice::String::create(exec_id),
                    ice::String::create(node_ref),
                    success != 0,
                    ice::String::create(output_json),
                    ice::builder::completion_from_c(completion), cb_user_data
                );
                if (!res) res.error().to_c(status);
            }
        };
        return &vtable;
    }
};

} // namespace ice::builder
