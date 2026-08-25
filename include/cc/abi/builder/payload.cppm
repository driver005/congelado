module;

#include "c/extern/payload.h"

export module cc_abi_builder:payload;


export namespace ice {

enum class PayloadBuilderType {
  WorkflowInput = TF_PAYLOAD_WORKFLOW_INPUT,
  WorkflowOutput = TF_PAYLOAD_WORKFLOW_OUTPUT,
  TaskInput = TF_PAYLOAD_TASK_INPUT,
  TaskOutput = TF_PAYLOAD_TASK_OUTPUT,
};

class PayloadBuilder {
 public:
  PayloadBuilder() : m_handle{TP_PayloadNew()} {}
  ~PayloadBuilder() { TP_PayloadDelete(m_handle); }

  PayloadBuilder(const PayloadBuilder&) = delete;
  PayloadBuilder& operator=(const PayloadBuilder&) = delete;

  PayloadBuilder(PayloadBuilder&& other) noexcept : m_handle{other.m_handle} { other.m_handle = nullptr; }
  PayloadBuilder& operator=(PayloadBuilder&& other) noexcept {
    if (this != &other) {
      TP_PayloadDelete(m_handle);
      m_handle = other.m_handle;
      other.m_handle = nullptr;
    }
    return *this;
  }

  PayloadBuilder& set_write(TP_Payload_WriteFn callback) {
    TP_Payload_SetWriteCallback(m_handle, callback);
    return *this;
  }
  PayloadBuilder& set_read(TP_Payload_ReadFn callback) {
    TP_Payload_SetReadCallback(m_handle, callback);
    return *this;
  }

  // Underlying handle — pass directly to the C ABI
  TP_Payload *get_handle() { return m_handle; }
  const TP_Payload *get_handle() const { return m_handle; }

 private:
  TP_Payload* m_handle;
};

}  // namespace ice
