module;

#include "c/extern/otel/meter.h"

export module cc_abi_builder_otel:meter;

import cc_abi_builder_intern;

import :counter;
import :histogram;

export namespace ice {

class MeterBuilder {
 public:
  MeterBuilder() : m_handle{TP_OtelMeterNew()} {}
  ~MeterBuilder() { TP_OtelMeterDelete(m_handle); }

  MeterBuilder(const MeterBuilder&) = delete;
  MeterBuilder& operator=(const MeterBuilder&) = delete;

  MeterBuilder(MeterBuilder&& other) noexcept : m_handle{other.m_handle} { other.m_handle = nullptr; }
  MeterBuilder& operator=(MeterBuilder&& other) noexcept {

    if (this != &other) {
      TP_OtelMeterDelete(m_handle);
      m_handle = other.m_handle;
      other.m_handle = nullptr;
    }
    return *this;

  }

  MeterBuilder& set_create_counter(TP_Otel_Meter_CreateCounterFn callback) {

    TP_OtelMeter_SetCreateCounterCallback(m_handle, callback);
    return *this;

  }

  MeterBuilder& set_create_histogram(TP_Otel_Meter_CreateHistogramFn callback) {

    TP_OtelMeter_SetCreateHistogramCallback(m_handle, callback);
    return *this;

  }

  // Underlying handle — pass directly to the C ABI
  TP_Otel_Meter *get_handle() { return m_handle; }
  const TP_Otel_Meter *get_handle() const { return m_handle; }

 private:
  TP_Otel_Meter* m_handle;
};

}  // namespace ice
