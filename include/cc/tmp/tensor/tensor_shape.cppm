module;

#include <cstddef>
#include <cstdint>
#include <vector>
#include <string>
#include <initializer_list>
#include <numeric>
#include <iostream>

export module cc_tmp:tensor_tensor_shape;

import std;
import cc_abi;

export {

namespace tensorflow {

class TensorShape {
public:
    TensorShape() : m_dims{} {}

    explicit TensorShape(const std::vector<int64_t>& dims) : m_dims{dims} {}
    TensorShape(std::initializer_list<int64_t> dims) : m_dims{dims} {}

    explicit TensorShape(const ice::Shape& shape) {
        m_dims = shape.to_vector();
    }

    int dims() const { return static_cast<int>(m_dims.size()); }
    int64_t dim_size(int d) const {
        if (d >= 0 && d < static_cast<int>(m_dims.size())) {
            return m_dims[d];
        }
        return 0;
    }

    int64_t num_elements() const {
        if (m_dims.empty()) return 1;
        int64_t count = 1;
        for (int64_t d : m_dims) {
            count *= d;
        }
        return count;
    }

    void AddDim(int64_t size) { m_dims.push_back(size); }
    void set_dim(int d, int64_t size) {
        if (d >= 0 && d < static_cast<int>(m_dims.size())) {
            m_dims[d] = size;
        }
    }

    const std::vector<int64_t>& dim_sizes() const { return m_dims; }

    ice::Shape to_ice() const {
        return ice::Shape(m_dims);
    }

    static TensorShape from_ice(const ice::Shape& shape) {
        return TensorShape(shape);
    }

    bool operator==(const TensorShape& other) const {
        return m_dims == other.m_dims;
    }
    bool operator!=(const TensorShape& other) const {
        return m_dims != other.m_dims;
    }

    std::string DebugString() const {
        std::string s = "[";
        for (size_t i = 0; i < m_dims.size(); ++i) {
            if (i > 0) s += ",";
            s += std::to_string(m_dims[i]);
        }
        s += "]";
        return s;
    }

private:
    std::vector<int64_t> m_dims;
};

inline ice::Shape TensorShapeToIce(const TensorShape& shape) {
    return shape.to_ice();
}

inline TensorShape TensorShapeFromIce(const ice::Shape& ice_shape) {
    return TensorShape::from_ice(ice_shape);
}

} // namespace tensorflow

} // export
