module;

export module aten_xpu_extern_gradients:math_multiply;

import std;
import aten_xpu_intern;
import :emitter;
import :gradient;

export namespace aten_xpu {

// Mul: each input receives the upstream gradient times the other input.
class SyclMultiplyGradient : public SyclGradient
{
public:
    explicit SyclMultiplyGradient(const SyclOpsTable& ops) noexcept :
        SyclGradient{ops, "SyclMultiplyGradient"}
    {
    }

protected:
    void emit(SyclGradientEmitter& emitter) override
    {
        const auto upstream = emitter.upstream(0);
        const auto first = emitter.input(0);
        const auto second = emitter.input(1);
        emitter.setResult(0, emitter.call("Mul", {upstream, second}));
        emitter.setResult(1, emitter.call("Mul", {upstream, first}));
    }
};

} // namespace aten_xpu
