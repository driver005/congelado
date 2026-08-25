module;

#include "c/extern/otel/histogram.h"

export module cc_abi_runtime_otel:histogram;

export namespace ice {

// Non-owning wrapper around a `TP_Otel_Histogram*` (e.g. returned by MeterRuntime::invoke_create_histogram).
class HistogramRuntime {
 public:
  HistogramRuntime() : m_handle{nullptr} {}
  explicit HistogramRuntime(TP_Otel_Histogram* handle) : m_handle{handle} {}

  void invoke_record(double value) const {
    if (m_handle && m_handle->record_cb) {
      m_handle->record_cb(m_handle->ext, value);
    }
  }

  // Underlying handle — pass directly to the C ABI
  TP_Otel_Histogram *get_handle() const { return m_handle; }

 private:
  TP_Otel_Histogram* m_handle;
};

}  // namespace ice
