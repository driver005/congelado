module;

#include "c/extern/protocol.h"

export module cc_abi_builder:protocol;

import cc_abi_builder_intern;


export namespace ice {

class ProtocolBuilder {
 public:
  ProtocolBuilder() : m_handle{TP_ProtocolNew()} {}
  ~ProtocolBuilder() { TP_ProtocolDelete(m_handle); }

  ProtocolBuilder(const ProtocolBuilder&) = delete;
  ProtocolBuilder& operator=(const ProtocolBuilder&) = delete;

  ProtocolBuilder(ProtocolBuilder&& other) noexcept : m_handle{other.m_handle} { other.m_handle = nullptr; }
  ProtocolBuilder& operator=(ProtocolBuilder&& other) noexcept {

    if (this != &other) {
      TP_ProtocolDelete(m_handle);
      m_handle = other.m_handle;
      other.m_handle = nullptr;
    }
    return *this;

  }

  ProtocolBuilder& set_create_server(TP_Protocol_CreateServerFn callback) {

    TP_Protocol_SetCreateServerCallback(m_handle, callback);
    return *this;

  }

  StringBuilder get_name() { return StringBuilder{&m_handle->name}; }
  StringBuilder get_bind_host() { return StringBuilder{&m_handle->bind_host}; }
  uint16_t get_bind_port() { return m_handle->bind_port; }
  StringBuilder get_tls_cert() { return StringBuilder{&m_handle->tls_cert}; }
  StringBuilder get_tls_key() { return StringBuilder{&m_handle->tls_key}; }

  // Underlying handle — pass directly to the C ABI
  TP_Protocol *get_handle() { return m_handle; }
  const TP_Protocol *get_handle() const { return m_handle; }

 private:
  TP_Protocol* m_handle;
};

}  // namespace ice
