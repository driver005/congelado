// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/intern/tstring.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/intern/tstring.h"

export module cc_ice_intern_sonic:t_string_type;

import std;

export namespace ice::sonic {

class TStringType
{
public:
    enum Value : std::underlying_type_t<::TFTStringType>
    {
        k_small = ::TF_TSTR_SMALL,
        k_large = ::TF_TSTR_LARGE,
        k_offset = ::TF_TSTR_OFFSET,
        k_view = ::TF_TSTR_VIEW
    };

    constexpr TStringType() noexcept = default;

    constexpr TStringType(::TFTStringType value) noexcept :
        m_value{value}
    {
    }

    constexpr TStringType(Value value) noexcept :
        m_value{static_cast<::TFTStringType>(value)}
    {
    }

    constexpr ::TFTStringType get_value() const noexcept
    {
        return m_value;
    }

    constexpr operator ::TFTStringType() const noexcept
    {
        return m_value;
    }

private:
    ::TFTStringType m_value{};
};

} // namespace ice::sonic
