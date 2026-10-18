module;

#include "include/c/extern/stream_executor/types.h"
#include "include/c/intern/tensor.h"

#include <sycl/sycl.hpp>

export module aten_xpu_intern:tensor;

import std;
import cc_ice_extern_stream_executor_sonic;
import cc_ice_intern_sonic;
import cc_ice_intern_builder;
import :ops_table;
import :placement;
import :datatype;
import :status;

export namespace aten_xpu {

class SyclTensor : public ice::builder::Tensor
{
public:
    explicit SyclTensor(const SyclOpsTable& ops) noexcept :
        ice::builder::Tensor{ops.getStatusOps(), ops.getStringOps(), ops.getTensorOps()},
        m_status{ops},
        m_placement{ops.getPlacement()},
        m_device_index{ops.getPlacement().getDefaultDeviceIndex()}
    {
    }

    ~SyclTensor() override = default;
    SyclTensor(const SyclTensor&) = delete;
    SyclTensor& operator=(const SyclTensor&) = delete;
    SyclTensor(SyclTensor&&) = delete;
    SyclTensor& operator=(SyclTensor&&) = delete;

    void setMemorySpace(ice::sonic::MemorySpace memory_space) noexcept
    {
        m_memory_space = memory_space;
    }

    void destroy() noexcept override
    {
        delete this;
    }

    void get_name(const ice::sonic::String& out_name) noexcept override
    {
        m_status.copy_into(out_name, "SyclTensor");
    }

    void set_dtype(ice::sonic::DataTypeEnum dtype) noexcept override
    {
        m_dtype = dtype;
    }

    void set_dims(std::span<const std::int64_t> dims) noexcept override
    {
        m_dims.assign(dims.begin(), dims.end());
        fill_contiguous_strides();
        const auto bytes =
            static_cast<std::size_t>(element_count()) * m_datatype.element_size(m_dtype);
        if (bytes != m_byte_size || !m_storage) {
            set_byte_size(bytes);
        }
    }

    void set_byte_size(size_t len) noexcept override
    {
        m_storage.reset();
        m_byte_size = len;
        m_storage_offset = 0;
        if (len == 0) {
            return;
        }

        const auto memory_space = m_memory_space;
        void* pointer = m_placement.allocate_storage(len, memory_space);
        if (pointer != nullptr) {
            m_storage = std::shared_ptr<void>(
                pointer,
                [placement = &m_placement, memory_space](void* storage)
                {
                    placement->deallocate_storage(storage, memory_space);
                }
            );
        }
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
        out_count = element_count();
    }

    void tensor_byte_size(std::size_t& out_byte_size) noexcept override
    {
        out_byte_size = m_byte_size;
    }

    void tensor_data(void** out_data) noexcept override
    {
        *out_data = getData();
    }

    void tensor_bitcast_from(
        ice::sonic::DataTypeEnum dtype,
        ::TF_Tensor** out_tensor,
        const ice::sonic::Status& out_status
    ) noexcept override
    {
        bitcast(dtype, out_tensor, out_status);
    }

    void tensor_bitcast_to(
        ice::sonic::DataTypeEnum dtype,
        ::TF_Tensor** out_tensor,
        const ice::sonic::Status& out_status
    ) noexcept override
    {
        bitcast(dtype, out_tensor, out_status);
    }

