// SYCL reference plugin — single-layer, single-direction LSTM forward (inference) via oneDNN.
//
// Not built by Bazel (docs/ only). Replaces mkldnn/RNN.cpp's lstm_onednn_xpu for the
// single-layer, unidirectional case: no per-layer loop, no bidirectional concat, no PyTorch
// flat-params-list unpacking (at::cat'ing w_ih_l0/w_ih_l0_reverse/... into oneDNN's ldgoi
// layout) — the caller is expected to hand in weights already shaped [1, 1, 4, hidden, input]
// (w_ih) / [1, 1, 4, hidden, hidden] (w_hh) / [1, 1, 4, hidden] (bias), the same per-layer shape
// RNN.cpp builds before calling oneDNN. Multi-layer support is a loop over this kernel with
// each layer's output tensor as the next layer's input, same as RNN.cpp does — not implemented
// as one kernel.

module;

#include "docs/aten_xpu/sycl_backend/kernels/kernel_context.h"
#include "include/c/extern/kernel/builder.h"

#include <oneapi/dnnl/dnnl.hpp>
#include <oneapi/dnnl/dnnl_sycl.hpp>

export module sycl_backend:kernels_rnn_lstm;

import std;
import :tensor;
import :stream;
import :engine_cache;

export namespace sycl_backend::kernels {

class RnnLstmKernel
{
public:
    RnnLstmKernel() = delete;

    // Input 0: input [seq, batch, input_size]. Inputs 1/2: initial hidden/cell state
    // [1, 1, batch, hidden]. Inputs 3/4/5: w_ih [1,1,4,hidden,input], w_hh
    // [1,1,4,hidden,hidden], bias [1,1,4,hidden]. Output 0: layer output
    // [seq, batch, hidden]. Outputs 1/2: final hidden/cell state.
    static void compute(void* plugin_data, TF_OpKernelContext* raw_context) noexcept
    {
        (void)plugin_data;

        KernelContextView ctx{raw_context};
        TF_Status status{};

        auto* input_handle = ctx.get_input(0, &status);
        auto* h0_handle = ctx.get_input(1, &status);
        auto* c0_handle = ctx.get_input(2, &status);
        auto* w_ih_handle = ctx.get_input(3, &status);
        auto* w_hh_handle = ctx.get_input(4, &status);
        auto* bias_handle = ctx.get_input(5, &status);
        if (input_handle == nullptr || h0_handle == nullptr || c0_handle == nullptr ||
            w_ih_handle == nullptr || w_hh_handle == nullptr || bias_handle == nullptr) {
            ctx.fail(&status);
            return;
        }

        auto* input = static_cast<SyclTensor*>(input_handle->plugin_data);
        const std::vector<int64_t> input_dims = shape_of(*input);
        const int64_t seq_length = input_dims[0];
        const int64_t batch = input_dims[1];
        const int64_t input_size = input_dims[2];

        auto* h0 = static_cast<SyclTensor*>(h0_handle->plugin_data);
        const int64_t hidden_size = shape_of(*h0).back();

        const std::vector<int64_t> output_dims{seq_length, batch, hidden_size};
        auto* output_handle = ctx.allocate_output(
            0,
            TF_FLOAT,
            output_dims.data(),
            static_cast<int>(output_dims.size()),
            static_cast<std::size_t>(seq_length * batch * hidden_size) *
                SyclTensor::element_size(TF_FLOAT),
            &status
        );

        const std::vector<int64_t> state_dims{1, 1, batch, hidden_size};
        auto* hy_handle = ctx.allocate_output(
            1,
            TF_FLOAT,
            state_dims.data(),
            static_cast<int>(state_dims.size()),
            static_cast<std::size_t>(batch * hidden_size) * SyclTensor::element_size(TF_FLOAT),
            &status
        );
        auto* cy_handle = ctx.allocate_output(
            2,
            TF_FLOAT,
            state_dims.data(),
            static_cast<int>(state_dims.size()),
            static_cast<std::size_t>(batch * hidden_size) * SyclTensor::element_size(TF_FLOAT),
            &status
        );

        if (output_handle == nullptr || hy_handle == nullptr || cy_handle == nullptr) {
            ctx.fail(&status);
            return;
        }

        auto* stream_handle = ctx.get_stream(&status);
        if (stream_handle == nullptr) {
            ctx.fail(&status);
            return;
        }
        auto* stream = static_cast<SyclStream*>(stream_handle->plugin_data);

        run_lstm(
            *stream,
            seq_length,
            batch,
            input_size,
            hidden_size,
            raw_data(*input),
            raw_data(*h0),
            raw_data(*c0),
            raw_data(*static_cast<SyclTensor*>(w_ih_handle->plugin_data)),
            raw_data(*static_cast<SyclTensor*>(w_hh_handle->plugin_data)),
            raw_data(*static_cast<SyclTensor*>(bias_handle->plugin_data)),
            raw_data(*static_cast<SyclTensor*>(output_handle->plugin_data)),
            raw_data(*static_cast<SyclTensor*>(hy_handle->plugin_data)),
            raw_data(*static_cast<SyclTensor*>(cy_handle->plugin_data))
        );
    }

private:
    static std::vector<int64_t> shape_of(SyclTensor& tensor)
    {
        int rank = 0;
        tensor.num_dims(&rank);

        std::vector<int64_t> dims(static_cast<std::size_t>(rank));
        for (int index = 0; index < rank; ++index) {
            tensor.dim(index, &dims[static_cast<std::size_t>(index)]);
        }
        return dims;
    }

