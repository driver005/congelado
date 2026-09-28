// SYCL reference plugin — one SyclTensor per TF_Tensor handle.
//
// Not built by Bazel (docs/ only). Replaces core/EmptyTensor.cpp's at::detail::empty_xpu path
// for the parts TF_TensorOps actually exposes: dtype, contiguous-by-default dims/strides,
// storage offset, bitcast and copy. A real backend would route allocation through
// SyclAllocator/TF_MemorySpace so scratch reuse and stats stay accurate; this reference tensor
// allocates its own USM buffer directly with sycl::malloc_device/sycl::free instead, since
// TF_TensorOps::set_byte_size has no stream parameter to allocate against — that wiring belongs
// to whichever kernel-context implementation actually calls set_byte_size (see kernels/README).
//
// Same self-registering pattern as include/cc/ice/support/status.cppm and string.cppm: a
// SyclTensor sets handle->plugin_data = this at construction, rather than going through
// tensor.h's create_tensor/destroy_tensor (that pair only hands back the shared TF_TensorOps*
// vtable; the plugin_data of any one TF_Tensor instance is this object).

module;

#include "include/c/intern/tensor.h"

export module sycl_backend:tensor;

import std;
import cc_ice_extern_stream_executor_builder;
import cc_ice_intern_builder;

export namespace sycl_backend {

class SyclTensor : public ice::builder::Tensor
{
public:
    SyclTensor(TF_Tensor* handle, sycl::context& context, sycl::device& device, int device_index) :
        m_context{context},
        m_device{device},
        m_device_index{device_index}
    {
        handle->plugin_data = this;
    }

    ~SyclTensor() override
    {
        free_storage();
    }

    SyclTensor(const SyclTensor&) = delete;
    SyclTensor& operator=(const SyclTensor&) = delete;
    SyclTensor(SyclTensor&&) = delete;
    SyclTensor& operator=(SyclTensor&&) = delete;

    void get_name(ice::builder::String& out_name) noexcept override
    {
        static constexpr std::string_view name = "SyclTensor";
        out_name.copy(name.data(), name.size());
    }

    void set_dtype(TFDataTypeEnum dtype) noexcept override
    {
        m_dtype = dtype;
    }

    // Contiguous by default; set_strides below overrides for a transposed/permuted view.
    void set_dims(const int64_t* dims, int num_dims) noexcept override
    {
        m_dims.assign(dims, dims + num_dims);
        m_strides = contiguous_strides(m_dims);
    }

    void set_byte_size(size_t len) noexcept override
    {
        free_storage();
        m_byte_size = len;

        if (len == 0) {
            return;
        }

        m_data = sycl::malloc_device(len, m_device, m_context);
    }

    void delete_tensor() noexcept override
    {
        delete this;
    }

    void tensor_type(TFDataTypeEnum* out_dtype) noexcept override
    {
        *out_dtype = m_dtype;
    }

    void num_dims(int* out_num_dims) noexcept override
    {
        *out_num_dims = static_cast<int>(m_dims.size());
    }

    void dim(int dim_index, int64_t* out_dim) noexcept override
    {
        *out_dim = m_dims.at(static_cast<std::size_t>(dim_index));
    }

    void tensor_element_count(int64_t* out_count) noexcept override
    {
        int64_t count = 1;
        for (int64_t extent: m_dims) {
            count *= extent;
        }
        *out_count = count;
    }

    void tensor_byte_size(size_t* out_byte_size) noexcept override
    {
        *out_byte_size = m_byte_size;
    }