    void tensor_copy(const ice::sonic::Tensor& dst) noexcept override
    {
        auto& target = static_cast<SyclTensor&>(SyclTensor::from_handle(dst.get_handle()));
        if (getData() == nullptr || target.getData() == nullptr || !m_placement.hasPlacement()) {
            return;
        }

        try {
            sycl::queue queue{*m_placement.getContext(), *m_placement.getDevice()};
            queue.memcpy(target.getData(), getData(), std::min(m_byte_size, target.m_byte_size))
                .wait_and_throw();
        } catch (const sycl::exception&) {
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

    void set_storage_offset(int64_t offset_elements) noexcept override
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
        int64_t offset_elements,
        ::TF_Tensor** out_view,
        const ice::sonic::Status& out_status
    ) noexcept override
    {
        if (!fits_storage(
                dims.data(),
                strides.data(),
                static_cast<int>(dims.size()),
                offset_elements,
                m_datatype.element_size(m_dtype)
            )) {
            m_status
                .fail(out_status, ice::sonic::Code::k_out_of_range, "view exceeds tensor storage");
            return;
        }

        auto* handle = create_handle();
        auto& view = static_cast<SyclTensor&>(SyclTensor::from_handle(handle));
        view.share_storage_from(*this, m_dtype);
        view.m_dims.assign(dims.begin(), dims.end());
        view.m_strides.assign(
            strides.begin(),
            strides.begin() + static_cast<std::ptrdiff_t>(dims.size())
        );
        view.m_storage_offset = offset_elements;
        *out_view = handle;
    }

    void* getData() const noexcept
    {
        if (!m_storage) {
            return nullptr;
        }
        return static_cast<std::byte*>(m_storage.get()) +
               static_cast<std::size_t>(m_storage_offset) * m_datatype.element_size(m_dtype);
    }

    ice::sonic::DataTypeEnum getDtype() const noexcept
    {
        return m_dtype;
    }

    const std::vector<int64_t>& getDims() const noexcept
    {
        return m_dims;
    }

    const std::vector<int64_t>& getStrides() const noexcept
    {
        return m_strides;
    }

    int64_t getStorageOffset() const noexcept
    {
        return m_storage_offset;
    }

    std::size_t getByteSize() const noexcept
    {
        return m_byte_size;
    }

    int getDeviceIndex() const noexcept
    {
        return m_device_index;
    }

    ice::sonic::MemorySpace getMemorySpace() const noexcept
    {
        return m_memory_space;
    }

    int64_t element_count() const noexcept
    {
        return std::accumulate(m_dims.begin(), m_dims.end(), int64_t{1}, std::multiplies<>{});
    }

    bool is_contiguous() const noexcept
    {
        int64_t expected = 1;
        for (std::size_t index = m_dims.size(); index-- > 0;) {
            if (m_dims[index] != 1 && m_strides[index] != expected) {
                return false;
            }
            expected *= m_dims[index];
        }
        return true;
    }

private:
    void fill_contiguous_strides()
    {
        m_strides.assign(m_dims.size(), 1);
        for (std::size_t index = m_dims.size(); index-- > 1;) {
            m_strides[index - 1] = m_strides[index] * m_dims[index];
        }
    }

    void share_storage_from(const SyclTensor& source, ice::sonic::DataTypeEnum dtype)
    {
        m_storage = source.m_storage;
        m_byte_size = source.m_byte_size;
        m_memory_space = source.m_memory_space;
        m_device_index = source.m_device_index;
        m_dtype = dtype;
    }

    void bitcast(
        ice::sonic::DataTypeEnum dtype,
        TF_Tensor** out_tensor,
        const ice::sonic::Status& out_status
    )
    {
        const auto source_size = m_datatype.element_size(m_dtype);
        const auto target_size = m_datatype.element_size(dtype);
        if (m_dims.empty() || source_size == target_size) {
            auto* handle = create_handle();
            auto& view = static_cast<SyclTensor&>(SyclTensor::from_handle(handle));
            view.share_storage_from(*this, dtype);
            view.m_dims = m_dims;
            view.m_strides = m_strides;
            view.m_storage_offset = m_storage_offset;
            *out_tensor = handle;
            return;
        }

        const auto last_bytes = static_cast<std::size_t>(m_dims.back()) * source_size;
        if (!is_contiguous() || last_bytes % target_size != 0) {
            m_status.fail(
                out_status,
                ice::sonic::Code::k_invalid_argument,
                "bitcast changes the innermost extent"
            );
            return;
        }

        auto* handle = create_handle();
        auto& view = static_cast<SyclTensor&>(SyclTensor::from_handle(handle));
        view.share_storage_from(*this, dtype);
        view.m_dims = m_dims;
        view.m_dims.back() = static_cast<int64_t>(last_bytes / target_size);
        view.fill_contiguous_strides();
        view.m_storage_offset = m_storage_offset * static_cast<int64_t>(source_size) /
                                static_cast<int64_t>(target_size);
        *out_tensor = handle;
    }

    bool fits_storage(
        const int64_t* dims,
        const int64_t* strides,
        int num_dims,
        int64_t offset_elements,
        std::size_t item_size
    ) const noexcept
    {
        int64_t last_element = offset_elements;
        for (int index = 0; index < num_dims; ++index) {
            if (dims[index] == 0) {
                return true;
            }
            last_element += (dims[index] - 1) * strides[index];
        }
        return static_cast<std::size_t>(last_element + 1) * item_size <= m_byte_size;
    }

    SyclStatus m_status;
    SyclDataType m_datatype;
    const SyclTensorPlacement& m_placement;
    std::shared_ptr<void> m_storage;
    std::size_t m_byte_size{0};
    ice::sonic::DataTypeEnum m_dtype{ice::sonic::DataTypeEnum::k_float};
    std::vector<int64_t> m_dims;
    std::vector<int64_t> m_strides;
    int64_t m_storage_offset{0};
    int m_device_index{0};
    ice::sonic::MemorySpace m_memory_space{ice::sonic::MemorySpace::k_device};
};

} // namespace aten_xpu
