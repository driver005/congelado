module;

#include "c/extern/worker/worker.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"

export module cc_abi_builder_worker;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;

export namespace ice::builder {

// Abstract base class for a worker (task executor) backend — pure interface, zero C-ABI/TF_*
// knowledge, mirrors ice::builder::Builder's role. A backend implements this
// directly and registers a factory function pointer into ice::sonic::RegistrationRuntime under
// type="worker".
class Worker
{
public:
    // Recover the Worker instance from the opaque void* context slot that every
    // C vtable callback receives.  Named accessor so the cast intent is explicit
    // at the call site and the static_cast appears exactly once, here.
    static Worker* create(void* ctx) noexcept
    {
        return static_cast<Worker*>(ctx);
    }

    virtual ~Worker() = default;

    virtual [[nodiscard]] std::expected<ice::String, ice::Status> get_task_type() = 0;

    virtual [[nodiscard]] std::expected<ice::String, ice::Status>
    execute(const ice::String& input_json) = 0;

    virtual [[nodiscard]] std::expected<void, ice::Status> execute_async(
        const ice::String& input_json,
        TF_Worker_CompletionFn completion,
        void* cb_user_data
    ) = 0;

    virtual [[nodiscard]] std::expected<void, ice::Status> on_error(const ice::String& message) = 0;

    virtual [[nodiscard]] std::expected<void, ice::Status> on_released() = 0;

    virtual ice::String get_name() const = 0;

    static TF_Worker* get_generic_vtable()
    {
        static TF_Worker vtable = {
            .struct_size = sizeof(TF_Worker),
            .get_name =
                [](void* plugin_context, TF_String* out)
            {
                auto* self = Worker::create(plugin_context);
                auto name = self->get_name();
                name.to_c(out);
            },
            .get_task_type =
                [](void* plugin_context, TF_String* out, TF_Status* status)
            {
                auto res = Worker::create(plugin_context)->get_task_type();
                if (!res) {
                    res.error().to_c(status);
                    return;
                }
                res->to_c(out);
            },
            .execute =
                [](void* plugin_context,
                   const TF_TString* input_json,
                   TF_String* out_result_json,
                   TF_Status* status)
            {
                auto res = Worker::create(plugin_context)->execute(ice::String::create(input_json));
                if (!res) {
                    res.error().to_c(status);
                    return;
                }
                res->to_c(out_result_json);
            },
            .execute_async =
                [](void* plugin_context,
                   const TF_TString* input_json,
                   TF_Worker_CompletionFn completion,
                   void* cb_user_data,
                   TF_Status* status)
            {
                auto* self = Worker::create(plugin_context);
                auto res =
                    self->execute_async(ice::String::create(input_json), completion, cb_user_data);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .on_error =
                [](void* plugin_context, const TF_TString* message, TF_Status* status)
            {
                auto* self = Worker::create(plugin_context);
                auto res = self->on_error(ice::String::create(message));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .on_released =
                [](void* plugin_context, TF_Status* status)
            {
                auto* self = Worker::create(plugin_context);
                auto res = self->on_released();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .destroy =
                [](void* plugin_context)
            {
                delete Worker::create(plugin_context);
            }
        };
        return &vtable;
    }
};

} // namespace ice::builder
