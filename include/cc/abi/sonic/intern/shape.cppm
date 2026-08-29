module;

#include "c/intern/tf_shape.h"

export module cc_abi_sonic_intern:shape;

import std;
import :runtime;
import cc_abi_primitives;

export namespace ice::sonic {

class Shape : public Runtime<Shape, TF_Shape>
{
public:
    static constexpr std::string_view domain_name = "shape";

    explicit Shape(TF_Shape* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    ice::String get_name() const
    {
        ice::String out;
        m_ops->get_name(m_host_context, out.get_handle());
        return out;
    }

    [[nodiscard]] std::expected<TF_Shape_Handle*, ice::Status>
    new_shape(const int64_t* dims, int num_dims)
    {
        TF_Shape_Handle* handle = m_ops->new_shape(m_host_context, dims, num_dims);
        if (handle == nullptr) {
            return std::unexpected{ice::Status{"shape allocation failed"}};
        }
        return handle;
    }

    void delete_shape(TF_Shape_Handle* shape)
    {
        m_ops->delete_shape(m_host_context, shape);
    }

    int get_num_dims(const TF_Shape_Handle* shape) const
    {
        return m_ops->shape_num_dims(m_host_context, shape);
    }

    int64_t get_dim(const TF_Shape_Handle* shape, int index) const
    {
        return m_ops->shape_dim(m_host_context, shape, index);
    }
};

} // namespace ice::sonic
