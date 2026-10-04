module;

#include "include/c/extern/stream_executor/types.h"
#include "include/c/intern/tensor.h"

#include <sycl/sycl.hpp>

export module aten_xpu_intern:tensor;

import std;
import cc_ice_intern_sonic;
import cc_ice_intern_builder;
import :ops_table;
import :handle;
import :status;

export namespace aten_xpu {

class SyclTensor : public ice::builder::TF_TensorOps
{
public:
    using Allocate = std::function<void*(std::size_t, TF_MemorySpace)>;
    using Deallocate = std::function<void(void*, TF_MemorySpace)>;

    explicit SyclTensor(const SyclOpsTable& ops) noexcept :
        ice::builder::TF_TensorOps{ops.getStatusOps(), ops.getStringOps(), ops.getTensorOps()},
        m_status{ops},
        m_device_index{s_default_device_index}
    {
    }

    ~SyclTensor() override = default;
    SyclTensor(const SyclTensor&) = delete;
    SyclTensor& operator=(const SyclTensor&) = delete;
    SyclTensor(SyclTensor&&) = delete;
    SyclTensor& operator=(SyclTensor&&) = delete;

    static void create(::TF_Tensor* handle)
    {

        auto* tensor = new SyclTensor{SyclOpsTable::getInstance()};
        SyclHandle::attach(handle, *tensor);

    }

    static ::TF_Tensor* create_handle()
    {

        auto* handle = new ::TF_Tensor{};
        create(handle);
        return handle;

    }

    static void setPlacement(const sycl::context& context, const sycl::device& device, int device_index)
    {

        s_context = context;
        s_device = device;
        s_default_device_index = device_index;

    }

    static void setStorageAllocator(Allocate allocate, Deallocate deallocate)
    {

        s_allocate = std::move(allocate);
        s_deallocate = std::move(deallocate);

    }

    void setMemorySpace(TF_MemorySpace memory_space) noexcept { m_memory_space = memory_space; }

    void destroy() noexcept override { delete this; }

    void get_name(const ice::sonic::String& out_name) noexcept override
    {

        m_status.copy_into(out_name, "SyclTensor");

    }

    void set_dtype(TFDataTypeEnum dtype) noexcept override { m_dtype = dtype; }

    void set_dims(const int64_t* dims, int num_dims) noexcept override
    {

        m_dims.assign(dims, dims + num_dims);
        fill_contiguous_strides();
        const auto bytes = static_cast<std::size_t>(element_count()) * element_size(m_dtype);
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
        void* pointer = allocate_storage(len, memory_space);
        if (pointer != nullptr) {
            m_storage = std::shared_ptr<void>(
                pointer,
                [memory_space](void* storage)
                {

                    deallocate_storage(storage, memory_space);

                }
            );
        }

    }

    void delete_tensor() noexcept override { delete this; }

    void tensor_type(TFDataTypeEnum* out_dtype) noexcept override { *out_dtype = m_dtype; }

    void num_dims(int* out_num_dims) noexcept override { *out_num_dims = static_cast<int>(m_dims.size()); }

    void dim(int dim_index, int64_t* out_dim) noexcept override
    {

        *out_dim = m_dims.at(static_cast<std::size_t>(dim_index));

    }

    void tensor_element_count(int64_t* out_count) noexcept override { *out_count = element_count(); }

    void tensor_byte_size(size_t* out_byte_size) noexcept override { *out_byte_size = m_byte_size; }

    void tensor_data(void** out_data) noexcept override { *out_data = getData(); }

    void tensor_bitcast_from(TFDataTypeEnum dtype, TF_Tensor** out_tensor, const ice::sonic::Status& out_status)
        noexcept override
    {

        bitcast(dtype, out_tensor, out_status);

    }

    void tensor_bitcast_to(TFDataTypeEnum dtype, TF_Tensor** out_tensor, const ice::sonic::Status& out_status)
        noexcept override
    {

        bitcast(dtype, out_tensor, out_status);

    }

