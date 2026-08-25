module;

#include "c/extern/io/request.h"

export module cc_abi_builder_io:request;

import cc_abi_builder_intern;

export namespace ice {

class RequestBuilder {
 public:
  RequestBuilder() : m_handle{TP_IORequestNew()} {}
  ~RequestBuilder() { TP_IORequestDelete(m_handle); }

  RequestBuilder(const RequestBuilder&) = delete;
  RequestBuilder& operator=(const RequestBuilder&) = delete;

  RequestBuilder(RequestBuilder&& other) noexcept : m_handle{other.m_handle} { other.m_handle = nullptr; }
  RequestBuilder& operator=(RequestBuilder&& other) noexcept {
    if (this != &other) {
      TP_IORequestDelete(m_handle);
      m_handle = other.m_handle;
      other.m_handle = nullptr;
    }
    return *this;
  }

  RequestBuilder& set_get_method(TP_IO_Request_GetMethodFn callback) {
    TP_IORequest_SetGetMethodCallback(m_handle, callback);
    return *this;
  }
  RequestBuilder& set_get_path(TP_IO_Request_GetPathFn callback) {
    TP_IORequest_SetGetPathCallback(m_handle, callback);
    return *this;
  }
  RequestBuilder& set_set_header(TP_IO_Request_SetHeaderFn callback) {
    TP_IORequest_SetSetHeaderCallback(m_handle, callback);
    return *this;
  }
  RequestBuilder& set_set_body(TP_IO_Request_SetBodyFn callback) {
    TP_IORequest_SetSetBodyCallback(m_handle, callback);
    return *this;
  }

  // Underlying handle — pass directly to the C ABI
  TP_IO_Request *get_handle() { return m_handle; }
  const TP_IO_Request *get_handle() const { return m_handle; }

 private:
  TP_IO_Request* m_handle;
};

}  // namespace ice
