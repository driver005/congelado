module;

#include "c/extern/serde.h"

export module cc_abi_runtime:serde;

import cc_abi_runtime_intern;

export namespace ice {

class SerdeRuntime {
 public:
  SerdeRuntime() : m_handle{nullptr} {}
  explicit SerdeRuntime(TP_Serde* handle) : m_handle{handle} {}

  TF_Status* invoke_encode(const TF_TString* value_json, TF_TString** out_encoded) const {
    return (m_handle && m_handle->encode_cb) ? m_handle->encode_cb(m_handle->ext, value_json, out_encoded) : nullptr;
  }
  TF_Status* invoke_decode(const TF_TString* data, TF_TString** out_json) const {
    return (m_handle && m_handle->decode_cb) ? m_handle->decode_cb(m_handle->ext, data, out_json) : nullptr;
  }

  StringRuntime get_content_type() const { return m_handle ? StringRuntime{&m_handle->content_type} : StringRuntime{}; }
  StringRuntime get_format_name() const { return m_handle ? StringRuntime{&m_handle->format_name} : StringRuntime{}; }

  // Underlying handle — pass directly to the C ABI
  TP_Serde *get_handle() const { return m_handle; }

 private:
  TP_Serde* m_handle;
};

}  // namespace ice
