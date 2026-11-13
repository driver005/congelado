module;

export module aten_xpu_extern_gradients:plugin_copy;

import std;
import cc_ice_intern_sonic;
import aten_xpu_intern;
import :emitter;
import :gradient;

export namespace aten_xpu {

// _CopyFromXpuToHost and _CopyFromHostToXpu: the gradient travels back through the opposite copy.
class SyclCopyGradient : public SyclGradient
{
public:
    explicit SyclCopyGradient(const SyclOpsTable& ops) noexcept :
        SyclGradient{ops, "SyclCopyGradient"}
    {
    }

protected:
    void emit(SyclGradientEmitter& emitter) override
    {
        const auto upstream = emitter.upstream(0);
        const bool to_host = emitter.getForwardName() == "_CopyFromXpuToHost";
        emitter.setResult(
            0,
            emitter.call(to_host ? "_CopyFromHostToXpu" : "_CopyFromXpuToHost", {upstream})
        );
    }
};

} // namespace aten_xpu
