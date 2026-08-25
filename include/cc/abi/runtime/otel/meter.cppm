module;

#include "c/extern/otel/meter.h"

export module cc_abi_runtime_otel:meter;

import cc_abi_runtime_intern;
import :counter;
import :histogram;

export namespace ice {

// Non-owning wrapper around a `TP_Otel_Meter*`.
class MeterRuntime {
 public:
  MeterRuntime() : m_handle{nullptr} {}
  explicit MeterRuntime(TP_Otel_Meter* handle) : m_handle{handle} {}

  CounterRuntime invoke_create_counter(const TF_TString* name, const TF_TString* description,
                                       const TF_TString* unit) const {
    if (m_handle && m_handle->create_counter_cb) {
      return CounterRuntime{m_handle->create_counter_cb(m_handle->ext, name, description, unit)};
    }
    return CounterRuntime{};
  }
  HistogramRuntime invoke_create_histogram(const TF_TString* name, const TF_TString* description,
                                           const TF_TString* unit) const {
    if (m_handle && m_handle->create_histogram_cb) {
      return HistogramRuntime{m_handle->create_histogram_cb(m_handle->ext, name, description, unit)};
    }
    return HistogramRuntime{};
  }

  // Underlying handle — pass directly to the C ABI
  TP_Otel_Meter *get_handle() const { return m_handle; }

 private:
  TP_Otel_Meter* m_handle;
};

}  // namespace ice
