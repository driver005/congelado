module;

#include "c/extern/logger.h"

export module cc_abi_builder:logger;

import cc_abi_builder_intern;


export namespace ice {

enum class LogLevel {
  Debug = TF_LOGGER_DEBUG,
  Info = TF_LOGGER_INFO,
  Important = TF_LOGGER_IMPORTANT,
  Warning = TF_LOGGER_WARNING,
  Error = TF_LOGGER_ERROR,
  Fatal = TF_LOGGER_FATAL,
};

class LoggerBuilder {
 public:
  LoggerBuilder() : m_handle{TP_LoggerNew()} {}
  ~LoggerBuilder() { TP_LoggerDelete(m_handle); }

  LoggerBuilder(const LoggerBuilder&) = delete;
  LoggerBuilder& operator=(const LoggerBuilder&) = delete;

  LoggerBuilder(LoggerBuilder&& other) noexcept : m_handle{other.m_handle} { other.m_handle = nullptr; }
  LoggerBuilder& operator=(LoggerBuilder&& other) noexcept {

    if (this != &other) {
      TP_LoggerDelete(m_handle);
      m_handle = other.m_handle;
      other.m_handle = nullptr;
    }
    return *this;

  }

  LoggerBuilder& set_write(TP_Logger_WriteFn callback) {

    TP_Logger_SetWriteCallback(m_handle, callback);
    return *this;

  }
  LoggerBuilder& set_required(TP_Logger_RequiredFn callback) {

    TP_Logger_SetRequiredCallback(m_handle, callback);
    return *this;

  }

  StringBuilder get_name() { return StringBuilder{&m_handle->name}; }

  // Underlying handle — pass directly to the C ABI
  TP_Logger *get_handle() { return m_handle; }
  const TP_Logger *get_handle() const { return m_handle; }

 private:
  TP_Logger* m_handle;
};

}  // namespace ice
