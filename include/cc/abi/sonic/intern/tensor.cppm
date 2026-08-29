module;

#include "c/intern/tf_tensor.h"

export module cc_abi_sonic_intern:tensor;

import std;
import :datatype;
import :runtime;
import cc_abi_primitives;

export namespace ice::sonic {

class Tensor : public Runtime<Tensor, TF_TensorOps>
{
public:
    static constexpr std::string_view domain_name = "tensor";

    explicit Tensor(TF_TensorOps* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    ice::String get_name() const noexcept
    {
        ice::String out;
        m_ops->get_name(m_host_context, out.get_handle());
        return out;
    }

    // Range-first: the C vtable carries (const int64_t*, int) — this C++ interface takes a
    // span and adapts at the single boundary line below.
    [[nodiscard]] std::expected<TF_Tensor_Handle*, ice::Status>
    allocate_tensor(DataTypeEnum dtype, std::span<const int64_t> dims, size_t len) noexcept
    {
        TF_Tensor_Handle* handle = m_ops->allocate_tensor(
            m_host_context,
            data_type_to_c(dtype),
            dims.data(),
            static_cast<int>(dims.size()),
            len
        );
        if (handle == nullptr) {
            return std::unexpected{ice::Status{"tensor allocation failed"}};
        }
        return handle;
    }

    void delete_tensor(TF_Tensor_Handle* tensor) noexcept
    {
        m_ops->delete_tensor(m_host_context, tensor);
    }

    DataTypeEnum get_type(const TF_Tensor_Handle* tensor) const noexcept
    {
        return data_type_from_c(m_ops->tensor_type(m_host_context, tensor));
    }

    int get_num_dims(const TF_Tensor_Handle* tensor) const noexcept
    {
        return m_ops->num_dims(m_host_context, tensor);
    }

    int64_t get_dim(const TF_Tensor_Handle* tensor, int dim_index) const noexcept
    {
        return m_ops->dim(m_host_context, tensor, dim_index);
    }

    int64_t get_num_elements(const TF_Tensor_Handle* tensor) const noexcept
    {
        return m_ops->tensor_element_count(m_host_context, tensor);
    }

    size_t get_byte_size(const TF_Tensor_Handle* tensor) const noexcept
    {
        return m_ops->tensor_byte_size(m_host_context, tensor);
    }

    // Raw data buffer — non-owning, valid while the tensor handle is alive; paired with
    // get_byte_size. Prefer get_data_as<T>() for typed, range-checked access.
    void* get_data(const TF_Tensor_Handle* tensor) const noexcept
    {
        return m_ops->tensor_data(m_host_context, tensor);
    }

    // Typed, range-first view of the tensor's element buffer — avoids static_cast<T*>
    // at call sites and carries the element count.
    template<typename T>
    std::span<T> get_data_as(const TF_Tensor_Handle* tensor) const noexcept
    {
        return {static_cast<T*>(get_data(tensor)), static_cast<size_t>(get_num_elements(tensor))};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    bitcast_from(TF_Tensor_Handle* src, DataTypeEnum dtype, TF_Tensor_Handle** out) noexcept
    {
        ice::Status status;
        m_ops->tensor_bitcast_from(
            m_host_context,
            src,
            data_type_to_c(dtype),
            out,
            status.get_handle()
        );
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    bitcast_to(const TF_Tensor_Handle* src, DataTypeEnum dtype, TF_Tensor_Handle** out) noexcept
    {
        ice::Status status;
        m_ops->tensor_bitcast_to(
            m_host_context,
            src,
            data_type_to_c(dtype),
            out,
            status.get_handle()
        );
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    void copy(TF_Tensor_Handle* src, TF_Tensor_Handle* dst) noexcept
    {
        m_ops->tensor_copy(m_host_context, src, dst);
    }
};

} // namespace ice::sonic
