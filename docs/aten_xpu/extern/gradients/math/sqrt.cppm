module;

export module aten_xpu_extern_gradients:tensorflow_sqrt;

import std;
import aten_xpu_intern;
import :emitter;
import :gradient;

export namespace aten_xpu {

// Sqrt: SqrtGrad of the forward output and the upstream gradient.
class SyclSqrtGradient : public SyclGradient
{
public:
    explicit SyclSqrtGradient(const SyclOpsTable& ops) noexcept :
        SyclGradient{ops, "SyclSqrtGradient"}
    {
    }

protected:
    void emit(SyclGradientEmitter& emitter) override
    {
        const auto output = emitter.output(0);
        const auto upstream = emitter.upstream(0);
        emitter.setResult(0, emitter.call("SqrtGrad", {output, upstream}));
    }
};

} // namespace aten_xpu
