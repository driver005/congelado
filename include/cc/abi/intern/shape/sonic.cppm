// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from c/extern/shape/shape.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "c/extern/shape/shape.h"

export module cc_abi_sonic_shape;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;
import cc_abi_sonic_registration;

export namespace ice::sonic {

class Shape : public ice::sonic::Runtime<Shape, TF_Shape>
{
public:
    explicit Shape(TF_Shape* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "shape";

    [[nodiscard]] std::expected<void, ice::Status>
    new_shape(const int64_t* dims, int num_dims) noexcept
    {
        ice::Status status;
        m_ops->new_shape(get_handle(), dims, num_dims, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> delete_shape() noexcept
    {
        ice::Status status;
        m_ops->delete_shape(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> shape_num_dims() noexcept
    {
        ice::Status status;
        m_ops->shape_num_dims(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> shape_dim(int index) noexcept
    {
        ice::Status status;
        m_ops->shape_dim(get_handle(), index, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    ice::String get_name() const noexcept
    {
        ice::String result;
        m_ops->get_name(get_handle(), result.get_handle());
        return result;
    }
};

} // namespace ice::sonic
