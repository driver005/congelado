// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from c/extern/tensor/tensor.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "c/extern/tensor/tensor.h"

export module cc_abi_sonic_tensor;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;
import cc_abi_sonic_registration;

export namespace ice::sonic {

class Tensor : public ice::sonic::Runtime<Tensor, TF_Tensor>
{
public:
    explicit Tensor(TF_Tensor* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "tensor";

    [[nodiscard]] std::expected<void, ice::Status>
    allocate_tensor(TF_DataType_Enum dtype, const int64_t* dims, int num_dims, size_t len) noexcept
    {
        ice::Status status;
        m_ops->allocate_tensor(get_handle(), dtype, dims, num_dims, len, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> delete_tensor() noexcept
    {
        ice::Status status;
        m_ops->delete_tensor(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> tensor_type() noexcept
    {
        ice::Status status;
        m_ops->tensor_type(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> num_dims() noexcept
    {
        ice::Status status;
        m_ops->num_dims(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> dim(int dim_index) noexcept
    {
        ice::Status status;
        m_ops->dim(get_handle(), dim_index, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> tensor_element_count() noexcept
    {
        ice::Status status;
        m_ops->tensor_element_count(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> tensor_byte_size() noexcept
    {
        ice::Status status;
        m_ops->tensor_byte_size(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> tensor_data() noexcept
    {
        ice::Status status;
        m_ops->tensor_data(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    tensor_bitcast_from(TF_DataType_Enum dtype, TF_Tensor_Handle** out) noexcept
    {
        ice::Status status;
        m_ops->tensor_bitcast_from(get_handle(), dtype, out, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    tensor_bitcast_to(TF_DataType_Enum dtype, TF_Tensor_Handle** out) noexcept
    {
        ice::Status status;
        m_ops->tensor_bitcast_to(get_handle(), dtype, out, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    tensor_copy(const ice::sonic::Tensor& dst) noexcept
    {
        ice::Status status;
        m_ops->tensor_copy(get_handle(), dst.get_handle(), status.get_handle());

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
