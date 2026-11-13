module;

#include "include/c/intern/datatype.h"
#include "include/c/intern/tensor.h"

#include <sycl/sycl.hpp>

export module aten_xpu_extern_kernel:copy_to_host;

import std;
import aten_xpu_intern;
import :kernel;
import :context;
import :construction;

export namespace aten_xpu {

inline constexpr std::string_view k_copy_to_host_kernel_name = "_CopyFromXpuToHost";

// Copies a device tensor into host memory (the swap out of the memory optimizer). The output is
// registered as a host memory argument, so the context allocates it in host memory.
class SyclCopyToHostKernel : public SyclKernel<SyclCopyToHostKernel>
{
public:
    static constexpr std::array<std::string_view, 1> k_host_memory_args{"y"};

    SyclCopyToHostKernel(const SyclOpsTable& ops, SyclKernelConstruction& construction) :
        SyclKernel<SyclCopyToHostKernel>{ops, k_copy_to_host_kernel_name}
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
