module;

#include "c/extern/database.h"

export module cc_abi_builder:database;

import cc_abi_builder_intern;


export namespace ice {

class DatabaseBuilder {
 public:
  DatabaseBuilder() : m_handle{TP_DatabaseNew()} {}
  ~DatabaseBuilder() { TP_DatabaseDelete(m_handle); }

  DatabaseBuilder(const DatabaseBuilder&) = delete;
  DatabaseBuilder& operator=(const DatabaseBuilder&) = delete;

  DatabaseBuilder(DatabaseBuilder&& other) noexcept : m_handle{other.m_handle} { other.m_handle = nullptr; }
  DatabaseBuilder& operator=(DatabaseBuilder&& other) noexcept {

    if (this != &other) {
      TP_DatabaseDelete(m_handle);
      m_handle = other.m_handle;
      other.m_handle = nullptr;
    }
    return *this;

  }

  DatabaseBuilder& set_is_connected(TP_Database_IsConnectedFn callback) {

    TP_Database_SetIsConnectedCallback(m_handle, callback);
    return *this;

  }
  DatabaseBuilder& set_query(TP_Database_QueryFn callback) {

    TP_Database_SetQueryCallback(m_handle, callback);
    return *this;

  }
  DatabaseBuilder& set_insert(TP_Database_InsertFn callback) {

    TP_Database_SetInsertCallback(m_handle, callback);
    return *this;

  }
  DatabaseBuilder& set_update(TP_Database_UpdateFn callback) {

    TP_Database_SetUpdateCallback(m_handle, callback);
    return *this;

  }
  DatabaseBuilder& set_remove(TP_Database_RemoveFn callback) {

    TP_Database_SetRemoveCallback(m_handle, callback);
    return *this;

  }

  StringBuilder get_name() { return StringBuilder{&m_handle->backend_name}; }

  // Underlying handle — pass directly to the C ABI
  TP_Database *get_handle() { return m_handle; }
  const TP_Database *get_handle() const { return m_handle; }

 private:
  TP_Database* m_handle;
};

}  // namespace ice
