module;

export module aten_xpu_extern_gradients:math_identity_n;

import std;
import aten_xpu_intern;
import :emitter;
import :gradient;

export namespace aten_xpu {

// IdentityN: every input takes the upstream gradient of the same position.
class SyclIdentityNGradient : public SyclGradient
{
public:
    explicit SyclIdentityNGradient(const SyclOpsTable& ops) noexcept :
        SyclGradient{ops, "SyclIdentityNGradient"}
    {
    }

protected:
    void emit(SyclGradientEmitter& emitter) override
    {
        const int count = emitter.getUpstreamCount();
        for (int index = 0; index < count; ++index) {
            emitter.setResult(index, emitter.upstream(index));
        }
    }
};

} // namespace aten_xpu
