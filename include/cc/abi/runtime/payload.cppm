module;

#include "c/extern/payload.h"

export module cc_abi_runtime:payload;


export namespace ice {

class PayloadRuntime {
 public:
  PayloadRuntime() : m_handle{nullptr} {}
  explicit PayloadRuntime(TP_Payload* handle) : m_handle{handle} {}

  void invoke_write(TF_Payload_Type type, const TF_TString* data, TF_Payload_CompletionFn completion,
                    void* cb_user_data) const {

    if (m_handle && m_handle->write_cb) {
      m_handle->write_cb(m_handle->ext, type, data, completion, cb_user_data);
    }

  }
  void invoke_read(const TF_TString* reference, TF_Payload_CompletionFn completion, void* cb_user_data) const {

    if (m_handle && m_handle->read_cb) {
      m_handle->read_cb(m_handle->ext, reference, completion, cb_user_data);
    }

  }

  // Underlying handle — pass directly to the C ABI
  TP_Payload *get_handle() const { return m_handle; }

 private:
  TP_Payload* m_handle;
};

}  // namespace ice
