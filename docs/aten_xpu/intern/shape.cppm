module;

#include "include/c/intern/shape.h"
#include "include/c/intern/tstring.h"

export module aten_xpu_intern:shape;

import std;
import cc_ice_intern_builder;
import cc_ice_intern_sonic;

export namespace aten_xpu {

// A shape of dimensions; a rank-unknown shape reports -1 dims and is set through setUnknownRank.
class Shape : public ice::builder::Shape
{
public:
    explicit Shape(const ::TF_StringOps* string_ops) noexcept :
        ice::builder::Shape{string_ops}
    {
    }

    ~Shape() override = default;
    Shape(const Shape&) = delete;
    Shape& operator=(const Shape&) = delete;
    Shape(Shape&&) = delete;
    Shape& operator=(Shape&&) = delete;

    void destroy() noexcept override
    {
        delete this;
    }

    void setUnknownRank(bool unknown_rank) noexcept
    {
        m_unknown_rank = unknown_rank;
    }

    void get_name(const ice::sonic::String& out_name) noexcept override
    {
        out_name.copy("Shape");
    }

    void set_dims(std::span<const std::int64_t> dims) noexcept override
    {
        m_dims.assign(dims.begin(), dims.end());
        m_unknown_rank = false;
    }

    void delete_shape() noexcept override
    {
        delete this;
    }

    void shape_num_dims(int& out_num_dims) noexcept override
    {
        out_num_dims = m_unknown_rank ? -1 : static_cast<int>(m_dims.size());
    }

    void shape_dim(int index, std::int64_t& out_dim) noexcept override
    {
        const bool in_range = index >= 0 && static_cast<std::size_t>(index) < m_dims.size();
        out_dim = in_range ? m_dims[static_cast<std::size_t>(index)] : -1;
    }

private:
    std::vector<std::int64_t> m_dims;
    bool m_unknown_rank = false;
};

} // namespace aten_xpu
