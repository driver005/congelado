// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/intern/tensor.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/intern/tensor.h"

export module cc_abi_sonic_intern;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;
import cc_abi_sonic_registration;

export namespace ice::sonic {

class TF_TensorOps : public ice::sonic::Runtime<TF_TensorOps, TF_TensorOps>
{
public:
    explicit TF_TensorOps(TF_TensorOps* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "intern";

    [[nodiscard]] std::expected<void, ice::Status> set_dtype(TFDataTypeEnum dtype) noexcept
    {
        ice::Status status;
        m_ops->set_dtype(get_handle(), dtype status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    set_dims(const int64_t* dims, int num_dims) noexcept
    {
        ice::Status status;
        m_ops->set_dims(get_handle(), dims, num_dims status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> set_byte_size(size_t len) noexcept
    {
        ice::Status status;
        m_ops->set_byte_size(get_handle(), len status.get_handle());

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

    [[nodiscard]] std::expected<void, ice::Status> tensor_type(TFDataTypeEnum* out_dtype) noexcept
    {
        ice::Status status;
        m_ops->tensor_type(get_handle(), out_dtype status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> num_dims(int* out_num_dims) noexcept
    {
        ice::Status status;
        m_ops->num_dims(get_handle(), out_num_dims status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> dim(int dim_index, int64_t* out_dim) noexcept
    {
        ice::Status status;
        m_ops->dim(get_handle(), dim_index, out_dim status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> tensor_element_count(int64_t* out_count) noexcept
    {
        ice::Status status;
        m_ops->tensor_element_count(get_handle(), out_count status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> tensor_byte_size(size_t* out_byte_size) noexcept
    {
        ice::Status status;
        m_ops->tensor_byte_size(get_handle(), out_byte_size status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> tensor_data(void** out_data) noexcept
    {
        ice::Status status;
        m_ops->tensor_data(get_handle(), out_data status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    tensor_bitcast_from(TFDataTypeEnum dtype, TF_Tensor** out_tensor) noexcept
    {
        ice::Status status;
        m_ops->tensor_bitcast_from(get_handle(), dtype, out_tensor status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    tensor_bitcast_to(TFDataTypeEnum dtype, TF_Tensor** out_tensor) noexcept
    {
        ice::Status status;
        m_ops->tensor_bitcast_to(get_handle(), dtype, out_tensor status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    tensor_copy(const ice::sonic::TF_TensorOps& dst) noexcept
    {
        ice::Status status;
        m_ops->tensor_copy(get_handle(), dst.get_handle() status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    virtual ice::String get_name() const noexcept = 0;

    ice::String get_name() const noexcept
    {
        ice::String result;
        m_ops->get_name(get_handle(), result.get_handle());
        return result;
    }
};

} // namespace ice::sonic
