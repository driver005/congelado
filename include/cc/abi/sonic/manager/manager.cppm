module;

#include "c/extern/manager/manager.h"

export module cc_abi_sonic_manager;

import std;
import cc_abi_sonic_intern;
import cc_abi_primitives;
import cc_abi_sonic_registration;
export namespace ice::sonic {

// Runtime — the mainframe-facing worker-manager handle. Same in-process/cross-plugin duality as
// ice::sonic::Cache and ice::sonic::Generator.
class Manager : public ice::sonic::Runtime<Manager, TF_WorkerManager, /*PassNameToFactory=*/true>
{
public:
    static constexpr std::string_view domain_name = "manager";

    // Cross-plugin backends can't accept an owned in-process Worker across the C ABI — see the
    // interface's own doc comment. Only supported when this Runtime itself is in-process.
    std::expected<void, ice::Status>
    add_worker(std::unique_ptr<ice::builder::Worker> worker)
    {


        return std::unexpected{
            ice::Status{"WorkerManagerRuntime: add_worker is not supported across the "
                                "cross-plugin C ABI"}
        };
    }

    std::expected<void, ice::Status> spawn(const ice::String& spec_json
    )
    {


        ice::Status status;
        this->m_ops->spawn(this->get_handle(), spec_json.get_handle(), status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    std::expected<bool, ice::Status> start(const ice::String& worker_id
    )
    {


        ice::Status status;
        bool result =
            this->m_ops->start(this->get_handle(), worker_id.get_handle(), status.get_handle())
            != 0;
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return result;
    }

    std::expected<bool, ice::Status> stop(const ice::String& worker_id
    )
    {


        ice::Status status;
        bool result =
            this->m_ops->stop(this->get_handle(), worker_id.get_handle(), status.get_handle())
            != 0;
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return result;
    }

    std::expected<ice::String, ice::Status> list()
    {


        ice::Status status;
        ice::String result;
        this->m_ops->list(this->get_handle(), result.get_handle(), status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return result;
    }

    ice::String get_name() const
    {


        ice::String tf_name;
        this->m_ops->get_name(this->get_handle(), tf_name.get_handle());
        return std::move(tf_name);
    }

public:
    explicit Manager(TF_WorkerManager* ops, void* plugin_context) : Runtime(ops, plugin_context) {}
};

} // namespace ice::sonic
