module;

#include "include/c/intern/datatype.h"
#include "include/c/intern/tensor.h"

export module aten_xpu_extern_kernel:scoped_allocator;

import std;
import aten_xpu_intern;
import :kernel;
import :context;
import :construction;

export namespace aten_xpu {

inline constexpr std::string_view k_scoped_allocator_kernel_name = "_ScopedAllocator";

// Makes the backing buffer of a scoped allocator group: one flat tensor of `total` elements.
class SyclScopedAllocatorKernel : public SyclKernel<SyclScopedAllocatorKernel>
{
public:
    SyclScopedAllocatorKernel(const SyclOpsTable& ops, SyclKernelConstruction& construction) :
        SyclKernel<SyclScopedAllocatorKernel>{ops, k_scoped_allocator_kernel_name},
        m_total{construction.getInt64("total", 0)},
        m_type{construction.getType("T", ice::sonic::DataTypeEnum::k_float)}
    {
    }

    void compute(SyclKernelContext& context)
    {
        const std::array<std::int64_t, 1> dims{m_total};
        if (!context.allocateOutput(0, m_type, dims)) {
            context.propagate();
        }
    }

private:
    std::int64_t m_total;
    ice::sonic::DataTypeEnum m_type;
};

} // namespace aten_xpu
