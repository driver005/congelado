module;

#include "c/extern/logger.h"

export module cc_abi_runtime:logger;

import cc_abi_runtime_intern;

export namespace ice {

class LoggerRuntime {
 public:
  LoggerRuntime() : m_handle{nullptr} {}
  explicit LoggerRuntime(TP_Logger* handle) : m_handle{handle} {}

  void invoke_write(TF_Logger_LogLevel level, const TF_TString* message) const {
    if (m_handle && m_handle->write_cb) {
      m_handle->write_cb(m_handle->ext, level, message);
    }
  }
  TF_Bool invoke_required() const {
    return (m_handle && m_handle->required_cb) ? m_handle->required_cb(m_handle->ext) : 0;
  }

  StringRuntime get_name() const { return m_handle ? StringRuntime{&m_handle->name} : StringRuntime{}; }

  // Underlying handle — pass directly to the C ABI
  TP_Logger *get_handle() const { return m_handle; }

 private:
  TP_Logger* m_handle;
};

}  // namespace ice
