module;

#include "include/c/intern/datatype.h"
#include "include/c/intern/tensor.h"

#include <sycl/sycl.hpp>

export module aten_xpu_extern_kernel:scoped_allocator_concat;

import std;
import aten_xpu_intern;
import :kernel;
import :context;
import :construction;

export namespace aten_xpu {

inline constexpr std::string_view k_scoped_allocator_concat_kernel_name = "_ScopedAllocatorConcat";

// Packs the group's inputs one after another into a flat tensor of `total` elements. The executor
// is what could make the producers write into the backing buffer directly; the plugin is not told
// to, so the inputs are copied and the first input (the backing buffer) is not used.
class SyclScopedAllocatorConcatKernel : public SyclKernel<SyclScopedAllocatorConcatKernel>
{
public:
    SyclScopedAllocatorConcatKernel(const SyclOpsTable& ops, SyclKernelConstruction& construction) :
        SyclKernel<SyclScopedAllocatorConcatKernel>{ops, k_scoped_allocator_concat_kernel_name},
        m_total{construction.getInt64("total", 0)},
        m_type{construction.getType("T", ice::sonic::DataTypeEnum::k_float)}
    {
    }

    void compute(SyclKernelContext& context)
    {
        auto queue = context.getQueue();
        if (!queue) {
            context.propagate();
            return;
        }
        const std::array<std::int64_t, 1> dims{m_total};
        auto output = context.allocateOutput(0, m_type, dims);
        if (!output) {
            context.propagate();
            return;
        }

        std::size_t offset = 0;
        for (int slot = 1; slot < context.getInputCount(); ++slot) {
            auto input = context.getInput(slot);
            if (!input) {
                context.propagate();
                return;
            }
            const std::size_t bytes = input->get().getByteSize();
            if (offset + bytes > output->get().getByteSize()) {
                context.fail(
                    ice::sonic::Code::k_invalid_argument,
                    "inputs exceed the scoped buffer"
                );
                return;
            }
            queue->get().memcpy(
                static_cast<std::byte*>(output->get().getData()) + offset,
                input->get().getData(),
                bytes
            );
            offset += bytes;
        }
        queue->get().wait();
    }

private:
    std::int64_t m_total;
    ice::sonic::DataTypeEnum m_type;
};

} // namespace aten_xpu
