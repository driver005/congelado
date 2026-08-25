module;

#include "c/extern/io/registration.h"

export module cc_abi_runtime_io:io;

import :request;
import :response;

export namespace ice {

// Non-owning wrapper around a `TP_IO*` received from a plugin. Holds the invoke logic split
// out of today's Io class (io/io.cppm), which mixed ownership (builder trait) and direct
// callback invocation (runtime trait) in one class.
class IoRuntime {
 public:
  IoRuntime() : m_handle{nullptr} {}
  explicit IoRuntime(TP_IO* handle) : m_handle{handle} {}

  RequestRuntime invoke_create_request(TF_Status* status) const {

    if (m_handle && m_handle->create_request_cb) {
      return RequestRuntime{m_handle->create_request_cb(m_handle->ext, status)};
    }
    return RequestRuntime{};

  }

  ResponseRuntime invoke_create_response(TF_Status* status) const {

    if (m_handle && m_handle->create_response_cb) {
      return ResponseRuntime{m_handle->create_response_cb(m_handle->ext, status)};
    }
    return ResponseRuntime{};

  }

  // Underlying handle — pass directly to the C ABI
  TP_IO *get_handle() const { return m_handle; }

 private:
  TP_IO* m_handle;
};

}  // namespace ice
