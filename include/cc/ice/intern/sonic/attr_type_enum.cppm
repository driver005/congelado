// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/intern/attrtype.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/intern/attrtype.h"

export module cc_ice_intern_sonic:attr_type_enum;

import std;

export namespace ice::sonic {

class AttrTypeEnum
{
public:
    enum Value : std::underlying_type_t<::TFAttrTypeEnum>
    {
        k_string = ::TF_ATTR_STRING,
        k_int = ::TF_ATTR_INT,
        k_float = ::TF_ATTR_FLOAT,
        k_bool = ::TF_ATTR_BOOL,
        k_type = ::TF_ATTR_TYPE,
        k_shape = ::TF_ATTR_SHAPE,
        k_tensor = ::TF_ATTR_TENSOR,
        k_placeholder = ::TF_ATTR_PLACEHOLDER,
        k_func = ::TF_ATTR_FUNC
    };

    constexpr AttrTypeEnum() noexcept = default;

    constexpr AttrTypeEnum(::TFAttrTypeEnum value) noexcept :
        m_value{value}
    {
    }

    constexpr AttrTypeEnum(Value value) noexcept :
        m_value{static_cast<::TFAttrTypeEnum>(value)}
    {
    }

    constexpr ::TFAttrTypeEnum get_value() const noexcept
    {
        return m_value;
    }

    constexpr operator ::TFAttrTypeEnum() const noexcept
    {
        return m_value;
    }

private:
    ::TFAttrTypeEnum m_value{};
};

} // namespace ice::sonic
