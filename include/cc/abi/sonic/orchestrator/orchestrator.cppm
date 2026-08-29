module;

#include "c/extern/orchestrator/orchestrator.h"

export module cc_abi_sonic_orchestrator;

import std;
import cc_abi_sonic_intern;
import cc_abi_primitives;
import cc_abi_sonic_registration;

namespace ice::sonic::detail {
inline TF_Worker_CompletionFn to_c(ice::builder::CompletionFn fn) noexcept {
    static_assert(sizeof(ice::builder::CompletionFn) == sizeof(TF_Worker_CompletionFn));
    return std::bit_cast<TF_Worker_CompletionFn>(fn);
}
} // namespace ice::sonic::detail
export namespace ice::sonic {

// Runtime — the mainframe-facing orchestrator handle. Same in-process/cross-plugin duality as
// ice::sonic::Cache and ice::sonic::Generator.
class Orchestrator : public ice::sonic::Runtime<Orchestrator, TF_Orchestrator, /*PassNameToFactory=*/true>
{
public:
    static constexpr std::string_view domain_name = "orchestrator";

    std::expected<void, ice::Status> start_workflow(
        const ice::String& def_name,
        const ice::String& variables_json,
        ice::builder::CompletionFn completion,
        void* cb_user_data
    )
    {
        ice::Status status;
        this->m_ops->start_workflow(this->get_handle(), def_name.get_handle(), variables_json.get_handle(),
            detail::to_c(completion), cb_user_data, status.get_handle()
        );
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    std::expected<void, ice::Status> pause(
        const ice::String& exec_id, ice::builder::CompletionFn completion,
        void* cb_user_data
    )
    {
        ice::Status status;
        this->m_ops->pause(this->get_handle(), exec_id.get_handle(),
            detail::to_c(completion), cb_user_data, status.get_handle()
        );
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    std::expected<void, ice::Status> resume(
        const ice::String& exec_id, ice::builder::CompletionFn completion,
        void* cb_user_data
    )
    {
        ice::Status status;
        this->m_ops->resume(this->get_handle(), exec_id.get_handle(),
            detail::to_c(completion), cb_user_data, status.get_handle()
        );
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    std::expected<void, ice::Status> terminate(
        const ice::String& exec_id, ice::builder::CompletionFn completion,
        void* cb_user_data
    )
    {
        ice::Status status;
        this->m_ops->terminate(this->get_handle(), exec_id.get_handle(),
            detail::to_c(completion), cb_user_data, status.get_handle()
        );
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    std::expected<void, ice::Status> complete_task(
        const ice::String& exec_id,
        const ice::String& node_ref,
        bool success,
        const ice::String& output_json,
        ice::builder::CompletionFn completion,
        void* cb_user_data
    )
    {
        ice::Status status;
        this->m_ops->complete_task(this->get_handle(), exec_id.get_handle(), node_ref.get_handle(), success,
            output_json.get_handle(), detail::to_c(completion), cb_user_data, status.get_handle()
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

public:
    explicit Orchestrator(TF_Orchestrator* ops, void* plugin_context) : Runtime(ops, plugin_context) {}
};

} // namespace ice::sonic
