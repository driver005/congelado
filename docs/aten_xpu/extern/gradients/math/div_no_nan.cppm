module;

export module aten_xpu_extern_gradients:math_div_no_nan;

import std;
import aten_xpu_intern;
import :emitter;
import :gradient;

export namespace aten_xpu {

// DivNoNan: the numerator takes upstream / y, the denominator takes upstream * (-x / y) / y.
class SyclDivNoNanGradient : public SyclGradient
{
public:
    explicit SyclDivNoNanGradient(const SyclOpsTable& ops) noexcept :
        SyclGradient{ops, "SyclDivNoNanGradient"}
    {
    }

protected:
    void emit(SyclGradientEmitter& emitter) override
    {
        const auto upstream = emitter.upstream(0);
        const auto numerator = emitter.input(0);
        const auto denominator = emitter.input(1);
        emitter.setResult(0, emitter.call("DivNoNan", {upstream, denominator}));
        const auto negated = emitter.call("Neg", {numerator});
        const auto once = emitter.call("DivNoNan", {negated, denominator});
        const auto twice = emitter.call("DivNoNan", {once, denominator});
        emitter.setResult(1, emitter.call("Mul", {upstream, twice}));
    }
};

} // namespace aten_xpu
