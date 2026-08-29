module;

#include "c/extern/payload/payload.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"

export module cc_abi_builder_payload;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;

export namespace ice::builder {

enum class PayloadType
{
    WorkflowInput,
    WorkflowOutput,
    TaskInput,
    TaskOutput,
};

using CompletionFn = void (*)(const ice::String& result, void* user_data);

inline CompletionFn completion_from_c(TF_Payload_CompletionFn fn) noexcept {
    static_assert(sizeof(CompletionFn) == sizeof(TF_Payload_CompletionFn));
    return std::bit_cast<CompletionFn>(fn);
}
inline TF_Payload_CompletionFn completion_to_c(CompletionFn fn) noexcept {
    static_assert(sizeof(TF_Payload_CompletionFn) == sizeof(CompletionFn));
    return std::bit_cast<TF_Payload_CompletionFn>(fn);
}

// Abstract base class for a payload backend — pure interface, zero C-ABI/TF_* knowledge, mirrors
// ice::builder::Builder's role. A backend implements this directly and registers a
// factory function pointer into ice::sonic::RegistrationRuntime under type="payload".
class Payload
{
public:
    virtual ~Payload() = default;

    virtual std::expected<void, ice::Status> write(
        PayloadType type, const ice::String& data, CompletionFn completion, void* cb_user_data
    ) = 0;

    virtual std::expected<void, ice::Status>
    read(const ice::String& reference, CompletionFn completion, void* cb_user_data) = 0;

    virtual ice::String get_name() const = 0;

    TF_Payload* get_generic_vtable()
    {
        static TF_Payload vtable = {
            .struct_size = sizeof(TF_Payload),
            .destroy =
                [](void* ctx) {
                    delete ctx_as<Payload>(ctx);
                },
            .get_name =
                [](void* ctx, TF_String* out) {
                    auto* self = ctx_as<Payload>(ctx);
                    auto name = self->get_name();
                    name.to_c(out);
                },
            .write =
                [](void* ctx, TF_Payload_Type type, const TF_TString* data,
                   TF_Payload_CompletionFn completion, void* cb_user_data, TF_Status* status) {
                    auto* self = ctx_as<Payload>(ctx);
                    PayloadType cpp_type;
                    switch (type) {
                        case TF_PAYLOAD_WORKFLOW_INPUT:
                            cpp_type = PayloadType::WorkflowInput;
                            break;
                        case TF_PAYLOAD_WORKFLOW_OUTPUT:
                            cpp_type = PayloadType::WorkflowOutput;
                            break;
                        case TF_PAYLOAD_TASK_INPUT:
                            cpp_type = PayloadType::TaskInput;
                            break;
                        case TF_PAYLOAD_TASK_OUTPUT:
                            cpp_type = PayloadType::TaskOutput;
                            break;
                        default:
                            cpp_type = PayloadType::WorkflowInput;
                            break;
                    }
                    auto res = self->write(
                        cpp_type, ice::String::create(data),
                        completion_from_c(completion), cb_user_data
                    );
                    if (!res) {
                        res.error().to_c(status);
                    }
                },
            .read =
                [](void* ctx, const TF_TString* reference, TF_Payload_CompletionFn completion,
                   void* cb_user_data, TF_Status* status) {
                    auto* self = ctx_as<Payload>(ctx);
                    auto res = self->read(
                        ice::String::create(reference), completion_from_c(completion),
                        cb_user_data
                    );
                    if (!res) {
                        res.error().to_c(status);
                    }
                }
        };
        return &vtable;
    }
};

} // namespace ice::builder
