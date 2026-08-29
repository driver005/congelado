module;

#include "c/extern/orchestrator/orchestrator.h"

export module cc_abi_sonic_orchestrator;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;
import cc_abi_sonic_registration;

export namespace ice::sonic {

// Runtime — the mainframe-facing orchestrator handle. Same in-process/cross-plugin duality as
// ice::sonic::Cache and ice::sonic::Generator.
class Orchestrator : public ice::sonic::Runtime<Orchestrator, TF_Orchestrator>
{
public:
    explicit Orchestrator(TF_Orchestrator* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "orchestrator";

    [[nodiscard]] std::expected<void, ice::Status> start_workflow(
        const ice::String& def_name,
        const ice::String& variables_json,
        TF_Worker_CompletionFn completion,
        void* cb_user_data
    ) noexcept
    {
        ice::Status status;
        m_ops->start_workflow(
            get_handle(),
            def_name.get_handle(),
            variables_json.get_handle(),
            completion,
            cb_user_data,
            status.get_handle()
        );
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    pause(const ice::String& exec_id, TF_Worker_CompletionFn completion, void* cb_user_data) noexcept
    {
        ice::Status status;
        m_ops->pause(
            get_handle(),
            exec_id.get_handle(),
            completion,
            cb_user_data,
            status.get_handle()
        );
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    resume(const ice::String& exec_id, TF_Worker_CompletionFn completion, void* cb_user_data) noexcept
    {
        ice::Status status;
        m_ops->resume(
            get_handle(),
            exec_id.get_handle(),
            completion,
            cb_user_data,
            status.get_handle()
        );
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    terminate(const ice::String& exec_id, TF_Worker_CompletionFn completion, void* cb_user_data) noexcept
    {
        ice::Status status;
        m_ops->terminate(
            get_handle(),
            exec_id.get_handle(),
            completion,
            cb_user_data,
            status.get_handle()
        );
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> complete_task(
        const ice::String& exec_id,
        const ice::String& node_ref,
        bool success,
        const ice::String& output_json,
        TF_Worker_CompletionFn completion,
        void* cb_user_data
    ) noexcept
    {
        ice::Status status;
        m_ops->complete_task(
            get_handle(),
            exec_id.get_handle(),
            node_ref.get_handle(),
            success,
            output_json.get_handle(),
            completion,
            cb_user_data,
            status.get_handle()
        );
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    ice::String get_name() const noexcept
    {
        ice::String out;
        m_ops->get_name(get_handle(), out.get_handle());
        return out;
    }
};

} // namespace ice::sonic
