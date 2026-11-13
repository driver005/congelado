module;

export module aten_xpu_extern_gradients:matmul_matmul;

import std;
import aten_xpu_intern;
import :emitter;
import :gradient;

export namespace aten_xpu {

// MatMul: the four transpose cases of TensorFlow's MatMulGradientFunction.
class SyclMatMulGradient : public SyclGradient
{
public:
    explicit SyclMatMulGradient(const SyclOpsTable& ops) noexcept :
        SyclGradient{ops, "SyclMatMulGradient"}
    {
    }

protected:
    void emit(SyclGradientEmitter& emitter) override
    {
        const bool transpose_first = emitter.readBool("transpose_a", false);
        const bool transpose_second = emitter.readBool("transpose_b", false);
        const auto upstream = emitter.upstream(0);
        const auto first = emitter.input(0);
        const auto second = emitter.input(1);
        if (!transpose_first && !transpose_second) {
            emitter.setResult(0, emitter.matmul(upstream, second, false, true));
            emitter.setResult(1, emitter.matmul(first, upstream, true, false));
        } else if (!transpose_first && transpose_second) {
            emitter.setResult(0, emitter.matmul(upstream, second, false, false));
            emitter.setResult(1, emitter.matmul(upstream, first, true, false));
        } else if (transpose_first && !transpose_second) {
            emitter.setResult(0, emitter.matmul(second, upstream, false, true));
            emitter.setResult(1, emitter.matmul(first, upstream, false, false));
        } else {
            emitter.setResult(0, emitter.matmul(second, upstream, true, true));
            emitter.setResult(1, emitter.matmul(upstream, first, true, true));
        }
    }
};

} // namespace aten_xpu
