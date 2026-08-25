module;

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <span>
#include <string>
#include <vector>

export module cc_tmp:tensor_tensor;

import std;
import cc_abi;
import :types_types;
import :tensor_tensor_shape;
import :allocator_allocator;

export

    namespace tensorflow {

// TensorBuffer manages memory allocation and lifetime for Tensor.
class TensorBuffer {
  public:
    TensorBuffer() : m_ref(1), m_data(nullptr), m_size(0), m_allocator(nullptr) {}

    explicit TensorBuffer(size_t size, Allocator *allocator = nullptr)
        : m_ref(1), m_size(size), m_allocator(allocator) {
        if (size > 0) {
            if (allocator) {
                m_data = allocator->AllocateRaw(64, size);
            } else {
                void *p = nullptr;
                if (posix_memalign(&p, 64, size) == 0) {
                    m_data = p;
                }
            }
        } else {
            m_data = nullptr;
        }
    }

    TensorBuffer(void *data, size_t size, void (*deallocator)(void *data, size_t len))
        : m_ref(1), m_data(data), m_size(size), m_allocator(nullptr),
          m_custom_deallocator(deallocator) {}

    virtual ~TensorBuffer() {
        if (m_data) {
            if (m_custom_deallocator) {
                m_custom_deallocator(m_data, m_size);
            } else if (m_allocator) {
                m_allocator->DeallocateRaw(m_data);
            } else {
                std::free(m_data);
            }
        }
    }

    TensorBuffer(const TensorBuffer &) = delete;
    TensorBuffer &operator=(const TensorBuffer &) = delete;

    void Ref() const { m_ref.fetch_add(1, std::memory_order_relaxed); }

    void Unref() const {
        if (m_ref.fetch_sub(1, std::memory_order_acq_rel) == 1) {
            delete this;
        }
    }

    bool RefCountIsOne() const { return m_ref.load(std::memory_order_acquire) == 1; }

    void *data() const { return m_data; }
    size_t size() const { return m_size; }
    Allocator *allocator() const { return m_allocator; }

  protected:
    mutable std::atomic<int32_t> m_ref{1};
    void *m_data{nullptr};
    size_t m_size{0};
    Allocator *m_allocator{nullptr};
    void (*m_custom_deallocator)(void *data, size_t len){nullptr};
};

// Tensor holds DataType, TensorShape, and refcounted TensorBuffer*.
// Concrete C++ implementation wrapped by C ABI ice::Tensor / TF_Tensor.
class Tensor {
  public:
    Tensor() : m_dtype{DT_INVALID}, m_shape{}, m_buf{nullptr} {}

    Tensor(DataType dtype, const TensorShape &shape, Allocator *a = nullptr)
        : m_dtype{dtype}, m_shape{shape}, m_buf{nullptr} {
        size_t total_bytes = shape.num_elements() * DataTypeSize(dtype);
        if (total_bytes > 0) {
            m_buf = new TensorBuffer(total_bytes, a ? a : cpu_allocator());
        }
    }

    Tensor(DataType dtype, const TensorShape &shape, TensorBuffer *buf)
        : m_dtype{dtype}, m_shape{shape}, m_buf{buf} {
        if (m_buf) {
            m_buf->Ref();
        }
    }

    ~Tensor() {
        if (m_buf) {
            m_buf->Unref();
        }
    }

    Tensor(const Tensor &other)
        : m_dtype{other.m_dtype}, m_shape{other.m_shape}, m_buf{other.m_buf} {
        if (m_buf) {
            m_buf->Ref();
        }
    }

    Tensor &operator=(const Tensor &other) {
        if (this != &other) {
            if (other.m_buf)
                other.m_buf->Ref();
            if (m_buf)
                m_buf->Unref();
            m_dtype = other.m_dtype;
            m_shape = other.m_shape;
            m_buf = other.m_buf;
        }
        return *this;
    }

    Tensor(Tensor &&other) noexcept
        : m_dtype{other.m_dtype}, m_shape{std::move(other.m_shape)}, m_buf{other.m_buf} {
        other.m_buf = nullptr;
        other.m_dtype = DT_INVALID;
    }

    Tensor &operator=(Tensor &&other) noexcept {
        if (this != &other) {
            if (m_buf)
                m_buf->Unref();
            m_dtype = other.m_dtype;
            m_shape = std::move(other.m_shape);
            m_buf = other.m_buf;
            other.m_buf = nullptr;
            other.m_dtype = DT_INVALID;
        }
        return *this;
    }

    bool IsInitialized() const { return m_buf != nullptr && m_buf->data() != nullptr; }
    DataType dtype() const { return m_dtype; }
    const TensorShape &shape() const { return m_shape; }
    int dims() const { return m_shape.dims(); }
    int64_t dim_size(int d) const { return m_shape.dim_size(d); }
    int64_t NumElements() const { return m_shape.num_elements(); }
    size_t TotalBytes() const { return m_buf ? m_buf->size() : 0; }

    void *data() { return m_buf ? m_buf->data() : nullptr; }
    const void *data() const { return m_buf ? m_buf->data() : nullptr; }

    TensorBuffer *buffer() const { return m_buf; }

    template <typename T>
    T *flat() {
        return static_cast<T *>(data());
    }

    template <typename T>
    const T *flat() const {
        return static_cast<const T *>(data());
    }

    template <typename T>
    std::span<T> span() {
        return std::span<T>(flat<T>(), static_cast<size_t>(NumElements()));
    }

    template <typename T>
    std::span<const T> span() const {
        return std::span<const T>(flat<T>(), static_cast<size_t>(NumElements()));
    }

    std::string DebugString() const {
        return "Tensor<type: " + std::string(DataTypeString(m_dtype)) +
               " shape: " + m_shape.DebugString() + ">";
    }
};

} // namespace tensorflow
