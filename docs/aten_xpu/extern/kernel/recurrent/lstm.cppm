module;

#include "include/c/intern/datatype.h"

#include <oneapi/dnnl/dnnl.hpp>
#include <sycl/sycl.hpp>

export module aten_xpu_extern_kernel:recurrent_lstm;

import std;
import aten_xpu_intern;
import :kernel;
import :context;
import :construction;
import :onednn_memory_layout;
import :onednn_engine_cache;
import :onednn_primitive_executor;

export namespace aten_xpu {

class SyclLstmKernel : public SyclKernel<SyclLstmKernel>
{
public:
    static constexpr std::string_view k_name = "LSTM";
    static constexpr int k_tensors_per_direction = 3;
    static constexpr int k_first_weight = 3;

    SyclLstmKernel(const SyclOpsTable& ops, SyclKernelConstruction& construction) :
        SyclKernel<SyclLstmKernel>{ops},
        m_layers{construction.getInt64("num_layers", 1)},
        m_directions{construction.getBool("bidirectional", false) ? 2 : 1}
    {
    }

    void compute(SyclKernelContext& context)
    {

        auto input = context.getInput(0);
        auto initial_hidden = context.getInput(1);
        auto initial_cell = context.getInput(2);
        auto queue = context.getQueue();
        if (!input || !initial_hidden || !initial_cell || !queue) {
            context.propagate();
            return;
        }
        const int expected_inputs = k_first_weight + static_cast<int>(m_layers * m_directions) * k_tensors_per_direction;
        if (context.getInputCount() < expected_inputs) {
            context.fail(TF_INVALID_ARGUMENT, "LSTM expects w_ih, w_hh and bias per layer and direction");
            return;
        }

        const auto& input_dims = input->get().getDims();
        const int64_t length = input_dims[0];
        const int64_t batch = input_dims[1];
        const int64_t hidden = initial_hidden->get().getDims()[2];
        const auto dtype = input->get().getDtype();

        m_scratch_dims.assign({length, batch, m_directions * hidden});
        auto output = context.allocateOutput(0, dtype, m_scratch_dims);
        auto final_hidden = context.allocateOutput(1, dtype, initial_hidden->get().getDims());
        auto final_cell = context.allocateOutput(2, dtype, initial_cell->get().getDims());
        if (!output || !final_hidden || !final_cell) {
            context.propagate();
            return;
        }

        const auto* layer_input = input->get().getData();
        int64_t layer_input_size = input_dims[2];
        std::array<std::optional<std::reference_wrapper<SyclTensor>>, 2> buffers;
        for (int64_t layer = 0; layer < m_layers; ++layer) {
            void* layer_output = output->get().getData();
            if (layer + 1 < m_layers) {
                auto& slot = buffers[layer % 2];
                if (!slot) {
                    slot = context.allocateTemp(dtype, m_scratch_dims);
                    if (!slot) {
                        context.propagate();
                        return;
                    }
                }
                layer_output = slot->get().getData();
            }

            auto weights = pack_weights(context, queue->get(), layer, layer_input_size, hidden, dtype);
            if (!weights) {
                context.propagate();
                return;
            }
            run_layer(
                queue->get(),
                layer,
                length,
                batch,
                layer_input_size,
                hidden,
                dtype,
                layer_input,
                initial_hidden->get(),
                initial_cell->get(),
                *weights,
                layer_output,
                final_hidden->get(),
                final_cell->get()
            );

            layer_input = layer_output;
            layer_input_size = m_directions * hidden;
        }

    }

private:
    using PackedWeights = std::array<std::reference_wrapper<SyclTensor>, 3>;

    std::optional<PackedWeights> pack_weights(
        SyclKernelContext& context,
        sycl::queue& queue,
        int64_t layer,
        int64_t input_size,
        int64_t hidden,
        TFDataTypeEnum dtype
    )
    {

        const std::array<int64_t, 2> layer_dims{m_directions, 4 * hidden * input_size};
        const std::array<int64_t, 2> iter_dims{m_directions, 4 * hidden * hidden};
        const std::array<int64_t, 2> bias_dims{m_directions, 4 * hidden};
        auto weights_layer = context.allocateTemp(dtype, layer_dims);
        auto weights_iter = context.allocateTemp(dtype, iter_dims);
        auto bias = context.allocateTemp(dtype, bias_dims);
        if (!weights_layer || !weights_iter || !bias) {
            return std::nullopt;
        }

        const auto item = SyclTensor::element_size(dtype);
        for (int64_t direction = 0; direction < m_directions; ++direction) {
            const int base = k_first_weight + static_cast<int>((layer * m_directions + direction) * k_tensors_per_direction);
            auto input_weight = context.getInput(base);
            auto hidden_weight = context.getInput(base + 1);
            auto direction_bias = context.getInput(base + 2);
            if (!input_weight || !hidden_weight || !direction_bias) {
                return std::nullopt;
            }
            copy_slice(queue, weights_layer->get(), direction, input_weight->get(), item);
            copy_slice(queue, weights_iter->get(), direction, hidden_weight->get(), item);
            copy_slice(queue, bias->get(), direction, direction_bias->get(), item);
        }
        return PackedWeights{weights_layer->get(), weights_iter->get(), bias->get()};

    }

