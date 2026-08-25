module;

#include "c/intern/tf_shape.h"

export module cc_abi_runtime_intern:shape;

import std;

export namespace ice {

// ShapeRuntime — non-owning, read-only view over a `TF_Shape*` received from a plugin.
class ShapeRuntime {
  public:
    ShapeRuntime() : m_shape{nullptr} {}
    explicit ShapeRuntime(TF_Shape *shape) : m_shape{shape} {}

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
    TF_Shape *get_handle() const { return m_shape; }

  private:
    TF_Shape *m_shape;
};

} // namespace ice
