module;

#include "c/extern/otel/tracer.h"

export module cc_abi_builder_otel:tracer;

import cc_abi_builder_intern;

export namespace ice {

class TracerBuilder {
 public:
  TracerBuilder() : m_handle{TP_OtelTracerNew()} {}
  ~TracerBuilder() { TP_OtelTracerDelete(m_handle); }

  TracerBuilder(const TracerBuilder&) = delete;
  TracerBuilder& operator=(const TracerBuilder&) = delete;

  TracerBuilder(TracerBuilder&& other) noexcept : m_handle{other.m_handle} { other.m_handle = nullptr; }
  TracerBuilder& operator=(TracerBuilder&& other) noexcept {
    if (this != &other) {
      TP_OtelTracerDelete(m_handle);
      m_handle = other.m_handle;
      other.m_handle = nullptr;
    }
    return *this;
  }

  TracerBuilder& set_start_span(TP_Otel_Tracer_StartSpanFn callback) {
    TP_OtelTracer_SetStartSpanCallback(m_handle, callback);
    return *this;
  }

  // Underlying handle — pass directly to the C ABI
  TP_Otel_Tracer *get_handle() { return m_handle; }
  const TP_Otel_Tracer *get_handle() const { return m_handle; }

 private:
  TP_Otel_Tracer* m_handle;
};

}  // namespace ice
