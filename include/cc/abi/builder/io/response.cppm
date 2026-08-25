module;

#include "c/extern/io/response.h"

export module cc_abi_builder_io:response;

import cc_abi_builder_intern;

export namespace ice {

class ResponseBuilder {
 public:
  ResponseBuilder() : m_handle{TP_IOResponseNew()} {}
  ~ResponseBuilder() { TP_IOResponseDelete(m_handle); }

  ResponseBuilder(const ResponseBuilder&) = delete;
  ResponseBuilder& operator=(const ResponseBuilder&) = delete;

  ResponseBuilder(ResponseBuilder&& other) noexcept : m_handle{other.m_handle} { other.m_handle = nullptr; }
  ResponseBuilder& operator=(ResponseBuilder&& other) noexcept {

    if (this != &other) {
      TP_IOResponseDelete(m_handle);
      m_handle = other.m_handle;
      other.m_handle = nullptr;
    }
    return *this;

  }

  ResponseBuilder& set_status(TP_IO_Response_SetStatusFn callback) {

    TP_IOResponse_SetSetStatusCallback(m_handle, callback);
    return *this;

  }
  ResponseBuilder& set_header(TP_IO_Response_SetHeaderFn callback) {

    TP_IOResponse_SetSetHeaderCallback(m_handle, callback);
    return *this;

  }
  ResponseBuilder& set_body(TP_IO_Response_SetBodyFn callback) {

    TP_IOResponse_SetSetBodyCallback(m_handle, callback);
    return *this;

  }

  // Underlying handle — pass directly to the C ABI
  TP_IO_Response *get_handle() { return m_handle; }
  const TP_IO_Response *get_handle() const { return m_handle; }

 private:
  TP_IO_Response* m_handle;
};

}  // namespace ice
