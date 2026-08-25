module;

#include "c/extern/otel/span.h"

export module cc_abi_runtime_otel:span;

import cc_abi_runtime_intern;

export namespace ice {

// Non-owning wrapper around a `TP_Otel_Span*` (e.g. returned by TracerRuntime::invoke_start_span).
class SpanRuntime {
 public:
  SpanRuntime() : m_handle{nullptr} {}
  explicit SpanRuntime(TP_Otel_Span* handle) : m_handle{handle} {}

  void invoke_set_attribute(const TF_TString* key, const TF_TString* value) const {

    if (m_handle && m_handle->set_attribute_cb) {
      m_handle->set_attribute_cb(m_handle->ext, key, value);
    }

  }
  void invoke_set_status(int status, const TF_TString* description) const {

    if (m_handle && m_handle->set_status_cb) {
      m_handle->set_status_cb(m_handle->ext, status, description);
    }

  }
  void invoke_end() const {

    if (m_handle && m_handle->end_cb) {
      m_handle->end_cb(m_handle->ext);
    }

  }

  // Underlying handle — pass directly to the C ABI
  TP_Otel_Span *get_handle() const { return m_handle; }

 private:
  TP_Otel_Span* m_handle;
};

}  // namespace ice
