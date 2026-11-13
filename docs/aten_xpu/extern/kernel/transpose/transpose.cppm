module;

#include "include/c/intern/datatype.h"
#include "include/c/intern/tensor.h"

#include <sycl/sycl.hpp>

export module aten_xpu_extern_kernel:transpose;

import std;
import aten_xpu_intern;
import :kernel;
import :context;
import :construction;

export namespace aten_xpu {

inline constexpr std::string_view k_transpose_kernel_name = "Transpose";

// Permutes the axes of a tensor of any element type: the kernel moves whole elements, so one
// implementation per element size serves every dtype. Rank is limited to eight.
class SyclTransposeKernel : public SyclKernel<SyclTransposeKernel>
{
public:
    static constexpr std::size_t k_max_rank = 8;

    SyclTransposeKernel(const SyclOpsTable& ops, SyclKernelConstruction& construction) :
        SyclKernel<SyclTransposeKernel>{ops, k_transpose_kernel_name}
    {
        static_cast<void>(construction);
    }

    void compute(SyclKernelContext& context)
    {
        auto input = context.getInput(0);
        auto permutation = context.getInput(1);
        auto queue = context.getQueue();
        if (!input || !permutation || !queue) {
            context.propagate();
            return;
        }

        const auto& source_dims = input->get().getDims();
        const std::size_t rank = source_dims.size();
        if (rank == 0 || rank > k_max_rank || !readPermutation(*queue, *permutation, rank)) {
            context.fail(
                ice::sonic::Code::k_invalid_argument,
                "transpose expects a permutation of the input axes and a rank of at most eight"
            );
            return;
        }

        m_scratch_dims.assign(rank, 0);
        for (std::size_t axis = 0; axis < rank; ++axis) {
            m_scratch_dims[axis] = source_dims[static_cast<std::size_t>(m_permutation[axis])];
        }
        auto output = context.allocateOutput(0, input->get().getDtype(), m_scratch_dims);
        if (!output) {
            context.propagate();
            return;
        }

        const std::size_t element_bytes =
            input->get().getByteSize() / std::max<std::size_t>(input->get().element_count(), 1);
        launch(queue->get(), input->get(), output->get(), element_bytes);
    }

private:
    // The permutation is a small device or host tensor of int32 or int64; copy it to the host.
    bool readPermutation(sycl::queue& queue, SyclTensor& permutation, std::size_t rank)
    {
        if (permutation.element_count() != rank) {
            return false;
        }
        const std::size_t element_bytes = permutation.getByteSize() / rank;
        m_scratch_bytes.assign(permutation.getByteSize(), std::byte{0});
        queue.memcpy(m_scratch_bytes.data(), permutation.getData(), permutation.getByteSize())
            .wait();
        std::array<bool, k_max_rank> seen{};
        for (std::size_t axis = 0; axis < rank; ++axis) {
            std::int64_t value = 0;
            if (element_bytes == sizeof(std::int32_t)) {
                std::int32_t narrow = 0;
                std::memcpy(&narrow, m_scratch_bytes.data() + axis * element_bytes, element_bytes);
                value = narrow;
            } else if (element_bytes == sizeof(std::int64_t)) {
                std::memcpy(&value, m_scratch_bytes.data() + axis * element_bytes, element_bytes);
            } else {
                return false;
            }
            if (value < 0 || value >= static_cast<std::int64_t>(rank) ||
                seen[static_cast<std::size_t>(value)]) {
                return false;
            }
            seen[static_cast<std::size_t>(value)] = true;
            m_permutation[axis] = value;
        }
        return true;
    }

    // One output element per work item: unravel its index over the output dims, map each output
    // axis to the input axis it came from, and copy the element bytes.
    void launch(
        sycl::queue& queue,
        SyclTensor& input,
        SyclTensor& output,
        std::size_t element_bytes
    )
    {
        const std::size_t rank = input.getDims().size();
        std::array<std::int64_t, k_max_rank> output_dims{};
        std::array<std::int64_t, k_max_rank> input_strides{};
        std::int64_t stride = 1;
        for (std::size_t axis = rank; axis-- > 0;) {
            input_strides[axis] = stride;
            stride *= input.getDims()[axis];
        }
        std::array<std::int64_t, k_max_rank> mapped_strides{};
        for (std::size_t axis = 0; axis < rank; ++axis) {
            output_dims[axis] = m_scratch_dims[axis];
            mapped_strides[axis] = input_strides[static_cast<std::size_t>(m_permutation[axis])];
        }

        const auto* source = static_cast<const std::byte*>(input.getData());
        auto* target = static_cast<std::byte*>(output.getData());
        const auto count = static_cast<std::size_t>(output.element_count());
        queue.parallel_for(
            sycl::range<1>{count},
            [=](sycl::id<1> item)
            {
                std::int64_t remainder = static_cast<std::int64_t>(item[0]);
                std::int64_t from = 0;
                for (std::size_t axis = rank; axis-- > 0;) {
                    from += (remainder % output_dims[axis]) * mapped_strides[axis];
                    remainder /= output_dims[axis];
                }
                for (std::size_t byte = 0; byte < element_bytes; ++byte) {
                    target[item[0] * element_bytes + byte] =
                        source[static_cast<std::size_t>(from) * element_bytes + byte];
                }
            }
        );
    }

    std::array<std::int64_t, k_max_rank> m_permutation{};
    std::vector<std::int64_t> m_scratch_dims;
    std::vector<std::byte> m_scratch_bytes;
};

} // namespace aten_xpu
