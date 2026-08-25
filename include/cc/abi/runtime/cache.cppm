module;

#include "c/extern/cache.h"

export module cc_abi_runtime:cache;

import cc_abi_runtime_intern;

export namespace ice {

// Non-owning wrapper around a `TP_Cache*` handed to the mainframe after a plugin's
// TF_InitCache ran. Invokes the callbacks the plugin registered via CacheBuilder — mirrors
// CacheBuilder's set_* methods 1:1, but invokes instead of registers.
class CacheRuntime {
 public:
  CacheRuntime() : m_handle{nullptr} {}
  explicit CacheRuntime(TP_Cache* handle) : m_handle{handle} {}

  void invoke_get(const TF_TString* key, TF_Cache_CompletionFn completion, void* cb_user_data) const {

    if (m_handle && m_handle->get_cb) {
      m_handle->get_cb(m_handle->ext, key, completion, cb_user_data);
    }

  }
  void invoke_set(const TF_TString* key, const TF_TString* value, TF_Cache_CompletionFn completion,
                  void* cb_user_data) const {

    if (m_handle && m_handle->set_cb) {
      m_handle->set_cb(m_handle->ext, key, value, completion, cb_user_data);
    }

  }
  void invoke_remove(const TF_TString* key, TF_Cache_CompletionFn completion, void* cb_user_data) const {

    if (m_handle && m_handle->remove_cb) {
      m_handle->remove_cb(m_handle->ext, key, completion, cb_user_data);
    }

  }

  StringRuntime get_name() const { return m_handle ? StringRuntime{&m_handle->backend_name} : StringRuntime{}; }

  // Underlying handle — pass directly to the C ABI
  TP_Cache *get_handle() const { return m_handle; }

 private:
  TP_Cache* m_handle;
};

}  // namespace ice
