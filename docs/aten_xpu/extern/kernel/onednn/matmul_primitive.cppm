module;

#include <oneapi/dnnl/dnnl.hpp>
#include <sycl/sycl.hpp>

export module aten_xpu_extern_kernel:onednn_matmul_primitive;

import std;
import aten_xpu_intern;
import :onednn_memory_layout;
import :onednn_post_op_attributes;
import :onednn_primitive_cache;
import :onednn_primitive_executor;

export namespace aten_xpu {

class SyclMatmulPrimitive
{
public:
    using Entry = std::pair<dnnl::matmul::primitive_desc, dnnl::matmul>;

    static void setAllowTf32(bool allow) noexcept
    {
        s_allow_tf32 = allow;
    }

    static void setDeterministic(bool deterministic) noexcept
    {
        s_deterministic = deterministic;
    }

    static sycl::event
    run(sycl::queue& queue,
        const SyclTensor& first,
        const SyclTensor& second,
        std::optional<std::reference_wrapper<const SyclTensor>> bias,
        SyclTensor& destination,
        const SyclPostOpAttributes& post_ops,
        bool second_transposed = false)
    {
        const auto rank = destination.getDims().size();
        if (rank != 2 && rank != 3) {
            throw std::invalid_argument{"oneDNN matmul only supports 2D or 3D tensors"};
        }

        auto first_type = SyclOnednnLayout::data_type(first.getDtype()).value();
        auto second_type = SyclOnednnLayout::data_type(second.getDtype()).value();
        auto destination_type = SyclOnednnLayout::data_type(destination.getDtype()).value();
        if (first_type == dnnl::memory::data_type::bf16 &&
            second_type == dnnl::memory::data_type::f32) {
            second_type = dnnl::memory::data_type::bf16;
        } else if (
            first_type == dnnl::memory::data_type::f32 &&
            second_type == dnnl::memory::data_type::bf16
        ) {
            first_type = dnnl::memory::data_type::bf16;
        }

        const auto first_desc = matrix_desc(first, first_type, false);
        const auto second_desc = matrix_desc(second, second_type, second_transposed);
        const auto destination_desc = matrix_desc(destination, destination_type, false);
        const auto bias_desc =
            bias ? std::optional{bias_matrix_desc(bias->get(), destination)} : std::nullopt;

        dnnl::primitive_attr attributes = SyclPrimitiveExecutor::user_scratchpad_attributes();
        post_ops.apply(attributes);
        if (first_type == dnnl::memory::data_type::f32) {
            attributes.set_fpmath_mode(
                s_allow_tf32 ? dnnl::fpmath_mode::tf32 : dnnl::fpmath_mode::strict
            );
        }
        if (s_deterministic) {
            attributes.set_deterministic(true);
        }

        SyclPrimitiveExecutor executor{queue};
        auto& [primitive_desc, primitive] = s_cache.find_or_create(
            cache_key(first_desc, second_desc, bias_desc, destination_desc, attributes),
            [&]()
            {
                auto descriptor =
                    bias_desc
                        ? dnnl::matmul::
                              primitive_desc{executor.getEngine(), first_desc, second_desc, *bias_desc, destination_desc, attributes}
                        : dnnl::matmul::primitive_desc{
                              executor.getEngine(),
                              first_desc,
                              second_desc,
                              destination_desc,
                              attributes
                          };
                return Entry{descriptor, dnnl::matmul{descriptor}};
            }
        );

        executor.addArgument(DNNL_ARG_SRC, first_desc, first.getData());
        executor.addArgument(DNNL_ARG_WEIGHTS, second_desc, second.getData());
        executor.addArgument(DNNL_ARG_DST, destination_desc, destination.getData());
        if (bias_desc) {
            executor.addArgument(DNNL_ARG_BIAS, *bias_desc, bias->get().getData());
        }
        post_ops.add_binary_arguments(executor.getEngine(), executor.getArguments());
        return executor.execute(primitive, primitive_desc);
    }

private:
    static dnnl::memory::desc
    matrix_desc(const SyclTensor& tensor, dnnl::memory::data_type type, bool transposed)
    {
        dnnl::memory::dims dims(tensor.getDims().begin(), tensor.getDims().end());
        dnnl::memory::dims strides(tensor.getStrides().begin(), tensor.getStrides().end());
        if (transposed) {
            const auto rank = dims.size();
            std::swap(dims[rank - 1], dims[rank - 2]);
            std::swap(strides[rank - 1], strides[rank - 2]);
        }
        return dnnl::memory::desc{dims, type, strides};
    }

    static dnnl::memory::desc
    bias_matrix_desc(const SyclTensor& bias, const SyclTensor& destination)
    {
        const auto& output = destination.getDims();
        const auto rank = output.size();
        dnnl::memory::dims dims(rank, 1);
        const auto& bias_dims = bias.getDims();
        for (std::size_t index = 0; index < bias_dims.size() && index < rank; ++index) {
            dims[rank - 1 - index] = bias_dims[bias_dims.size() - 1 - index];
        }
        const auto type =
            SyclOnednnLayout::data_type(bias.getDtype()).value_or(dnnl::memory::data_type::f32);
        return dnnl::memory::desc{
            dims,
            type,
            SyclOnednnLayout::default_format(static_cast<int>(rank))
        };
    }

    static std::string cache_key(
        const dnnl::memory::desc& first,
        const dnnl::memory::desc& second,
        const std::optional<dnnl::memory::desc>& bias,
        const dnnl::memory::desc& destination,
        const dnnl::primitive_attr& attributes
    )
    {
        std::string key;
        for (const auto* desc: {&first, &second, &destination}) {
            append_desc(key, *desc);
        }
        if (bias) {
            append_desc(key, *bias);
        }
        key += std::format(
            "|post{}|fp{}",
            attributes.get_post_ops().len(),
            static_cast<int>(attributes.get_fpmath_mode())
        );
        return key;
    }

    static void append_desc(std::string& key, const dnnl::memory::desc& desc)
    {
        key += std::format("|t{}", static_cast<int>(desc.get_data_type()));
        for (const auto value: desc.get_dims()) {
            key += std::format(",{}", value);
        }
        key += ':';
        for (const auto value: desc.get_strides()) {
            key += std::format(",{}", value);
        }
    }

    static inline SyclPrimitiveCache<std::string, Entry> s_cache{};
    static inline bool s_allow_tf32{false};
    static inline bool s_deterministic{false};
};

} // namespace aten_xpu
