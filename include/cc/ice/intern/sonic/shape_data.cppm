// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/intern/shape.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/intern/shape.h"

export module cc_ice_intern_sonic:shape_data;

import std;

export namespace ice::sonic {

class ShapeData
{
public:
    ShapeData() noexcept
    {
        m_value.struct_size = sizeof(::TFShapeData);
    }

    explicit ShapeData(const ::TFShapeData& value) noexcept :
        m_value{value}
    {
    }

    ShapeData(std::int64_t* dims, int num_dims) noexcept
    {
        m_value.struct_size = sizeof(::TFShapeData);
        m_value.dims = dims;
        m_value.num_dims = num_dims;
    }

    std::int64_t* getDims() const noexcept
    {
        return m_value.dims;
    }

    void setDims(std::int64_t* value) noexcept
    {
        m_value.dims = value;
    }

    int getNumDims() const noexcept
    {
        return m_value.num_dims;
    }

    void setNumDims(int value) noexcept
    {
        m_value.num_dims = value;
    }

    ::TFShapeData* get_handle() noexcept
    {
        return &m_value;
    }

    const ::TFShapeData* get_handle() const noexcept
    {
        return &m_value;
    }

private:
    ::TFShapeData m_value{};
};

} // namespace ice::sonic
