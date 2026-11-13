module;

export module aten_xpu_extern_gradients:matmul_linear;

import std;
import cc_ice_intern_sonic;
import aten_xpu_intern;
import :emitter;
import :gradient;

export namespace aten_xpu {

// Linear: output = input @ weight^T; fused epilogues and extra operands have no gradient here.
class SyclLinearGradient : public SyclGradient
{
public:
    explicit SyclLinearGradient(const SyclOpsTable& ops) noexcept :
        SyclGradient{ops, "SyclLinearGradient"}
    {
    }

protected:
    void emit(SyclGradientEmitter& emitter) override
    {
        if (emitter.getInputCount() != 2 || emitter.readString("unary", "none") != "none" ||
            emitter.readString("binary", "none") != "none") {
            emitter.fail(
                ice::sonic::Code::k_unimplemented,
                "Linear gradient supports only the plain product"
            );
            return;
        }
        const auto input = emitter.input(0);
        const auto weight = emitter.input(1);
        const auto upstream = emitter.upstream(0);
        emitter.setResult(0, emitter.matmul(upstream, weight, false, false));
        emitter.setResult(1, emitter.matmul(upstream, input, true, false));
    }
};

} // namespace aten_xpu
