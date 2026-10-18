// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/intern/status.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/intern/status.h"

export module cc_ice_intern_sonic:code;

import std;

export namespace ice::sonic {

class Code
{
public:
    enum Value : std::underlying_type_t<::TF_Code>
    {
        k_ok = ::TF_OK,
        k_cancelled = ::TF_CANCELLED,
        k_unknown = ::TF_UNKNOWN,
        k_invalid_argument = ::TF_INVALID_ARGUMENT,
        k_deadline_exceeded = ::TF_DEADLINE_EXCEEDED,
        k_not_found = ::TF_NOT_FOUND,
        k_already_exists = ::TF_ALREADY_EXISTS,
        k_permission_denied = ::TF_PERMISSION_DENIED,
        k_unauthenticated = ::TF_UNAUTHENTICATED,
        k_resource_exhausted = ::TF_RESOURCE_EXHAUSTED,
        k_failed_precondition = ::TF_FAILED_PRECONDITION,
        k_aborted = ::TF_ABORTED,
        k_out_of_range = ::TF_OUT_OF_RANGE,
        k_unimplemented = ::TF_UNIMPLEMENTED,
        k_internal = ::TF_INTERNAL,
        k_unavailable = ::TF_UNAVAILABLE,
        k_data_loss = ::TF_DATA_LOSS
    };

    constexpr Code() noexcept = default;

    constexpr Code(::TF_Code value) noexcept :
        m_value{value}
    {
    }

    constexpr Code(Value value) noexcept :
        m_value{static_cast<::TF_Code>(value)}
    {
    }

    constexpr ::TF_Code get_value() const noexcept
    {
        return m_value;
    }

    constexpr operator ::TF_Code() const noexcept
    {
        return m_value;
    }

private:
    ::TF_Code m_value{};
};

} // namespace ice::sonic
