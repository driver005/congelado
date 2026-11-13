module;

export module aten_xpu_extern_gradients:math_subtract;

import std;
import aten_xpu_intern;
import :emitter;
import :gradient;

export namespace aten_xpu {

// Sub: the minuend takes the upstream gradient, the subtrahend its negation.
class SyclSubtractGradient : public SyclGradient
{
public:
    explicit SyclSubtractGradient(const SyclOpsTable& ops) noexcept :
        SyclGradient{ops, "SyclSubtractGradient"}
    {
    }

protected:
    void emit(SyclGradientEmitter& emitter) override
    {
        const auto upstream = emitter.upstream(0);
        emitter.setResult(0, upstream);
        emitter.setResult(1, emitter.call("Neg", {upstream}));
    }
};

} // namespace aten_xpu
