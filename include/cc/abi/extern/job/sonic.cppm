// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from c/extern/job/job.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "c/extern/job/job.h"

export module cc_abi_sonic_job;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;
import cc_abi_sonic_registration;

export namespace ice::sonic {

class Job : public ice::sonic::Runtime<Job, TF_JobOps>
{
public:
    explicit Job(TF_JobOps* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "job";

    [[nodiscard]] std::expected<void, ice::Status>
    execute(const ice::sonic::String& input, const ice::sonic::String& out_output) noexcept
    {
        ice::Status status;
        m_ops->execute(
            get_handle(),
            input.get_handle(),
            out_output.get_handle(),
            status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    submit(const ice::sonic::String& input, const TF_Job_Options* options) noexcept
    {
        ice::Status status;
        m_ops->submit(get_handle(), input.get_handle(), options, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> resubmit() noexcept
    {
        ice::Status status;
        m_ops->resubmit(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    wait(int64_t timeout_ms, TF_Job_ResultFn completion, void* user_data) noexcept
    {
        ice::Status status;
        m_ops->wait(get_handle(), timeout_ms, completion, user_data, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    on_complete(TF_Job_CompletionFn completion, void* user_data) noexcept
    {
        ice::Status status;
        m_ops->on_complete(get_handle(), completion, user_data, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    on_progress(TF_Job_ProgressFn progress, void* user_data) noexcept
    {
        ice::Status status;
        m_ops->on_progress(get_handle(), progress, user_data, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    get_status(TF_Job_StatusFn completion, void* user_data) noexcept
    {
        ice::Status status;
        m_ops->get_status(get_handle(), completion, user_data, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    get_result(TF_Job_ResultFn completion, void* user_data) noexcept
    {
        ice::Status status;
        m_ops->get_result(get_handle(), completion, user_data, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    get_history(const ice::sonic::Vector& out_transitions) noexcept
    {
        ice::Status status;
        m_ops->get_history(get_handle(), out_transitions.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    get_metrics(const ice::sonic::Map& out_metrics) noexcept
    {
        ice::Status status;
        m_ops->get_metrics(get_handle(), out_metrics.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    get_logs(const ice::sonic::Vector& out_lines) noexcept
    {
        ice::Status status;
        m_ops->get_logs(get_handle(), out_lines.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> get_options(TF_Job_Options* out_options) noexcept
    {
        ice::Status status;
        m_ops->get_options(get_handle(), out_options, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    update_options(const TF_Job_Options* options) noexcept
    {
        ice::Status status;
        m_ops->update_options(get_handle(), options, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> set_priority(int priority) noexcept
    {
        ice::Status status;
        m_ops->set_priority(get_handle(), priority, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    add_dependency(const ice::sonic::Job& depends_on) noexcept
    {
        ice::Status status;
        m_ops->add_dependency(get_handle(), depends_on.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    list_dependencies(const ice::sonic::Vector& out_job_ids) noexcept
    {
        ice::Status status;
        m_ops->list_dependencies(get_handle(), out_job_ids.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> pause() noexcept
    {
        ice::Status status;
        m_ops->pause(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> resume() noexcept
    {
        ice::Status status;
        m_ops->resume(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> cancel() noexcept
    {
        ice::Status status;
        m_ops->cancel(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> stop() noexcept
    {
        ice::Status status;
        m_ops->stop(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    signal(const ice::sonic::String& signal_name, const ice::sonic::String& payload) noexcept
    {
        ice::Status status;
        m_ops->signal(
            get_handle(),
            signal_name.get_handle(),
            payload.get_handle(),
            status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    checkpoint(TF_Job_AckFn completion, void* user_data) noexcept
    {
        ice::Status status;
        m_ops->checkpoint(get_handle(), completion, user_data, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> restore_checkpoint() noexcept
    {
        ice::Status status;
        m_ops->restore_checkpoint(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    complete(const ice::sonic::String& node_ref, const ice::sonic::String& output) noexcept
    {
        ice::Status status;
        m_ops->complete(
            get_handle(),
            node_ref.get_handle(),
            output.get_handle(),
            status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    create_task(const ice::sonic::String& node_ref, const ice::sonic::String& input) noexcept
    {
        ice::Status status;
        m_ops->create_task(
            get_handle(),
            node_ref.get_handle(),
            input.get_handle(),
            status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    get_task(const ice::sonic::String& node_ref) noexcept
    {
        ice::Status status;
        m_ops->get_task(get_handle(), node_ref.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    list_tasks(const ice::sonic::Vector& out_node_refs) noexcept
    {
        ice::Status status;
        m_ops->list_tasks(get_handle(), out_node_refs.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    cancel_task(const ice::sonic::String& node_ref) noexcept
    {
        ice::Status status;
        m_ops->cancel_task(get_handle(), node_ref.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    list(const ice::sonic::Map& filters, const ice::sonic::Vector& out_job_ids) noexcept
    {
        ice::Status status;
        m_ops->list(
            get_handle(),
            filters.get_handle(),
            out_job_ids.get_handle(),
            status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    ice::String get_name() const noexcept
    {
        ice::String result;
        m_ops->get_name(get_handle(), result.get_handle());
        return result;
    }
};

} // namespace ice::sonic
