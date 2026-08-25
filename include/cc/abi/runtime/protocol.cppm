module;

#include "c/extern/protocol.h"

export module cc_abi_runtime:protocol;

import cc_abi_runtime_intern;

export namespace ice {

// Non-owning wrapper around a `TP_Protocol_Server*` returned by ProtocolRuntime's
// invoke_create_server(). Drives the started server's lifecycle from the mainframe side.
class ProtocolServerRuntime {
 public:
  ProtocolServerRuntime() : m_handle{nullptr} {}
  explicit ProtocolServerRuntime(TP_Protocol_Server* handle) : m_handle{handle} {}

  void invoke_start(TF_Status* status) const {
    if (m_handle && m_handle->start_cb) {
      m_handle->start_cb(m_handle->ext, status);
    }
  }
  void invoke_stop(TF_Status* status) const {
    if (m_handle && m_handle->stop_cb) {
      m_handle->stop_cb(m_handle->ext, status);
    }
  }
  TF_Bool invoke_is_running() const {
    return (m_handle && m_handle->is_running_cb) ? m_handle->is_running_cb(m_handle->ext) : 0;
  }

  // Underlying handle — pass directly to the C ABI
  TP_Protocol_Server *get_handle() const { return m_handle; }

 private:
  TP_Protocol_Server* m_handle;
};

class ProtocolRuntime {
 public:
  ProtocolRuntime() : m_handle{nullptr} {}
  explicit ProtocolRuntime(TP_Protocol* handle) : m_handle{handle} {}

  ProtocolServerRuntime invoke_create_server(TF_Status* status) const {
    if (m_handle && m_handle->create_server_cb) {
      return ProtocolServerRuntime{m_handle->create_server_cb(m_handle->ext, status)};
    }
    return ProtocolServerRuntime{};
  }

  StringRuntime get_name() const { return m_handle ? StringRuntime{&m_handle->name} : StringRuntime{}; }
  StringRuntime get_bind_host() const { return m_handle ? StringRuntime{&m_handle->bind_host} : StringRuntime{}; }
  uint16_t get_bind_port() const { return m_handle ? m_handle->bind_port : 0; }
  StringRuntime get_tls_cert() const { return m_handle ? StringRuntime{&m_handle->tls_cert} : StringRuntime{}; }
  StringRuntime get_tls_key() const { return m_handle ? StringRuntime{&m_handle->tls_key} : StringRuntime{}; }

  // Underlying handle — pass directly to the C ABI
  TP_Protocol *get_handle() const { return m_handle; }

 private:
  TP_Protocol* m_handle;
};

}  // namespace ice
