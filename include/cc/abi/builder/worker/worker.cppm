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
// knowledge, mirrors ice::builder::Generator's role. A backend implements this
// directly and registers a factory function pointer into ice::sonic::Registration under
// type="worker".
//
// Exception contract: every member is noexcept — the std::expected return is the only
// failure channel; a throwing implementation fails fast at the ABI boundary.
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

    [[nodiscard]] virtual std::expected<ice::String, ice::Status> get_task_type() noexcept = 0;

    [[nodiscard]] virtual std::expected<ice::String, ice::Status>
    execute(const ice::String& input_json) noexcept = 0;

    [[nodiscard]] virtual std::expected<void, ice::Status> execute_async(
        const ice::String& input_json,
        TF_Worker_CompletionFn completion,
        void* cb_user_data
    ) noexcept = 0;

    [[nodiscard]] virtual std::expected<void, ice::Status>
    on_error(const ice::String& message) noexcept = 0;

    [[nodiscard]] virtual std::expected<void, ice::Status> on_released() noexcept = 0;

    virtual ice::String get_name() const noexcept = 0;

    static TF_Worker* get_generic_vtable()
    {
        static TF_Worker vtable = {
            .struct_size = TF_WORKER_STRUCT_SIZE,
            .destroy =
                [](void* plugin_context) noexcept
            {
                delete Worker::create(plugin_context);
            },
            .get_name =
                [](void* plugin_context, TF_String* out) noexcept
            {
                auto* self = Worker::create(plugin_context);
                auto name = self->get_name();
                name.to_c(out);
            },
            .get_task_type =
                [](void* plugin_context, TF_String* out, TF_Status* status) noexcept
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
                   TF_Status* status) noexcept
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
                   TF_Status* status) noexcept
            {
                auto* self = Worker::create(plugin_context);
                auto res =
                    self->execute_async(ice::String::create(input_json), completion, cb_user_data);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .on_error =
                [](void* plugin_context, const TF_TString* message, TF_Status* status) noexcept
            {
                auto* self = Worker::create(plugin_context);
                auto res = self->on_error(ice::String::create(message));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .on_released =
                [](void* plugin_context, TF_Status* status) noexcept
            {
                auto* self = Worker::create(plugin_context);
                auto res = self->on_released();
                if (!res) {
                    res.error().to_c(status);
                }
            }
        };
        return &vtable;
    }
};

} // namespace ice::builder
