module;

export module aten_xpu_extern_gradients:plugin_addmm;

import std;
import cc_ice_intern_sonic;
import aten_xpu_intern;
import :emitter;
import :gradient;

export namespace aten_xpu {

// Addmm: output = beta * addend + alpha * first @ second; the activation epilogue has no gradient
// here.
class SyclAddmmGradient : public SyclGradient
{
public:
    explicit SyclAddmmGradient(const SyclOpsTable& ops) noexcept :
        SyclGradient{ops, "SyclAddmmGradient"}
    {
    }

protected:
    void emit(SyclGradientEmitter& emitter) override
    {
        if (emitter.readInt("activation", 0) != 0) {
            emitter.fail(
                ice::sonic::Code::k_unimplemented,
                "Addmm gradient does not support a fused activation"
            );
            return;
        }
        const float alpha = emitter.readFloat("alpha", 1.0F);
        const float beta = emitter.readFloat("beta", 1.0F);
        const auto addend = emitter.input(0);
        const auto first = emitter.input(1);
        const auto second = emitter.input(2);
        const auto upstream = emitter.upstream(0);
        SyclGradientValue scaled;
        if (alpha == 1.0F) {
            scaled = emitter.call("Identity", {upstream});
        } else {
            const auto factor = emitter.constant(alpha);
            scaled = emitter.call("Mul", {upstream, factor});
        }
        emitter.setResult(1, emitter.matmul(scaled, second, false, true));
        emitter.setResult(2, emitter.matmul(first, scaled, true, false));
        SyclGradientValue shifted;
        if (beta == 1.0F) {
            shifted = emitter.call("Identity", {upstream});
        } else {
            const auto factor = emitter.constant(beta);
            shifted = emitter.call("Mul", {upstream, factor});
        }
        emitter.setResult(0, emitter.reduceTo(shifted, addend));
    }
};

} // namespace aten_xpu
