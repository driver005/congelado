module;

#include "c/extern/search.h"

export module cc_abi_runtime:search;

import cc_abi_runtime_intern;

export namespace ice {

// SearchQueryRuntime — owning value wrapper the mainframe builds up before calling
// SearchRuntime::invoke_search(); the plugin's search_cb only ever reads from it. Unlike
// CacheRuntime/WorkerRuntime/etc (non-owning views over a plugin-owned handle), this one owns
// its TF_TString fields because there's no allocator on the C ABI side for TF_Search_Query
// (no TF_Search_QueryNew) — the caller (mainframe) is the one who has to construct it.
class SearchQueryRuntime {
 public:
  SearchQueryRuntime() {
    m_query.struct_size = TF_SEARCH_QUERY_STRUCT_SIZE;
    m_query.ext = nullptr;
    TF_StringInit(&m_query.query);
    TF_StringInit(&m_query.free_text);
    m_query.start = 0;
    m_query.size = 10;
    TF_StringInit(&m_query.sort);
  }
  ~SearchQueryRuntime() {
    TF_StringDealloc(&m_query.query);
    TF_StringDealloc(&m_query.free_text);
    TF_StringDealloc(&m_query.sort);
  }

  SearchQueryRuntime(const SearchQueryRuntime&) = delete;
  SearchQueryRuntime& operator=(const SearchQueryRuntime&) = delete;
  SearchQueryRuntime(SearchQueryRuntime&&) = delete;
  SearchQueryRuntime& operator=(SearchQueryRuntime&&) = delete;

  SearchQueryRuntime& set_query(const char* data, size_t size) {
    TF_StringCopy(&m_query.query, data, size);
    return *this;
  }
  SearchQueryRuntime& set_free_text(const char* data, size_t size) {
    TF_StringCopy(&m_query.free_text, data, size);
    return *this;
  }
  SearchQueryRuntime& set_start(int64_t start) {
    m_query.start = start;
    return *this;
  }
  SearchQueryRuntime& set_size(int64_t size) {
    m_query.size = size;
    return *this;
  }
  SearchQueryRuntime& set_sort(const char* data, size_t size) {
    TF_StringCopy(&m_query.sort, data, size);
    return *this;
  }

  // Underlying handle — pass directly to the C ABI
  const TF_Search_Query *get_handle() const { return &m_query; }

 private:
  TF_Search_Query m_query;
};

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
