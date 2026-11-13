module;

export module aten_xpu_extern_gradients:plugin_attention;

import std;
import cc_ice_intern_sonic;
import aten_xpu_intern;
import :emitter;
import :gradient;

export namespace aten_xpu {

// ScaledDotProductAttention: ScaledDotProductAttentionGrad yields the query, key and value
// derivatives; dropout and extra operands have no gradient here.
class SyclAttentionGradient : public SyclGradient
{
public:
    explicit SyclAttentionGradient(const SyclOpsTable& ops) noexcept :
        SyclGradient{ops, "SyclAttentionGradient"}
    {
    }

protected:
    void emit(SyclGradientEmitter& emitter) override
    {
        if (emitter.getInputCount() != 3 || emitter.readFloat("dropout", 0.0F) != 0.0F) {
            emitter.fail(
                ice::sonic::Code::k_unimplemented,
                "attention gradient supports neither dropout nor extra operands"
            );
            return;
        }
        const auto query = emitter.input(0);
        const auto key = emitter.input(1);
        const auto value = emitter.input(2);
        const auto upstream = emitter.upstream(0);
        if (!emitter.begin("ScaledDotProductAttentionGrad")) {
            return;
        }
        emitter.add(upstream);
        emitter.add(query);
        emitter.add(key);
        emitter.add(value);
        emitter.setType("T", emitter.getElementType());
        emitter.setFloat("scale", emitter.readFloat("scale", 0.0F));
        emitter.setBool("is_causal", emitter.readBool("is_causal", false));
        if (emitter.finish() < 3) {
            return;
        }
        emitter.setResult(0, emitter.result(0));
        emitter.setResult(1, emitter.result(1));
        emitter.setResult(2, emitter.result(2));
    }
};

} // namespace aten_xpu
