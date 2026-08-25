module;

#include "c/extern/search.h"

export module cc_abi_runtime:search_runtime;

import cc_abi_runtime_intern;
import :search_query_runtime;

export namespace ice {

class SearchRuntime {
 public:
  SearchRuntime() : m_handle{nullptr} {}
  explicit SearchRuntime(TP_Search* handle) : m_handle{handle} {}

  void invoke_index(const TF_TString* collection, const TF_TString* id, const TF_TString* document_json,
                    TF_Search_CompletionFn completion, void* cb_user_data) const {

    if (m_handle && m_handle->index_cb) {
      m_handle->index_cb(m_handle->ext, collection, id, document_json, completion, cb_user_data);
    }

  }
  void invoke_remove(const TF_TString* collection, const TF_TString* id, TF_Search_CompletionFn completion,
                     void* cb_user_data) const {

    if (m_handle && m_handle->remove_cb) {
      m_handle->remove_cb(m_handle->ext, collection, id, completion, cb_user_data);
    }

  }
  void invoke_search(const TF_TString* collection, const SearchQueryRuntime& query, TF_Search_CompletionFn completion,
                     void* cb_user_data) const {

    if (m_handle && m_handle->search_cb) {
      m_handle->search_cb(m_handle->ext, collection, query.get_handle(), completion, cb_user_data);
    }

  }

  StringRuntime get_name() const { return m_handle ? StringRuntime{&m_handle->backend_name} : StringRuntime{}; }

  // Underlying handle — pass directly to the C ABI
  TP_Search *get_handle() const { return m_handle; }

 private:
  TP_Search* m_handle;
};

}  // namespace ice
