module;

#include "c/extern/worker/worker.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"

export module cc_abi_builder_worker;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;

export namespace ice::builder {

using CompletionFn = void (*)(const ice::String& result, void* user_data);

// ice::builder::CompletionFn and TF_Worker_CompletionFn are function pointers with identical
// calling conventions: both pass (const TF_TString*, void*) in registers, and ice::String is a
// standard-layout single-member struct wrapping TF_TString — so String::create() can alias a
// TF_TString* as a const String& at zero cost. std::bit_cast makes this explicit and auditable.
inline CompletionFn completion_from_c(TF_Worker_CompletionFn fn) noexcept {
    static_assert(sizeof(CompletionFn) == sizeof(TF_Worker_CompletionFn));
    return std::bit_cast<CompletionFn>(fn);
}
inline TF_Worker_CompletionFn completion_to_c(CompletionFn fn) noexcept {
    static_assert(sizeof(TF_Worker_CompletionFn) == sizeof(CompletionFn));
    return std::bit_cast<TF_Worker_CompletionFn>(fn);
}

// Abstract base class for a worker (task executor) backend — pure interface, zero C-ABI/TF_*
// knowledge, mirrors ice::builder::Builder's role. A backend implements this
// directly and registers a factory function pointer into ice::sonic::RegistrationRuntime under
// type="worker".
class Worker
{
public:
    virtual ~Worker() = default;

    virtual std::expected<ice::String, ice::Status> get_task_type() = 0;

    virtual std::expected<ice::String, ice::Status> execute(const ice::String& input_json) = 0;

    virtual std::expected<void, ice::Status>
    execute_async(const ice::String& input_json, CompletionFn completion, void* cb_user_data) = 0;

    virtual std::expected<void, ice::Status> on_error(const ice::String& message) = 0;

    virtual std::expected<void, ice::Status> on_released() = 0;

    virtual ice::String get_name() const = 0;

    TF_Worker* get_generic_vtable()
    {
        static TF_Worker vtable = {
            .struct_size = sizeof(TF_Worker),
            .get_name =
                [](void* ctx, TF_String* out) {
                    auto* self = ctx_as<Worker>(ctx);
                    auto name = self->get_name();
                    name.to_c(out);
                },
            .get_task_type =
                [](void* ctx, TF_String* out, TF_Status* status) {
                    auto res = ctx_as<Worker>(ctx)->get_task_type();
                    if (!res) { res.error().to_c(status); return; }
                    res->to_c(out);
                },
            .execute =
                [](void* ctx, const TF_TString* input_json, TF_String* out_result_json,
                   TF_Status* status) {
                    auto res = ctx_as<Worker>(ctx)->execute(ice::String::create(input_json));
                    if (!res) { res.error().to_c(status); return; }
                    res->to_c(out_result_json);
                },
            .execute_async =
                [](void* ctx, const TF_TString* input_json, TF_Worker_CompletionFn completion,
                   void* cb_user_data, TF_Status* status) {
                    auto* self = ctx_as<Worker>(ctx);
                    auto res = self->execute_async(
                        ice::String::create(input_json), completion_from_c(completion),
                        cb_user_data
                    );
                    if (!res) {
                        res.error().to_c(status);
                    }
                },
            .on_error =
                [](void* ctx, const TF_TString* message, TF_Status* status) {
                    auto* self = ctx_as<Worker>(ctx);
                    auto res = self->on_error(ice::String::create(message));
                    if (!res) {
                        res.error().to_c(status);
                    }
                },
            .on_released =
                [](void* ctx, TF_Status* status) {
                    auto* self = ctx_as<Worker>(ctx);
                    auto res = self->on_released();
                    if (!res) {
                        res.error().to_c(status);
                    }
                },
            .destroy =
                [](void* ctx) {
                    delete ctx_as<Worker>(ctx);
                }
        };
        return &vtable;
    }
};

} // namespace ice::builder
