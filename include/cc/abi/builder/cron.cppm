module;

#include "c/extern/cron.h"

export module cc_abi_builder:cron;

import cc_abi_builder_intern;


export namespace ice {

class CronBuilder {
 public:
  CronBuilder() : m_handle{TP_CronNew()} {}
  ~CronBuilder() { TP_CronDelete(m_handle); }

  CronBuilder(const CronBuilder&) = delete;
  CronBuilder& operator=(const CronBuilder&) = delete;

  CronBuilder(CronBuilder&& other) noexcept : m_handle{other.m_handle} { other.m_handle = nullptr; }
  CronBuilder& operator=(CronBuilder&& other) noexcept {
    if (this != &other) {
      TP_CronDelete(m_handle);
      m_handle = other.m_handle;
      other.m_handle = nullptr;
    }
    return *this;
  }

  CronBuilder& set_required(TP_Cron_RequiredFn callback) {
    TP_Cron_SetRequiredCallback(m_handle, callback);
    return *this;
  }
  CronBuilder& set_validate(TP_Cron_ValidateFn callback) {
    TP_Cron_SetValidateCallback(m_handle, callback);
    return *this;
  }
  CronBuilder& set_next_after(TP_Cron_NextAfterFn callback) {
    TP_Cron_SetNextAfterCallback(m_handle, callback);
    return *this;
  }
  CronBuilder& set_upsert_job(TP_Cron_UpsertJobFn callback) {
    TP_Cron_SetUpsertJobCallback(m_handle, callback);
    return *this;
  }
  CronBuilder& set_remove_job(TP_Cron_RemoveJobFn callback) {
    TP_Cron_SetRemoveJobCallback(m_handle, callback);
    return *this;
  }

  StringBuilder get_name() { return StringBuilder{&m_handle->backend_name}; }

  // Underlying handle — pass directly to the C ABI
  TP_Cron *get_handle() { return m_handle; }
  const TP_Cron *get_handle() const { return m_handle; }

 private:
  TP_Cron* m_handle;
};

}  // namespace ice
