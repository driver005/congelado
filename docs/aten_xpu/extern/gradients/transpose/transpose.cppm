module;

export module aten_xpu_extern_gradients:transpose_transpose;

import std;
import cc_ice_intern_sonic;
import aten_xpu_intern;
import :emitter;
import :gradient;

export namespace aten_xpu {

// Transpose: the upstream gradient transposed by the inverse permutation; the permutation gets
// none.
class SyclTransposeGradient : public SyclGradient
{
public:
    explicit SyclTransposeGradient(const SyclOpsTable& ops) noexcept :
        SyclGradient{ops, "SyclTransposeGradient"}
    {
    }

protected:
    void emit(SyclGradientEmitter& emitter) override
    {
        const auto permutation = emitter.input(1);
        const auto upstream = emitter.upstream(0);
        ice::sonic::DataTypeEnum permutation_type{ice::sonic::DataTypeEnum::k_int32};
        emitter.readType("Tperm", permutation_type);
        if (!emitter.begin("InvertPermutation")) {
            return;
        }
        emitter.add(permutation);
        emitter.setType("T", permutation_type);
        if (emitter.finish() < 1) {
            return;
        }
        const auto inverse = emitter.result(0);
        if (!emitter.begin("Transpose")) {
            return;
        }
        emitter.add(upstream);
        emitter.add(inverse);
        emitter.setType("T", emitter.getElementType());
        emitter.setType("Tperm", permutation_type);
        if (emitter.finish() > 0) {
            emitter.setResult(0, emitter.result(0));
        }
    }
};

} // namespace aten_xpu
