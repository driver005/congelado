module;

#include "c/extern/events.h"

export module cc_abi_sonic:events;

import cc_abi_sonic_intern;

export namespace ice::sonic {

class EventsRuntime
{
public:
    EventsRuntime() :
        m_handle{nullptr}
    {
    }

    explicit EventsRuntime(TP_Events* handle) :
        m_handle{handle}
    {
    }

    void invoke_publish(const TF_TString* event_name, const TF_TString* payload_json) const
    {

        if (m_handle && m_handle->publish_cb) {
            m_handle->publish_cb(m_handle->ext, event_name, payload_json);
        }
    }

    StringRuntime get_name() const
    {
        return m_handle ? StringRuntime{&m_handle->name} : StringRuntime{};
    }

    // Underlying handle — pass directly to the C ABI
    TP_Events* get_handle() const
    {
        return m_handle;
    }

private:
    TP_Events* m_handle;
};

} // namespace ice::sonic
