module;

export module aten_xpu_extern_gradients:tensorflow_add;

import std;
import aten_xpu_intern;
import :emitter;
import :gradient;

export namespace aten_xpu {

// Add: both inputs receive the upstream gradient (TensorFlow's AddGradientFunction).
class SyclAddGradient : public SyclGradient
{
public:
    explicit SyclAddGradient(const SyclOpsTable& ops) noexcept :
        SyclGradient{ops, "SyclAddGradient"}
    {
    }

protected:
    void emit(SyclGradientEmitter& emitter) override
    {
        const auto upstream = emitter.upstream(0);
        emitter.setResult(0, upstream);
        emitter.setResult(1, upstream);
    }
};

} // namespace aten_xpu
