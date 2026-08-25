module;

#include <cstddef>
#include <cstdint>
#include <cstring>

export module cc_tmp:tensor_tensor_matcher;

import std;
import cc_abi;
import :tensor_tensor;

export {

namespace tensorflow {

class TensorMatcher {
public:
    static bool Equal(const Tensor& a, const Tensor& b) {
        if (a.dtype() != b.dtype()) return false;
        if (a.shape() != b.shape()) return false;
        if (a.TotalBytes() != b.TotalBytes()) return false;
        if (a.data() == b.data()) return true;
        if (!a.data() || !b.data()) return false;
        return std::memcmp(a.data(), b.data(), a.TotalBytes()) == 0;
    }
};

} // namespace tensorflow

} // export
