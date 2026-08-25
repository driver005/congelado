module;

#include "c/extern/manager.h"

export module cc_abi_runtime:worker_manager;

import cc_abi_runtime_intern;

export namespace ice {

class WorkerManagerRuntime {
 public:
  WorkerManagerRuntime() : m_handle{nullptr} {}
  explicit WorkerManagerRuntime(TP_WorkerManager* handle) : m_handle{handle} {}

  TF_Bool invoke_required() const {
    return (m_handle && m_handle->required_cb) ? m_handle->required_cb(m_handle->ext) : 0;
  }
  void invoke_add_worker(TP_Worker* worker) const {
    if (m_handle && m_handle->add_worker_cb) {
      m_handle->add_worker_cb(m_handle->ext, worker);
    }
  }
  void invoke_spawn(const TF_TString* spec_json, TF_Status* status) const {
    if (m_handle && m_handle->spawn_cb) {
      m_handle->spawn_cb(m_handle->ext, spec_json, status);
    }
  }
  TF_Bool invoke_start(const TF_TString* worker_id) const {
    return (m_handle && m_handle->start_cb) ? m_handle->start_cb(m_handle->ext, worker_id) : 0;
  }
  TF_Bool invoke_stop(const TF_TString* worker_id) const {
    return (m_handle && m_handle->stop_cb) ? m_handle->stop_cb(m_handle->ext, worker_id) : 0;
  }
  void invoke_list(TF_TString** out_list_json) const {
    if (m_handle && m_handle->list_cb) {
      m_handle->list_cb(m_handle->ext, out_list_json);
    }
  }

  StringRuntime get_name() const { return m_handle ? StringRuntime{&m_handle->backend_name} : StringRuntime{}; }

  // Underlying handle — pass directly to the C ABI
  TP_WorkerManager *get_handle() const { return m_handle; }

 private:
  TP_WorkerManager* m_handle;
};

}  // namespace ice