    static void* raw_data(SyclTensor& tensor)
    {
        void* data = nullptr;
        tensor.tensor_data(&data);
        return data;
    }

    static void run_lstm(
        SyclStream& stream,
        int64_t seq_length,
        int64_t batch,
        int64_t input_size,
        int64_t hidden_size,
        void* input_data,
        void* h0_data,
        void* c0_data,
        void* w_ih_data,
        void* w_hh_data,
        void* bias_data,
        void* output_data,
        void* hy_data,
        void* cy_data
    )
    {
        using tag = dnnl::memory::format_tag;
        auto data_type = dnnl::memory::data_type::f32;

        sycl::queue& queue = stream.get_native_queue();
        dnnl::engine& engine =
            EngineCache::instance().get_engine(queue.get_device(), queue.get_context());
        dnnl::stream& dnnl_stream = EngineCache::instance().get_stream(engine, queue);

        dnnl::memory::desc src_layer_md{{seq_length, batch, input_size}, data_type, tag::tnc};
        dnnl::memory::desc src_iter_md{{1, 1, batch, hidden_size}, data_type, tag::ldnc};
        dnnl::memory::desc src_iter_c_md{{1, 1, batch, hidden_size}, data_type, tag::ldnc};
        dnnl::memory::desc weights_layer_md{
            {1, 1, input_size, 4, hidden_size},
            data_type,
            tag::ldgoi
        };
        dnnl::memory::desc weights_iter_md{
            {1, 1, hidden_size, 4, hidden_size},
            data_type,
            tag::ldgoi
        };
        dnnl::memory::desc bias_md{{1, 1, 4, hidden_size}, data_type, tag::ldgo};
        dnnl::memory::desc dst_layer_md{{seq_length, batch, hidden_size}, data_type, tag::tnc};
        dnnl::memory::desc dst_iter_md{{1, 1, batch, hidden_size}, data_type, tag::ldnc};
        dnnl::memory::desc dst_iter_c_md{{1, 1, batch, hidden_size}, data_type, tag::ldnc};

        dnnl::primitive_attr attributes;
        attributes.set_scratchpad_mode(dnnl::scratchpad_mode::user);

        auto primitive_desc = dnnl::lstm_forward::primitive_desc{
            engine,
            dnnl::prop_kind::forward_inference,
            dnnl::rnn_direction::unidirectional_left2right,
            src_layer_md,
            src_iter_md,
            src_iter_c_md,
            weights_layer_md,
            weights_iter_md,
            bias_md,
            dst_layer_md,
            dst_iter_md,
            dst_iter_c_md,
            attributes
        };
        dnnl::lstm_forward lstm{primitive_desc};

        std::unordered_map<int, dnnl::memory> arguments;
        arguments.emplace(DNNL_ARG_SRC_LAYER, dnnl::memory{src_layer_md, engine, input_data});
        arguments.emplace(DNNL_ARG_SRC_ITER, dnnl::memory{src_iter_md, engine, h0_data});
        arguments.emplace(DNNL_ARG_SRC_ITER_C, dnnl::memory{src_iter_c_md, engine, c0_data});
        arguments.emplace(DNNL_ARG_BIAS, dnnl::memory{bias_md, engine, bias_data});
        arguments.emplace(DNNL_ARG_DST_LAYER, dnnl::memory{dst_layer_md, engine, output_data});
        arguments.emplace(DNNL_ARG_DST_ITER, dnnl::memory{dst_iter_md, engine, hy_data});
        arguments.emplace(DNNL_ARG_DST_ITER_C, dnnl::memory{dst_iter_c_md, engine, cy_data});

        // oneDNN may want the RNN weights in a packed layout it chooses internally; reorder into
        // scratch USM if what was handed in does not already match, same as RNN.cpp does.
        void* weights_layer_scratch = nullptr;
        void* weights_iter_scratch = nullptr;

        const dnnl::memory::desc expected_layer_md = primitive_desc.weights_layer_desc();
        if (expected_layer_md != weights_layer_md) {
            weights_layer_scratch = sycl::malloc_device(expected_layer_md.get_size(), queue);
            dnnl::memory source{weights_layer_md, engine, w_ih_data};
            dnnl::memory target{expected_layer_md, engine, weights_layer_scratch};
            dnnl::reorder{source, target}.execute(dnnl_stream, source, target);
            arguments.emplace(DNNL_ARG_WEIGHTS_LAYER, target);
        } else {
            arguments.emplace(
                DNNL_ARG_WEIGHTS_LAYER,
                dnnl::memory{weights_layer_md, engine, w_ih_data}
            );
        }

        const dnnl::memory::desc expected_iter_md = primitive_desc.weights_iter_desc();
        if (expected_iter_md != weights_iter_md) {
            weights_iter_scratch = sycl::malloc_device(expected_iter_md.get_size(), queue);
            dnnl::memory source{weights_iter_md, engine, w_hh_data};
            dnnl::memory target{expected_iter_md, engine, weights_iter_scratch};
            dnnl::reorder{source, target}.execute(dnnl_stream, source, target);
            arguments.emplace(DNNL_ARG_WEIGHTS_ITER, target);
        } else {
            arguments.emplace(
                DNNL_ARG_WEIGHTS_ITER,
                dnnl::memory{weights_iter_md, engine, w_hh_data}
            );
        }

        const std::size_t scratchpad_size = primitive_desc.scratchpad_desc().get_size();
        void* scratchpad_data =
            scratchpad_size == 0 ? nullptr : sycl::malloc_device(scratchpad_size, queue);
        if (scratchpad_data != nullptr) {
            arguments.emplace(
                DNNL_ARG_SCRATCHPAD,
                dnnl::memory{primitive_desc.scratchpad_desc(), engine, scratchpad_data}
            );
        }

        dnnl::sycl_interop::execute(lstm, dnnl_stream, arguments);
        queue.wait();

        if (scratchpad_data != nullptr) {
            sycl::free(scratchpad_data, queue);
        }
        if (weights_layer_scratch != nullptr) {
            sycl::free(weights_layer_scratch, queue);
        }
        if (weights_iter_scratch != nullptr) {
            sycl::free(weights_iter_scratch, queue);
        }
    }
};

} // namespace sycl_backend::kernels
