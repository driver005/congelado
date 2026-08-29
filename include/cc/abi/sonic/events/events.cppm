module;

#include "c/extern/events/events.h"

export module cc_abi_sonic_events;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;
import cc_abi_sonic_registration;

export namespace ice::sonic {

class Events : public ice::sonic::Runtime<Events, TF_Events>
{
public:
    explicit Events(TF_Events* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "events";

    [[nodiscard]] std::expected<void, ice::Status>
    publish(const ice::String& event_name, const ice::String& payload_json) noexcept
    {
        ice::Status status;
        m_ops->publish(
            get_handle(),
            event_name.get_handle(),
            payload_json.get_handle(),
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
