module;

#include "c/extern/otel/span.h"

export module cc_abi_builder_otel:span;

import cc_abi_builder_intern;

export namespace ice {

class SpanBuilder {
 public:
  SpanBuilder() : m_handle{TP_OtelSpanNew()} {}
  ~SpanBuilder() { TP_OtelSpanDelete(m_handle); }

  SpanBuilder(const SpanBuilder&) = delete;
  SpanBuilder& operator=(const SpanBuilder&) = delete;

  SpanBuilder(SpanBuilder&& other) noexcept : m_handle{other.m_handle} { other.m_handle = nullptr; }
  SpanBuilder& operator=(SpanBuilder&& other) noexcept {
    if (this != &other) {
      TP_OtelSpanDelete(m_handle);
      m_handle = other.m_handle;
      other.m_handle = nullptr;
    }
    return *this;
  }

  SpanBuilder& set_attribute(TP_Otel_Span_SetAttributeFn callback) {
    TP_OtelSpan_SetSetAttributeCallback(m_handle, callback);
    return *this;
  }

  SpanBuilder& set_status(TP_Otel_Span_SetStatusFn callback) {
    TP_OtelSpan_SetSetStatusCallback(m_handle, callback);
    return *this;
  }

  SpanBuilder& set_end(TP_Otel_Span_EndFn callback) {
    TP_OtelSpan_SetEndCallback(m_handle, callback);
    return *this;
  }

  // Underlying handle — pass directly to the C ABI
  TP_Otel_Span *get_handle() { return m_handle; }
  const TP_Otel_Span *get_handle() const { return m_handle; }

 private:
  TP_Otel_Span* m_handle;
};

}  // namespace ice
