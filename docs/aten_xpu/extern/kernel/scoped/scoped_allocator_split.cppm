module;

#include "include/c/intern/datatype.h"
#include "include/c/intern/tensor.h"

#include <sycl/sycl.hpp>

export module aten_xpu_extern_kernel:scoped_allocator_split;

import std;
import aten_xpu_intern;
import :kernel;
import :context;
import :construction;

export namespace aten_xpu {

inline constexpr std::string_view k_scoped_allocator_split_kernel_name = "_ScopedAllocatorSplit";

// Hands every original consumer its slice of the flat tensor: output i has the shape given by the
// `ranks` and `dims` attrs (all dims back to back) and receives its bytes from the buffer.
class SyclScopedAllocatorSplitKernel : public SyclKernel<SyclScopedAllocatorSplitKernel>
{
public:
    static constexpr std::size_t k_max_outputs = 64;
    static constexpr std::size_t k_max_dims = 512;

    SyclScopedAllocatorSplitKernel(const SyclOpsTable& ops, SyclKernelConstruction& construction) :
        SyclKernel<SyclScopedAllocatorSplitKernel>{ops, k_scoped_allocator_split_kernel_name},
        m_type{construction.getType("T", ice::sonic::DataTypeEnum::k_float)}
    {
        m_output_count = construction.getInt64List("ranks", m_ranks);
        m_dim_count = construction.getInt64List("dims", m_dims);
    }

    void compute(SyclKernelContext& context)
    {
        auto input = context.getInput(0);
        auto queue = context.getQueue();
        if (!input || !queue) {
            context.propagate();
            return;
        }

        std::size_t offset = 0;
        std::size_t dim_cursor = 0;
        for (std::size_t slot = 0; slot < m_output_count; ++slot) {
            const auto rank = static_cast<std::size_t>(m_ranks[slot]);
            if (dim_cursor + rank > m_dim_count) {
                context.fail(ice::sonic::Code::k_invalid_argument, "dims do not match ranks");
                return;
            }
            const std::span<const std::int64_t> dims{m_dims.data() + dim_cursor, rank};
            dim_cursor += rank;
            auto output = context.allocateOutput(static_cast<int>(slot), m_type, dims);
            if (!output) {
                context.propagate();
                return;
            }
            const std::size_t bytes = output->get().getByteSize();
            if (offset + bytes > input->get().getByteSize()) {
                context.fail(
                    ice::sonic::Code::k_invalid_argument,
                    "slices exceed the scoped buffer"
                );
                return;
            }
            queue->get().memcpy(
                output->get().getData(),
                static_cast<const std::byte*>(input->get().getData()) + offset,
                bytes
            );
            offset += bytes;
        }
        queue->get().wait();
    }

private:
    ice::sonic::DataTypeEnum m_type;
    std::array<std::int64_t, k_max_outputs> m_ranks{};
    std::array<std::int64_t, k_max_dims> m_dims{};
    std::size_t m_output_count{0};
    std::size_t m_dim_count{0};
};

} // namespace aten_xpu
