// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/intern/shape.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/intern/shape.h"

export module cc_ice_intern_sonic:shape;

import std;
import cc_abi_sonic_registration;

export namespace ice::sonic {

class TF_ShapeOps : public ice::sonic::Runtime<TF_ShapeOps, TF_ShapeOps>
{
public:
    explicit TF_ShapeOps(TF_ShapeOps* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "intern";

    void get_name(const ice::sonic::String& out_name) noexcept
    {
        m_ops->get_name(get_handle(), out_name.get_handle());
    }

    void set_dims(const int64_t* dims, int num_dims) noexcept
    {
        m_ops->set_dims(get_handle(), dims, num_dims);
    }

    void delete_shape() noexcept
    {
        m_ops->delete_shape(get_handle());
    }

    void shape_num_dims(int* out_num_dims) noexcept
    {
        m_ops->shape_num_dims(get_handle(), out_num_dims);
    }

    void shape_dim(int index, int64_t* out_dim) noexcept
    {
        m_ops->shape_dim(get_handle(), index, out_dim);
    }
};

} // namespace ice::sonic
