module;

export module aten_xpu_intern:shared_state;

export namespace aten_xpu {

// Base for runtime state that a later module owns but the ops table must hand to objects the C
// ABI creates without a context pointer.
class SyclSharedState
{
public:
    SyclSharedState() = default;
    virtual ~SyclSharedState() = default;
    SyclSharedState(const SyclSharedState&) = delete;
    SyclSharedState& operator=(const SyclSharedState&) = delete;
    SyclSharedState(SyclSharedState&&) = delete;
    SyclSharedState& operator=(SyclSharedState&&) = delete;
};

} // namespace aten_xpu
