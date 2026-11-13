module;

export module aten_xpu_extern_gradients:math_exp;

import std;
import aten_xpu_intern;
import :emitter;
import :gradient;

export namespace aten_xpu {

// Exp: the upstream gradient times the forward output.
class SyclExpGradient : public SyclGradient
{
public:
    explicit SyclExpGradient(const SyclOpsTable& ops) noexcept :
        SyclGradient{ops, "SyclExpGradient"}
    {
    }

protected:
    void emit(SyclGradientEmitter& emitter) override
    {
        const auto upstream = emitter.upstream(0);
        const auto output = emitter.output(0);
        emitter.setResult(0, emitter.call("Mul", {upstream, output}));
    }
};

} // namespace aten_xpu
