module;

#include "c/intern/tf_shape.h"

export module cc_abi_builder_intern:shape;

import std;

export namespace ice {

class ShapeBuilder {
  public:
    ShapeBuilder() : m_shape{TF_NewShape(nullptr, 0)} {}

    explicit ShapeBuilder(const std::vector<int64_t> &dims)
        : m_shape{TF_NewShape(dims.data(), static_cast<int>(dims.size()))} {}

    ~ShapeBuilder() {
        if (m_shape)
            TF_DeleteShape(m_shape);
    }

    ShapeBuilder(const ShapeBuilder &) = delete;
    ShapeBuilder &operator=(const ShapeBuilder &) = delete;

    ShapeBuilder(ShapeBuilder &&other) noexcept : m_shape{other.m_shape} { other.m_shape = nullptr; }

    ShapeBuilder &operator=(ShapeBuilder &&other) noexcept {
        if (this != &other) {
            if (m_shape)
                TF_DeleteShape(m_shape);
            m_shape = other.m_shape;
            other.m_shape = nullptr;
        }
        return *this;
    }

    bool is_valid() const { return m_shape != nullptr; }
    int get_num_dims() const { return TF_ShapeNumDims(m_shape); }
    int64_t get_dim(int index) const { return TF_ShapeDim(m_shape, index); }

    std::vector<int64_t> to_vector() const {
        int n = get_num_dims();
        std::vector<int64_t> dims(n);
        for (int i = 0; i < n; ++i)
            dims[i] = get_dim(i);
        return dims;
    }

    // Underlying handle — pass directly to the C ABI
    TF_Shape *get_handle() { return m_shape; }
    const TF_Shape *get_handle() const { return m_shape; }

  private:
    TF_Shape *m_shape;
};

} // namespace ice
