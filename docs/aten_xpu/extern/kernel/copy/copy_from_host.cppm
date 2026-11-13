module;

#include "include/c/intern/datatype.h"
#include "include/c/intern/tensor.h"

#include <sycl/sycl.hpp>

export module aten_xpu_extern_kernel:copy_from_host;

import std;
import aten_xpu_intern;
import :kernel;
import :context;
import :construction;

export namespace aten_xpu {

inline constexpr std::string_view k_copy_from_host_kernel_name = "_CopyFromHostToXpu";

// Copies a host tensor back to the device (the swap in of the memory optimizer). The input is a
// host memory argument; the output is allocated on the device.
class SyclCopyFromHostKernel : public SyclKernel<SyclCopyFromHostKernel>
{
public:
    static constexpr std::array<std::string_view, 1> k_host_memory_args{"x"};

    SyclCopyFromHostKernel(const SyclOpsTable& ops, SyclKernelConstruction& construction) :
        SyclKernel<SyclCopyFromHostKernel>{ops, k_copy_from_host_kernel_name}
    {
        static_cast<void>(construction);
    }

    void compute(SyclKernelContext& context)
    {
        auto input = context.getInput(0);
        auto queue = context.getQueue();
        if (!input || !queue) {
            context.propagate();
            return;
        }
        auto output = context.allocateOutput(0, input->get().getDtype(), input->get().getDims());
        if (!output) {
            context.propagate();
            return;
        }
        queue->get()
            .memcpy(output->get().getData(), input->get().getData(), input->get().getByteSize())
            .wait();
    }
};

} // namespace aten_xpu
