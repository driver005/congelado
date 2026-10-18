// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/jobber/job.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/jobber/job.h"

export module cc_ice_extern_jobber_sonic:job_status;

import std;

export namespace ice::sonic {

class JobStatus
{
public:
    enum Value : std::underlying_type_t<::TFJobStatus>
    {
        k_pending = ::TF_JOB_PENDING,
        k_running = ::TF_JOB_RUNNING,
        k_paused = ::TF_JOB_PAUSED,
        k_completed = ::TF_JOB_COMPLETED,
        k_failed = ::TF_JOB_FAILED,
        k_cancelled = ::TF_JOB_CANCELLED
    };

    constexpr JobStatus() noexcept = default;

    constexpr JobStatus(::TFJobStatus value) noexcept :
        m_value{value}
    {
    }

    constexpr JobStatus(Value value) noexcept :
        m_value{static_cast<::TFJobStatus>(value)}
    {
    }

    constexpr ::TFJobStatus get_value() const noexcept
    {
        return m_value;
    }

    constexpr operator ::TFJobStatus() const noexcept
    {
        return m_value;
    }

private:
    ::TFJobStatus m_value{};
};

} // namespace ice::sonic
