module;

export module aten_xpu_extern_gradients:tensorflow_sparse_softmax_cross_entropy;

import std;
import aten_xpu_intern;
import :emitter;
import :gradient;

export namespace aten_xpu {

// SparseSoftmaxCrossEntropyWithLogits: the logits take the loss gradient expanded and times the
// backprop; labels get none.
class SyclSparseSoftmaxCrossEntropyGradient : public SyclGradient
{
public:
    explicit SyclSparseSoftmaxCrossEntropyGradient(const SyclOpsTable& ops) noexcept :
        SyclGradient{ops, "SyclSparseSoftmaxCrossEntropyGradient"}
    {
    }

protected:
    void emit(SyclGradientEmitter& emitter) override
    {
        const auto axis = emitter.constant(-1.0, ice::sonic::DataTypeEnum::k_int32);
        if (!emitter.begin("ExpandDims")) {
            return;
        }
        emitter.add(emitter.upstream(0));
        emitter.add(axis);
        emitter.setType("T", emitter.getElementType());
        emitter.setType("Tdim", ice::sonic::DataTypeEnum::k_int32);
        if (emitter.finish() < 1) {
            return;
        }
        const auto expanded = emitter.result(0);
        const auto backprop = emitter.output(1);
        emitter.setResult(0, emitter.call("Mul", {expanded, backprop}));
    }
};

} // namespace aten_xpu
