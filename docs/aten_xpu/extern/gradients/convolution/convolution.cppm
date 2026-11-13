module;

export module aten_xpu_extern_gradients:plugin_convolution;

import std;
import cc_ice_intern_sonic;
import aten_xpu_intern;
import :emitter;
import :gradient;

export namespace aten_xpu {

// Conv2D and Conv3D: ConvolutionBackward yields the source and weight derivatives; fused epilogues
// have no gradient here.
class SyclConvolutionGradient : public SyclGradient
{
public:
    explicit SyclConvolutionGradient(const SyclOpsTable& ops) noexcept :
        SyclGradient{ops, "SyclConvolutionGradient"}
    {
    }

protected:
    void emit(SyclGradientEmitter& emitter) override
    {
        if (emitter.getInputCount() != 2 || emitter.readString("unary", "none") != "none" ||
            emitter.readString("binary", "none") != "none") {
            emitter.fail(
                ice::sonic::Code::k_unimplemented,
                "convolution gradient supports only the plain convolution"
            );
            return;
        }
        const auto source = emitter.input(0);
        const auto weight = emitter.input(1);
        const auto upstream = emitter.upstream(0);
        if (!emitter.begin("ConvolutionBackward")) {
            return;
        }
        emitter.add(upstream);
        emitter.add(source);
        emitter.add(weight);
        emitter.setType("T", emitter.getElementType());
        for (const auto name: {"stride", "padding", "output_padding", "dilation"}) {
            emitter.setIntegers(name, emitter.readIntegers(name));
        }
        emitter.setInt("groups", emitter.readInt("groups", 1));
        emitter.setBool("input_gradient", true);
        emitter.setBool("weight_gradient", true);
        emitter.setBool("bias_gradient", false);
        if (emitter.finish() < 2) {
            return;
        }
        emitter.setResult(0, emitter.result(0));
        emitter.setResult(1, emitter.result(1));
    }
};

} // namespace aten_xpu
