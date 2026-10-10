// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/intern/shape.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/registration/registration.h"
#include "include/c/intern/shape.h"

export module cc_ice_intern_sonic:shape;

import std;
import :runtime;
import :tstring;

export namespace ice::sonic {

class TF_ShapeOps : public ice::sonic::Runtime<::TF_ShapeOps, ::TF_Shape>
{
public:
    TF_ShapeOps(
        const ::TF_RegistrationOps& registry_ops,
        ::TF_Registration* registry_handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry_ops, registry_handle, type.get_handle(), provider.get_handle())
    {
    }

    TF_ShapeOps(
        const ::TF_RegistrationOps& registry_ops,
        ::TF_Registration* registry_handle,
        ::TF_Shape* handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry_ops, registry_handle, handle, type.get_handle(), provider.get_handle())
    {
    }

    explicit TF_ShapeOps(const ::TF_ShapeOps* ops) noexcept :
        Runtime(ops)
    {
    }

    TF_ShapeOps(const ::TF_ShapeOps* ops, ::TF_Shape* handle) noexcept :
        Runtime(ops, handle)
    {
    }

    void create() const noexcept
    {
        m_ops->create(get_handle());
    }

    void destroy() const noexcept
    {
        m_ops->destroy(get_handle());
    }

    void get_name(const ice::sonic::String& out_name) const noexcept
    {
        m_ops->get_name(get_handle(), out_name.get_handle());
    }

    void set_dims(const int64_t* dims, int num_dims) const noexcept
    {
        m_ops->set_dims(get_handle(), dims, num_dims);
    }

    void delete_shape() const noexcept
    {
        m_ops->delete_shape(get_handle());
    }

    void shape_num_dims(int* out_num_dims) const noexcept
    {
        m_ops->shape_num_dims(get_handle(), out_num_dims);
    }

    void shape_dim(int index, int64_t* out_dim) const noexcept
    {
        m_ops->shape_dim(get_handle(), index, out_dim);
    }
};

} // namespace ice::sonic
