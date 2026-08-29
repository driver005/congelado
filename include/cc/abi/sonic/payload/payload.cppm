module;

#include "c/extern/payload/payload.h"

export module cc_abi_sonic_payload;

import std;
import cc_abi_sonic_intern;
import cc_abi_primitives;
import cc_abi_sonic_registration;

namespace ice::sonic::detail {
inline TF_Payload_CompletionFn to_c(ice::builder::CompletionFn fn) noexcept {
    static_assert(sizeof(ice::builder::CompletionFn) == sizeof(TF_Payload_CompletionFn));
    return std::bit_cast<TF_Payload_CompletionFn>(fn);
}
} // namespace ice::sonic::detail
export namespace ice::sonic {

// Runtime — the mainframe-facing payload handle. Same in-process/cross-plugin duality as
// ice::sonic::Cache and ice::sonic::Generator.
class Payload : public ice::sonic::Runtime<Payload, TF_Payload, /*PassNameToFactory=*/true>
{
public:
    static constexpr std::string_view domain_name = "payload";

    std::expected<void, ice::Status> write(
        ice::builder::PayloadType type,
        const ice::String& data,
        ice::builder::CompletionFn completion,
        void* cb_user_data
    )
    {
        ice::Status status;
        this->m_ops->write(this->get_handle(), to_raw_type(type), data.get_handle(),
            detail::to_c(completion), cb_user_data, status.get_handle()
        );
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    std::expected<void, ice::Status> read(
        const ice::String& reference,
        ice::builder::CompletionFn completion,
        void* cb_user_data
    )
    {
        ice::Status status;
        this->m_ops->read(this->get_handle(), reference.get_handle(),
            detail::to_c(completion), cb_user_data, status.get_handle()
        );
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    ice::String get_name() const
    {
        ice::String tf_name;
        this->m_ops->get_name(this->get_handle(), tf_name.get_handle());
        return std::move(tf_name);
    }

private:
    static TF_Payload_Type to_raw_type(ice::builder::PayloadType type)
    {
        switch (type) {
            case ice::builder::PayloadType::WorkflowInput:
                return TF_PAYLOAD_WORKFLOW_INPUT;
            case ice::builder::PayloadType::WorkflowOutput:
                return TF_PAYLOAD_WORKFLOW_OUTPUT;
            case ice::builder::PayloadType::TaskInput:
                return TF_PAYLOAD_TASK_INPUT;
            case ice::builder::PayloadType::TaskOutput:
                return TF_PAYLOAD_TASK_OUTPUT;
        }
        return TF_PAYLOAD_WORKFLOW_INPUT;
    }

public:
    explicit Payload(TF_Payload* ops, void* plugin_context) : Runtime(ops, plugin_context) {}
};

} // namespace ice::sonic
