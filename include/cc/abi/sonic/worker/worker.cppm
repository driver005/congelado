module;

#include "c/extern/worker/worker.h"

export module cc_abi_sonic_worker;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;
import cc_abi_sonic_registration;

export namespace ice::sonic {

// Runtime — the mainframe-facing worker handle. Same in-process/cross-plugin duality as
// ice::sonic::Cache and ice::sonic::Generator.
class Worker : public ice::sonic::Runtime<Worker, TF_Worker>
{
public:
    explicit Worker(TF_Worker* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "worker";

    [[nodiscard]] std::expected<ice::String, ice::Status> get_task_type() noexcept
    {
        ice::Status status;
        ice::String tf_task_type;
        m_ops->get_task_type(get_handle(), tf_task_type.get_handle(), status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return tf_task_type;
    }

    [[nodiscard]] std::expected<ice::String, ice::Status> execute(const ice::String& input_json) noexcept
    {
        ice::Status status;
        ice::String result;
        m_ops->execute(
            get_handle(),
            input_json.get_handle(),
            result.get_handle(),
            status.get_handle()
        );
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return result;
    }

    [[nodiscard]] std::expected<void, ice::Status> execute_async(
        const ice::String& input_json,
        TF_Worker_CompletionFn completion,
        void* cb_user_data
    ) noexcept
    {
        ice::Status status;
        m_ops->execute_async(
            get_handle(),
            input_json.get_handle(),
            completion,
            cb_user_data,
            status.get_handle()
        );
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> on_error(const ice::String& message) noexcept
    {
        ice::Status status;
        m_ops->on_error(get_handle(), message.get_handle(), status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> on_released() noexcept
    {
        ice::Status status;
        m_ops->on_released(get_handle(), status.get_handle());
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
