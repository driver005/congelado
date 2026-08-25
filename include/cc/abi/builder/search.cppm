module;

#include "c/extern/search.h"

export module cc_abi_builder:search;

import cc_abi_builder_intern;


export namespace ice {

class SearchBuilder {
 public:
  SearchBuilder() : m_handle{TP_SearchNew()} {}
  ~SearchBuilder() { TP_SearchDelete(m_handle); }

  SearchBuilder(const SearchBuilder&) = delete;
  SearchBuilder& operator=(const SearchBuilder&) = delete;

  SearchBuilder(SearchBuilder&& other) noexcept : m_handle{other.m_handle} { other.m_handle = nullptr; }
  SearchBuilder& operator=(SearchBuilder&& other) noexcept {
    if (this != &other) {
      TP_SearchDelete(m_handle);
      m_handle = other.m_handle;
      other.m_handle = nullptr;
    }
    return *this;
  }

  SearchBuilder& set_index(TP_Search_IndexFn callback) {
    TP_Search_SetIndexCallback(m_handle, callback);
    return *this;
  }
  SearchBuilder& set_remove(TP_Search_RemoveFn callback) {
    TP_Search_SetRemoveCallback(m_handle, callback);
    return *this;
  }
  SearchBuilder& set_search(TP_Search_SearchFn callback) {
    TP_Search_SetSearchCallback(m_handle, callback);
    return *this;
  }

  StringBuilder get_name() { return StringBuilder{&m_handle->backend_name}; }

  // Underlying handle — pass directly to the C ABI
  TP_Search *get_handle() { return m_handle; }
  const TP_Search *get_handle() const { return m_handle; }

 private:
  TP_Search* m_handle;
};

}  // namespace ice