    static void copy_slice(sycl::queue& queue, SyclTensor& destination, int64_t slot, const SyclTensor& source, std::size_t item)
    {

        const auto bytes = static_cast<std::size_t>(source.element_count()) * item;
        queue.memcpy(static_cast<std::byte*>(destination.getData()) + static_cast<std::size_t>(slot) * bytes, source.getData(), bytes);

    }

    void run_layer(
        sycl::queue& queue,
        int64_t layer,
        int64_t length,
        int64_t batch,
        int64_t input_size,
        int64_t hidden,
        TFDataTypeEnum dtype,
        const void* layer_input,
        const SyclTensor& initial_hidden,
        const SyclTensor& initial_cell,
        const PackedWeights& weights,
        void* layer_output,
        SyclTensor& final_hidden,
        SyclTensor& final_cell
    )
    {

        using tag = dnnl::memory::format_tag;
        const auto type = SyclOnednnLayout::data_type(dtype).value();
        const auto direction = m_directions == 2 ? dnnl::rnn_direction::bidirectional_concat
                                                 : dnnl::rnn_direction::unidirectional_left2right;

        const dnnl::memory::desc source_layer{{length, batch, input_size}, type, tag::tnc};
        const dnnl::memory::desc state{{1, m_directions, batch, hidden}, type, tag::ldnc};
        const dnnl::memory::desc weights_layer{{1, m_directions, input_size, 4, hidden}, type, tag::ldgoi};
        const dnnl::memory::desc weights_iter{{1, m_directions, hidden, 4, hidden}, type, tag::ldgoi};
        const dnnl::memory::desc bias{{1, m_directions, 4, hidden}, type, tag::ldgo};
        const dnnl::memory::desc destination_layer{{length, batch, m_directions * hidden}, type, tag::tnc};

        SyclPrimitiveExecutor executor{queue};
        const auto weights_layer_any = SyclOnednnLayout::any_desc(weights_layer);
        const auto weights_iter_any = SyclOnednnLayout::any_desc(weights_iter);
        const dnnl::lstm_forward::primitive_desc primitive_desc{
            executor.getEngine(),
            dnnl::prop_kind::forward_inference,
            direction,
            source_layer,
            state,
            state,
            weights_layer_any,
            weights_iter_any,
            bias,
            destination_layer,
            state,
            state,
            SyclPrimitiveExecutor::user_scratchpad_attributes()
        };

        const auto state_offset = static_cast<std::size_t>(layer * m_directions * batch * hidden) * SyclTensor::element_size(dtype);
        executor.addArgument(DNNL_ARG_SRC_LAYER, source_layer, layer_input);
        executor.addArgument(DNNL_ARG_SRC_ITER, state, static_cast<const std::byte*>(initial_hidden.getData()) + state_offset);
        executor.addArgument(DNNL_ARG_SRC_ITER_C, state, static_cast<const std::byte*>(initial_cell.getData()) + state_offset);
        executor.addArgument(DNNL_ARG_BIAS, bias, weights[2].get().getData());
        executor.addArgument(DNNL_ARG_DST_LAYER, destination_layer, layer_output);
        executor.addArgument(DNNL_ARG_DST_ITER, state, static_cast<std::byte*>(final_hidden.getData()) + state_offset);
        executor.addArgument(DNNL_ARG_DST_ITER_C, state, static_cast<std::byte*>(final_cell.getData()) + state_offset);
        add_reordered(executor, queue, DNNL_ARG_WEIGHTS_LAYER, weights_layer, primitive_desc.weights_layer_desc(), weights[0].get());
        add_reordered(executor, queue, DNNL_ARG_WEIGHTS_ITER, weights_iter, primitive_desc.weights_iter_desc(), weights[1].get());
        executor.execute(dnnl::lstm_forward{primitive_desc}, primitive_desc);

    }

    void add_reordered(
        SyclPrimitiveExecutor& executor,
        sycl::queue& queue,
        int argument,
        const dnnl::memory::desc& plain,
        const dnnl::memory::desc& expected,
        SyclTensor& weights
    )
    {

        if (plain == expected) {
            executor.addArgument(argument, plain, weights.getData());
            return;
        }
        auto& stream = SyclEngineCache::getInstance().getStream(queue);
        dnnl::memory source{plain, executor.getEngine(), weights.getData()};
        dnnl::memory target{expected, executor.getEngine()};
        dnnl::reorder{source, target}.execute(stream, source, target);
        executor.getArguments().insert_or_assign(argument, target);

    }

    int64_t m_layers;
    int64_t m_directions;
    std::vector<int64_t> m_scratch_dims;
};

} // namespace aten_xpu
