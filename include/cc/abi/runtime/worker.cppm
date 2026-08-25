module;

#include "c/extern/worker.h"

export module cc_abi_runtime:worker;

import cc_abi_runtime_intern;

export namespace ice {

class WorkerRuntime {
 public:
  WorkerRuntime() : m_handle{nullptr} {}
  explicit WorkerRuntime(TP_Worker* handle) : m_handle{handle} {}

  const TF_TString* invoke_get_task_type() const {

    return (m_handle && m_handle->get_task_type_cb) ? m_handle->get_task_type_cb(m_handle->ext) : nullptr;

  }
  TF_Status* invoke_execute(const TF_TString* input_json, TF_TString** out_result_json) const {

    return (m_handle && m_handle->execute_cb) ? m_handle->execute_cb(m_handle->ext, input_json, out_result_json)
                                              : nullptr;

  }
  void invoke_execute_async(const TF_TString* input_json, TF_Worker_CompletionFn completion,
                            void* cb_user_data) const {

    if (m_handle && m_handle->execute_async_cb) {
      m_handle->execute_async_cb(m_handle->ext, input_json, completion, cb_user_data);
    }

  }
  void invoke_on_error(const TF_TString* message) const {

    if (m_handle && m_handle->on_error_cb) {
      m_handle->on_error_cb(m_handle->ext, message);
    }

  }
  void invoke_on_released() const {

    if (m_handle && m_handle->on_released_cb) {
      m_handle->on_released_cb(m_handle->ext);
    }

  }

  StringRuntime get_name() const { return m_handle ? StringRuntime{&m_handle->task_type} : StringRuntime{}; }

  // Underlying handle — pass directly to the C ABI
  TP_Worker *get_handle() const { return m_handle; }

 private:
  TP_Worker* m_handle;
};

}  // namespace ice
