module;

#include <string>
#include <memory>
#include <vector>

export module cc_tmp:kernel_op_kernel;

import std;
import cc_abi;
import :types_types;
import :tensor_tensor_shape;
import :tensor_tensor;

export {

namespace tensorflow {

using OpKernelConstruction = ice::OpKernelConstruction;
using OpKernelContext = ice::OpKernelContext;
using KernelDefBuilder = ice::KernelBuilder;

class OpKernel {
public:
    explicit OpKernel(OpKernelConstruction* context) {}
    virtual ~OpKernel() = default;

    virtual void Compute(OpKernelContext* context) = 0;
};

} // namespace tensorflow

} // export
