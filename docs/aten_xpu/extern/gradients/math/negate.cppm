module;

export module aten_xpu_extern_gradients:math_negate;

import std;
import aten_xpu_intern;
import :emitter;
import :gradient;

export namespace aten_xpu {

// Neg: the upstream gradient negated.
class SyclNegateGradient : public SyclGradient
{
public:
    explicit SyclNegateGradient(const SyclOpsTable& ops) noexcept :
        SyclGradient{ops, "SyclNegateGradient"}
    {
    }

protected:
    void emit(SyclGradientEmitter& emitter) override
    {
        const auto upstream = emitter.upstream(0);
        emitter.setResult(0, emitter.call("Neg", {upstream}));
    }
};

} // namespace aten_xpu
