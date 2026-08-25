module;

#include "c/extern/otel/counter.h"

export module cc_abi_builder_otel:counter;

import cc_abi_builder_intern;

export namespace ice {

class CounterBuilder {
 public:
  CounterBuilder() : m_handle{TP_OtelCounterNew()} {}
  ~CounterBuilder() { TP_OtelCounterDelete(m_handle); }

  CounterBuilder(const CounterBuilder&) = delete;
  CounterBuilder& operator=(const CounterBuilder&) = delete;

  CounterBuilder(CounterBuilder&& other) noexcept : m_handle{other.m_handle} { other.m_handle = nullptr; }
  CounterBuilder& operator=(CounterBuilder&& other) noexcept {
    if (this != &other) {
      TP_OtelCounterDelete(m_handle);
      m_handle = other.m_handle;
      other.m_handle = nullptr;
    }
    return *this;
  }

  CounterBuilder& set_add(TP_Otel_Counter_AddFn callback) {
    TP_OtelCounter_SetAddCallback(m_handle, callback);
    return *this;
  }

  // Underlying handle — pass directly to the C ABI
  TP_Otel_Counter *get_handle() { return m_handle; }
  const TP_Otel_Counter *get_handle() const { return m_handle; }

 private:
  TP_Otel_Counter* m_handle;
};

}  // namespace ice
