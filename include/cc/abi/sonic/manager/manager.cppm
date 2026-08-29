module;

#include "c/extern/manager/manager.h"

export module cc_abi_sonic_manager;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;
import cc_abi_sonic_registration;
import cc_abi_sonic_worker;

export namespace ice::sonic {

// Runtime — the mainframe-facing worker-manager handle. Same in-process/cross-plugin duality as
// ice::sonic::Cache and ice::sonic::Generator.
class WorkerManager : public ice::sonic::Runtime<WorkerManager, TF_WorkerManager>
{
public:
    explicit WorkerManager(TF_WorkerManager* ops, void* plugin_context) :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "manager";

    // Cross-plugin backends can't accept an owned in-process Worker across the C ABI — see the
    // interface's own doc comment. Only supported when this Runtime itself is in-process.
    [[nodiscard]] std::expected<void, ice::Status>
    add_worker(std::unique_ptr<ice::sonic::Worker> worker)
    {
        return std::unexpected{ice::Status{"add_worker is not supported across the "
                                           "cross-plugin C ABI"}};
    }

    [[nodiscard]] std::expected<void, ice::Status> spawn(const ice::String& spec_json)
    {
        ice::Status status;
        m_ops->spawn(get_handle(), spec_json.get_handle(), status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<bool, ice::Status> start(const ice::String& worker_id)
    {
        ice::Status status;
        bool result = m_ops->start(get_handle(), worker_id.get_handle(), status.get_handle()) != 0;
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return result;
    }

    [[nodiscard]] std::expected<bool, ice::Status> stop(const ice::String& worker_id)
    {
        ice::Status status;
        bool result = m_ops->stop(get_handle(), worker_id.get_handle(), status.get_handle()) != 0;
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return result;
    }

    [[nodiscard]] std::expected<ice::String, ice::Status> list()
    {
        ice::Status status;
        ice::String result;
        m_ops->list(get_handle(), result.get_handle(), status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return result;
    }

    ice::String get_name() const
    {
        ice::String out;
        m_ops->get_name(get_handle(), out.get_handle());
        return out;
    }
};

} // namespace ice::sonic
