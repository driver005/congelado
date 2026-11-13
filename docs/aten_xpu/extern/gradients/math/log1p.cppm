module;

export module aten_xpu_extern_gradients:math_log1p;

import std;
import aten_xpu_intern;
import :emitter;
import :gradient;

export namespace aten_xpu {

// Log1p: the upstream gradient divided by one plus the input.
class SyclLog1pGradient : public SyclGradient
{
public:
    explicit SyclLog1pGradient(const SyclOpsTable& ops) noexcept :
        SyclGradient{ops, "SyclLog1pGradient"}
    {
    }

protected:
    void emit(SyclGradientEmitter& emitter) override
    {
        const auto one = emitter.constant(1.0);
        const auto input = emitter.input(0);
        const auto upstream = emitter.upstream(0);
        const auto shifted = emitter.call("Add", {one, input});
        emitter.setResult(0, emitter.call("DivNoNan", {upstream, shifted}));
    }
};

} // namespace aten_xpu
