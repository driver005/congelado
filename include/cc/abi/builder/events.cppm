module;

#include "c/extern/events.h"

export module cc_abi_builder:events;

import cc_abi_builder_intern;


export namespace ice {

class EventsBuilder {
 public:
  EventsBuilder() : m_handle{TP_EventsNew()} {}
  ~EventsBuilder() { TP_EventsDelete(m_handle); }

  EventsBuilder(const EventsBuilder&) = delete;
  EventsBuilder& operator=(const EventsBuilder&) = delete;

  EventsBuilder(EventsBuilder&& other) noexcept : m_handle{other.m_handle} { other.m_handle = nullptr; }
  EventsBuilder& operator=(EventsBuilder&& other) noexcept {

    if (this != &other) {
      TP_EventsDelete(m_handle);
      m_handle = other.m_handle;
      other.m_handle = nullptr;
    }
    return *this;

  }

  EventsBuilder& set_publish(TP_Events_PublishFn callback) {

    TP_Events_SetPublishCallback(m_handle, callback);
    return *this;

  }

  StringBuilder get_name() { return StringBuilder{&m_handle->name}; }

  // Underlying handle — pass directly to the C ABI
  TP_Events *get_handle() { return m_handle; }
  const TP_Events *get_handle() const { return m_handle; }

 private:
  TP_Events* m_handle;
};

}  // namespace ice
