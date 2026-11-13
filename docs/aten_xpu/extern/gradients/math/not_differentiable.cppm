module;

export module aten_xpu_extern_gradients:math_not_differentiable;

import std;
import aten_xpu_intern;
import :emitter;
import :gradient;

export namespace aten_xpu {

// An op with no gradient (RegisterNotDifferentiable): no input receives one.
class SyclNotDifferentiableGradient : public SyclGradient
{
public:
    explicit SyclNotDifferentiableGradient(const SyclOpsTable& ops) noexcept :
        SyclGradient{ops, "SyclNotDifferentiableGradient"}
    {
    }

protected:
    void emit(SyclGradientEmitter& emitter) override
    {
        static_cast<void>(emitter);
    }
};

} // namespace aten_xpu