    void tensor_data(void** out_data) noexcept override
    {
        *out_data = element_size(m_dtype) == 0
                        ? m_data
                        : static_cast<std::byte*>(m_data) +
                              static_cast<std::size_t>(m_storage_offset) * element_size(m_dtype);
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    tensor_bitcast_from(TFDataTypeEnum dtype, TF_Tensor** out_tensor) noexcept override
    {
        return make_view(m_dims, m_strides, m_storage_offset, dtype, out_tensor);
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    tensor_bitcast_to(TFDataTypeEnum dtype, TF_Tensor** out_tensor) noexcept override
    {
        return make_view(m_dims, m_strides, m_storage_offset, dtype, out_tensor);
    }

    void tensor_copy(ice::builder::Tensor& dst) noexcept override
    {
        auto& native_dst = static_cast<SyclTensor&>(dst);

        try {
            sycl::queue temporary{m_context, m_device};
            temporary.memcpy(native_dst.m_data, m_data, m_byte_size).wait_and_throw();
        } catch (const sycl::exception&) {
            // tensor_copy has no TF_Status out-param — there is nowhere to report a failed copy.
        }
    }

    void set_strides(const int64_t* strides, int num_strides) noexcept override
    {
        m_strides.assign(strides, strides + num_strides);
    }

    void stride(int dim_index, int64_t* out_stride) noexcept override
    {
        *out_stride = m_strides.at(static_cast<std::size_t>(dim_index));
    }

    void set_storage_offset(int64_t offset_elements) noexcept override
    {
        m_storage_offset = offset_elements;
    }

    void storage_offset(int64_t* out_offset_elements) noexcept override
    {
        *out_offset_elements = m_storage_offset;
    }

    void set_device_index(int device_index) noexcept override
    {
        m_device_index = device_index;
    }

    void get_device_index(int* out_device_index) noexcept override
    {
        *out_device_index = m_device_index;
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status> tensor_view(
        const int64_t* dims,
        int num_dims,
        const int64_t* strides,
        int64_t offset_elements,
        TF_Tensor** out_view
    ) noexcept override
    {
        return make_view(
            std::vector<int64_t>(dims, dims + num_dims),
            std::vector<int64_t>(strides, strides + num_dims),
            offset_elements,
            m_dtype,
            out_view
        );
    }

    // TF_DataTypeOps::datatype_size covers the general case; this reference tensor only needs
    // the sizes for the dtypes the kernels in kernels/ actually use.
    static size_t element_size(TFDataTypeEnum dtype) noexcept
    {
        switch (dtype) {
            case TF_BOOL:
            case TF_UINT8:
            case TF_INT8:
            case TF_QINT8:
            case TF_QUINT8:
            case TF_FLOAT8_E5M2:
            case TF_FLOAT8_E4M3FN:
            case TF_FLOAT8_E4M3FNUZ:
            case TF_FLOAT8_E4M3B11FNUZ:
            case TF_FLOAT8_E5M2FNUZ:
                return 1;
            case TF_HALF:
            case TF_BFLOAT16:
            case TF_INT16:
            case TF_UINT16:
            case TF_QINT16:
            case TF_QUINT16:
                return 2;
            case TF_FLOAT:
            case TF_INT32:
            case TF_UINT32:
            case TF_QINT32:
                return 4;
            case TF_DOUBLE:
            case TF_INT64:
            case TF_UINT64:
            case TF_COMPLEX64:
                return 8;
            case TF_COMPLEX128:
                return 16;
            default:
                return 0;
        }
    }

private:
    static std::vector<int64_t> contiguous_strides(const std::vector<int64_t>& dims)
    {
        std::vector<int64_t> strides(dims.size(), 1);
        for (std::size_t index = dims.size(); index-- > 1;) {
            strides[index - 1] = strides[index] * dims[index];
        }
        return strides;
    }

    void free_storage() noexcept
    {
        if (m_owns_storage && m_data != nullptr) {
            sycl::free(m_data, m_context);
        }
        m_data = nullptr;
    }

    std::expected<void, ice::sonic::Status> make_view(
        std::vector<int64_t> dims,
        std::vector<int64_t> strides,
        int64_t offset_elements,
        TFDataTypeEnum dtype,
        TF_Tensor** out_tensor
    ) noexcept
    {
        auto* handle = new TF_Tensor{};
        auto* view = new SyclTensor(handle, m_context, m_device, m_device_index);

        // A view shares this tensor's storage; it does not own or free it.
        view->m_owns_storage = false;
        view->m_data = m_data;
        view->m_byte_size = m_byte_size;
        view->m_dtype = dtype;
        view->m_dims = std::move(dims);
        view->m_strides = std::move(strides);
        view->m_storage_offset = offset_elements;

        *out_tensor = handle;
        return {};
    }

    sycl::context& m_context;
    sycl::device& m_device;
    int m_device_index;
    void* m_data{nullptr};
    bool m_owns_storage{true};
    size_t m_byte_size{0};
    TFDataTypeEnum m_dtype{TF_FLOAT};
    std::vector<int64_t> m_dims;
    std::vector<int64_t> m_strides;
    int64_t m_storage_offset{0};
};

} // namespace sycl_backend