    void tensor_copy(const ice::sonic::TF_TensorOps& dst) noexcept override
    {

        auto& target = SyclHandle::resolve<SyclTensor>(dst);
        if (getData() == nullptr || target.getData() == nullptr || !s_context || !s_device) {
            return;
        }

        try {
            sycl::queue queue{*s_context, *s_device};
            queue.memcpy(target.getData(), getData(), std::min(m_byte_size, target.m_byte_size)).wait_and_throw();
        } catch (const sycl::exception&) {
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

    void set_storage_offset(int64_t offset_elements) noexcept override { m_storage_offset = offset_elements; }

    void storage_offset(int64_t* out_offset_elements) noexcept override
    {

        *out_offset_elements = m_storage_offset;

    }

    void set_device_index(int device_index) noexcept override { m_device_index = device_index; }

    void get_device_index(int* out_device_index) noexcept override { *out_device_index = m_device_index; }

    void tensor_view(
        const int64_t* dims,
        int num_dims,
        const int64_t* strides,
        int64_t offset_elements,
        TF_Tensor** out_view,
        const ice::sonic::Status& out_status
    ) noexcept override
    {

        if (!fits_storage(dims, strides, num_dims, offset_elements, element_size(m_dtype))) {
            m_status.fail(out_status, TF_OUT_OF_RANGE, "view exceeds tensor storage");
            return;
        }

        auto* handle = create_handle();
        auto& view = SyclHandle::resolve_raw<SyclTensor>(handle);
        view.share_storage_from(*this, m_dtype);
        view.m_dims.assign(dims, dims + num_dims);
        view.m_strides.assign(strides, strides + num_dims);
        view.m_storage_offset = offset_elements;
        *out_view = handle;

    }

    void* getData() const noexcept
    {

        if (!m_storage) {
            return nullptr;
        }
        return static_cast<std::byte*>(m_storage.get()) +
               static_cast<std::size_t>(m_storage_offset) * element_size(m_dtype);

    }

    TFDataTypeEnum getDtype() const noexcept { return m_dtype; }

    const std::vector<int64_t>& getDims() const noexcept { return m_dims; }

    const std::vector<int64_t>& getStrides() const noexcept { return m_strides; }

    int64_t getStorageOffset() const noexcept { return m_storage_offset; }

    std::size_t getByteSize() const noexcept { return m_byte_size; }

    int getDeviceIndex() const noexcept { return m_device_index; }

    TF_MemorySpace getMemorySpace() const noexcept { return m_memory_space; }

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

    static std::size_t element_size(TFDataTypeEnum dtype) noexcept
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
                return 1;
        }

    }

private:
    void fill_contiguous_strides()
    {

        m_strides.assign(m_dims.size(), 1);
        for (std::size_t index = m_dims.size(); index-- > 1;) {
            m_strides[index - 1] = m_strides[index] * m_dims[index];
        }

    }

    void share_storage_from(const SyclTensor& source, TFDataTypeEnum dtype)
    {

        m_storage = source.m_storage;
        m_byte_size = source.m_byte_size;
        m_memory_space = source.m_memory_space;
        m_device_index = source.m_device_index;
        m_dtype = dtype;

    }

    void bitcast(TFDataTypeEnum dtype, TF_Tensor** out_tensor, const ice::sonic::Status& out_status)
    {

        const auto source_size = element_size(m_dtype);
        const auto target_size = element_size(dtype);
        if (m_dims.empty() || source_size == target_size) {
            auto* handle = create_handle();
            auto& view = SyclHandle::resolve_raw<SyclTensor>(handle);
            view.share_storage_from(*this, dtype);
            view.m_dims = m_dims;
            view.m_strides = m_strides;
            view.m_storage_offset = m_storage_offset;
            *out_tensor = handle;
            return;
        }

        const auto last_bytes = static_cast<std::size_t>(m_dims.back()) * source_size;
        if (!is_contiguous() || last_bytes % target_size != 0) {
            m_status.fail(out_status, TF_INVALID_ARGUMENT, "bitcast changes the innermost extent");
            return;
        }

        auto* handle = create_handle();
        auto& view = SyclHandle::resolve_raw<SyclTensor>(handle);
        view.share_storage_from(*this, dtype);
        view.m_dims = m_dims;
        view.m_dims.back() = static_cast<int64_t>(last_bytes / target_size);
        view.fill_contiguous_strides();
        view.m_storage_offset =
            m_storage_offset * static_cast<int64_t>(source_size) / static_cast<int64_t>(target_size);
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

    static void* allocate_storage(std::size_t size, TF_MemorySpace memory_space)
    {

        if (s_allocate) {
            return s_allocate(size, memory_space);
        }
        if (!s_context || !s_device) {
            return std::malloc(size);
        }
        switch (memory_space) {
            case TF_MEMORY_SPACE_HOST_PINNED:
                return sycl::malloc_host(size, *s_context);
            case TF_MEMORY_SPACE_UNIFIED:
                return sycl::malloc_shared(size, *s_device, *s_context);
            default:
                return sycl::malloc_device(size, *s_device, *s_context);
        }

    }

    static void deallocate_storage(void* storage, TF_MemorySpace memory_space)
    {

        if (s_deallocate) {
            s_deallocate(storage, memory_space);
            return;
        }
        if (!s_context) {
            std::free(storage);
            return;
        }
        sycl::free(storage, *s_context);

    }

    SyclStatus m_status;
    std::shared_ptr<void> m_storage;
    std::size_t m_byte_size{0};
    TFDataTypeEnum m_dtype{TF_FLOAT};
    std::vector<int64_t> m_dims;
    std::vector<int64_t> m_strides;
    int64_t m_storage_offset{0};
    int m_device_index{0};
    TF_MemorySpace m_memory_space{TF_MEMORY_SPACE_DEVICE};

    static inline std::optional<sycl::context> s_context;
    static inline std::optional<sycl::device> s_device;
    static inline int s_default_device_index{0};
    static inline Allocate s_allocate;
    static inline Deallocate s_deallocate;
};

} // namespace aten_xpu
