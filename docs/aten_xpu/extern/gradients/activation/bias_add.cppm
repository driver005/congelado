module;

export module aten_xpu_extern_gradients:tensorflow_bias_add;

import std;
import aten_xpu_intern;
import :emitter;
import :gradient;

export namespace aten_xpu {

// BiasAdd: the value takes the upstream gradient, the bias its BiasAddGrad reduction.
class SyclBiasAddGradient : public SyclGradient
{
public:
    explicit SyclBiasAddGradient(const SyclOpsTable& ops) noexcept :
        SyclGradient{ops, "SyclBiasAddGradient"}
    {
    }

protected:
    void emit(SyclGradientEmitter& emitter) override
    {
        const auto upstream = emitter.upstream(0);
        emitter.setResult(0, upstream);
        if (!emitter.begin("BiasAddGrad")) {
            return;
        }
        emitter.add(upstream);
        emitter.setType("T", emitter.getElementType());
        emitter.setString("data_format", emitter.readString("data_format", "NHWC"));
        if (emitter.finish() > 0) {
            emitter.setResult(1, emitter.result(0));
        }
    }
};

} // namespace aten_xpu
