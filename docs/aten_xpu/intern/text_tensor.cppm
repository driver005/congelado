module;

#include "include/c/intern/status.h"
#include "include/c/intern/tensor.h"
#include "include/c/intern/tstring.h"

export module aten_xpu_intern:text_tensor;

import std;
import cc_ice_intern_builder;
import cc_ice_intern_sonic;
import :failure;

export namespace aten_xpu {

// A one dimensional string tensor in plain host memory: every line is stored NUL terminated, back
// to back. Views and bitcasts are not supported.
class TextTensor : public ice::builder::Tensor
{
public:
    TextTensor(
        const ::TF_StatusOps* status_ops,
        const ::TF_StringOps* string_ops,
        const ::TF_TensorOps* tensor_ops
    ) noexcept :
        ice::builder::Tensor{status_ops, string_ops, tensor_ops},
        m_failure{string_ops}
    {
    }

    ~TextTensor() override = default;
    TextTensor(const TextTensor&) = delete;
    TextTensor& operator=(const TextTensor&) = delete;
    TextTensor(TextTensor&&) = delete;
    TextTensor& operator=(TextTensor&&) = delete;

    // Becomes a string tensor holding `lines`.
    void assign(std::span<const std::string> lines)
    {
        m_dtype = ice::sonic::DataTypeEnum::k_string;
        m_dims.assign(1, static_cast<std::int64_t>(lines.size()));
        m_strides.assign(1, 1);
        m_bytes.clear();
        for (const std::string& line: lines) {
            for (const char letter: line) {
                m_bytes.push_back(static_cast<std::byte>(letter));
            }
            m_bytes.push_back(std::byte{0});
        }
    }

    // Splits a string tensor's bytes back into lines.
    static std::vector<std::string> lines_of(const ice::sonic::Tensor& tensor)
    {
        void* data = nullptr;
        std::size_t size = 0;
        tensor.tensor_data(&data);
        tensor.tensor_byte_size(size);

        std::vector<std::string> lines;
        const auto* text = static_cast<const char*>(data);
        std::size_t start = 0;
        for (std::size_t index = 0; text != nullptr && index < size; ++index) {
            if (text[index] == '\0') {
                lines.emplace_back(text + start, index - start);
                start = index + 1;
            }
        }

        return lines;
    }

    void destroy() noexcept override
    {
        delete this;
    }

    void get_name(const ice::sonic::String& out_name) noexcept override
    {
        out_name.copy("TextTensor");
    }

    void set_dtype(ice::sonic::DataTypeEnum dtype) noexcept override
    {
        m_dtype = dtype;
    }

    void set_dims(std::span<const std::int64_t> dims) noexcept override
    {
        m_dims.assign(dims.begin(), dims.end());
    }

    void set_byte_size(std::size_t len) noexcept override
    {
        m_bytes.resize(len);
    }

    void delete_tensor() noexcept override
    {
        delete this;
    }

    void tensor_type(ice::sonic::DataTypeEnum& out_dtype) noexcept override
    {
        out_dtype = m_dtype;
    }

    void num_dims(int& out_num_dims) noexcept override
    {
        out_num_dims = static_cast<int>(m_dims.size());
    }

    void dim(int dim_index, std::int64_t& out_dim) noexcept override
    {
        out_dim = m_dims.at(static_cast<std::size_t>(dim_index));
    }

    void tensor_element_count(std::int64_t& out_count) noexcept override
    {
        out_count = std::ranges::fold_left(m_dims, std::int64_t{1}, std::multiplies<>{});
    }

    void tensor_byte_size(std::size_t& out_byte_size) noexcept override
    {
        out_byte_size = m_bytes.size();
    }

    void tensor_data(void** out_data) noexcept override
    {
        *out_data = m_bytes.data();
    }

    void tensor_bitcast_from(
        ice::sonic::DataTypeEnum dtype,
        ::TF_Tensor** out_tensor,
        const ice::sonic::Status& out_status
    ) noexcept override
    {
        static_cast<void>(dtype);
        unsupported(out_tensor, out_status);
    }

    void tensor_bitcast_to(
        ice::sonic::DataTypeEnum dtype,
        ::TF_Tensor** out_tensor,
        const ice::sonic::Status& out_status
    ) noexcept override
    {
        static_cast<void>(dtype);
        unsupported(out_tensor, out_status);
    }

    void tensor_copy(const ice::sonic::Tensor& dst) noexcept override
    {
        void* target = nullptr;
        std::size_t capacity = 0;
        dst.tensor_data(&target);
        dst.tensor_byte_size(capacity);
        if (target != nullptr) {
            std::memcpy(target, m_bytes.data(), std::min(capacity, m_bytes.size()));
        }
    }

    void set_strides(std::span<const std::int64_t> strides) noexcept override
    {
        m_strides.assign(strides.begin(), strides.end());
    }

    void stride(int dim_index, std::int64_t& out_stride) noexcept override
    {
        out_stride = m_strides.at(static_cast<std::size_t>(dim_index));
    }

    void set_storage_offset(std::int64_t offset_elements) noexcept override
    {
        m_storage_offset = offset_elements;
    }

    void storage_offset(std::int64_t& out_offset_elements) noexcept override
    {
        out_offset_elements = m_storage_offset;
    }

    void set_device_index(int device_index) noexcept override
    {
        m_device_index = device_index;
    }

    void get_device_index(int& out_device_index) noexcept override
    {
        out_device_index = m_device_index;
    }

    void tensor_view(
        std::span<const std::int64_t> dims,
        std::span<const std::int64_t> strides,
        std::int64_t offset_elements,
        ::TF_Tensor** out_view,
        const ice::sonic::Status& out_status
    ) noexcept override
    {
        static_cast<void>(dims);
        static_cast<void>(strides);
        static_cast<void>(offset_elements);
        unsupported(out_view, out_status);
    }

private:
    void unsupported(::TF_Tensor** out_tensor, const ice::sonic::Status& out_status) const noexcept
    {
        *out_tensor = nullptr;
        m_failure.fail(
            out_status,
            ice::sonic::Code::k_unimplemented,
            "text tensors are plain host memory: no views or bitcasts"
        );
    }

    Failure m_failure;
    ice::sonic::DataTypeEnum m_dtype{ice::sonic::DataTypeEnum::k_string};
    std::vector<std::int64_t> m_dims;
    std::vector<std::int64_t> m_strides;
    std::vector<std::byte> m_bytes;
    std::int64_t m_storage_offset{0};
    int m_device_index{0};
};

} // namespace aten_xpu
