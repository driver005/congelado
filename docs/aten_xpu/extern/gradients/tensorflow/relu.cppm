module;

export module aten_xpu_extern_gradients:tensorflow_relu;

import std;
import aten_xpu_intern;
import :emitter;
import :gradient;

export namespace aten_xpu {

// Relu: ReluGrad of the upstream gradient and the forward output.
class SyclReluGradient : public SyclGradient
{
public:
    explicit SyclReluGradient(const SyclOpsTable& ops) noexcept :
        SyclGradient{ops, "SyclReluGradient"}
    {
    }

protected:
    void emit(SyclGradientEmitter& emitter) override
    {
        const auto upstream = emitter.upstream(0);
        const auto output = emitter.output(0);
        emitter.setResult(0, emitter.call("ReluGrad", {upstream, output}));
    }
};

} // namespace aten_xpu
