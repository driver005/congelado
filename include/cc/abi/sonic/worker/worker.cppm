module;

#include "c/extern/worker/worker.h"

export module cc_abi_sonic_worker;

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

// Runtime — the mainframe-facing worker handle. Same in-process/cross-plugin duality as
// ice::sonic::Cache and ice::sonic::Generator.
class Worker : public ice::sonic::Runtime<Worker, TF_Worker, /*PassNameToFactory=*/true>
{
public:
    static constexpr std::string_view domain_name = "worker";

    std::expected<ice::String, ice::Status> get_task_type()
    {
        ice::Status status;
        ice::String tf_task_type;
        this->m_ops->get_task_type(this->get_handle(), tf_task_type.get_handle(), status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return std::move(tf_task_type);
    }

    std::expected<ice::String, ice::Status>
    execute(const ice::String& input_json)
    {
        ice::Status status;
        ice::String result;
        this->m_ops->execute(this->get_handle(), input_json.get_handle(), result.get_handle(), status.get_handle()
        );
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return result;
    }

    std::expected<void, ice::Status> execute_async(
        const ice::String& input_json,
        ice::builder::CompletionFn completion,
        void* cb_user_data
    )
    {
        ice::Status status;
        this->m_ops->execute_async(this->get_handle(), input_json.get_handle(),
            detail::to_c(completion), cb_user_data, status.get_handle()
        );
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    std::expected<void, ice::Status>
    on_error(const ice::String& message)
    {
        ice::Status status;
        this->m_ops->on_error(this->get_handle(), message.get_handle(), status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    std::expected<void, ice::Status> on_released()
    {
        ice::Status status;
        this->m_ops->on_released(this->get_handle(), status.get_handle());
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
    explicit Worker(TF_Worker* ops, void* plugin_context) : Runtime(ops, plugin_context) {}
};

} // namespace ice::sonic
