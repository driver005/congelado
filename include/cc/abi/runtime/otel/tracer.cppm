module;

#include "c/extern/otel/tracer.h"

export module cc_abi_runtime_otel:tracer;

import :span;

export namespace ice {

// Non-owning wrapper around a `TP_Otel_Tracer*`.
class TracerRuntime {
 public:
  TracerRuntime() : m_handle{nullptr} {}
  explicit TracerRuntime(TP_Otel_Tracer* handle) : m_handle{handle} {}

  SpanRuntime invoke_start_span(const TF_TString* name, int kind, TF_Status* status) const {
    if (m_handle && m_handle->start_span_cb) {
      return SpanRuntime{m_handle->start_span_cb(m_handle->ext, name, kind, status)};
    }
    return SpanRuntime{};
  }

  // Underlying handle — pass directly to the C ABI
  TP_Otel_Tracer *get_handle() const { return m_handle; }

 private:
  TP_Otel_Tracer* m_handle;
};

}  // namespace ice
