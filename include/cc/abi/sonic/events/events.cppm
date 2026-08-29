module;

#include "c/extern/events/events.h"

export module cc_abi_sonic_events;

import std;
import cc_abi_sonic_intern;
import cc_abi_primitives;
import cc_abi_sonic_registration;
export namespace ice::sonic {

class Events : public ice::sonic::Runtime<Events, TF_Events, /*PassNameToFactory=*/true>
{
public:
    static constexpr std::string_view domain_name = "events";

    explicit Events(TF_Events* ops, void* plugin_context) 
        : Runtime(ops, plugin_context) {}

    std::expected<void, ice::Status> publish(
        const ice::String& event_name, const ice::String& payload_json
    )
    {
        ice::Status status;
        this->m_ops->publish(
            this->get_handle(), event_name.get_handle(), payload_json.get_handle(),
            status.get_handle()
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
};

} // namespace ice::sonic
