module;

#include "c/intern/tf_shape.h"

export module cc_abi_sonic_intern:shape;

import std;
import :runtime_base;
export namespace ice::sonic {

class Shape : public Runtime<Shape, TF_Shape, true>
{
public:
    static constexpr std::string_view domain_name = "shape";

    TF_Shape_Handle* new_shape(const int64_t* dims, int num_dims)
    {
        return m_ops->TF_NewShape(m_host_context, dims, num_dims);
    }

    void delete_shape(TF_Shape_Handle* shape)
    {
        m_ops->TF_DeleteShape(m_host_context, shape);
    }

    int get_num_dims(const TF_Shape_Handle* shape) const
    {
        return m_ops->TF_ShapeNumDims(m_host_context, shape);
    }

    int64_t get_dim(const TF_Shape_Handle* shape, int index) const
    {
        return m_ops->TF_ShapeDim(m_host_context, shape, index);
    }
};

} // namespace ice::sonic
