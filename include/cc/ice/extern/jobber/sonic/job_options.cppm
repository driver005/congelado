// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/jobber/options.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/jobber/options.h"

export module cc_ice_extern_jobber_sonic:job_options;

import std;

export namespace ice::sonic {

class JobOptions
{
public:
    JobOptions() noexcept
    {
        m_value.struct_size = sizeof(::TFJobOptions);
    }

    explicit JobOptions(const ::TFJobOptions& value) noexcept :
        m_value{value}
    {
    }

    JobOptions(
        const ::TF_String* cron_expression,
        int priority,
        int max_retries,
        std::int64_t timeout_ms
    ) noexcept
    {
        m_value.struct_size = sizeof(::TFJobOptions);
        m_value.cron_expression = cron_expression;
        m_value.priority = priority;
        m_value.max_retries = max_retries;
        m_value.timeout_ms = timeout_ms;
    }

    const ::TF_String* getCronExpression() const noexcept
    {
        return m_value.cron_expression;
    }

    void setCronExpression(const ::TF_String* value) noexcept
    {
        m_value.cron_expression = value;
    }

    int getPriority() const noexcept
    {
        return m_value.priority;
    }

    void setPriority(int value) noexcept
    {
        m_value.priority = value;
    }

    int getMaxRetries() const noexcept
    {
        return m_value.max_retries;
    }

    void setMaxRetries(int value) noexcept
    {
        m_value.max_retries = value;
    }

    std::int64_t getTimeoutMs() const noexcept
    {
        return m_value.timeout_ms;
    }

    void setTimeoutMs(std::int64_t value) noexcept
    {
        m_value.timeout_ms = value;
    }

    ::TFJobOptions* get_handle() noexcept
    {
        return &m_value;
    }

    const ::TFJobOptions* get_handle() const noexcept
    {
        return &m_value;
    }

private:
    ::TFJobOptions m_value{};
};

} // namespace ice::sonic
