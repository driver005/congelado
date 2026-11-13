module;

#include "include/c/extern/gradients/gradient.h"

export module aten_xpu_extern_gradients:gradient;

import std;
import cc_ice_intern_sonic;
import cc_ice_extern_gradients_sonic;
import cc_ice_extern_gradients_builder;
import aten_xpu_intern;
import :emitter;

export namespace aten_xpu {

// Base of every gradient the plugin registers: compute opens an emitter on the host context and
// calls emit, which sets one result per differentiable forward input.
class SyclGradient : public ice::builder::Gradient
{
public:
    SyclGradient(const SyclOpsTable& ops, std::string_view name) noexcept :
        ice::builder::Gradient{ops.getGradientContextOps(), ops.getStatusOps(), ops.getStringOps()},
        m_helper{ops},
        m_name{name}
    {
    }

    ~SyclGradient() override = default;
    SyclGradient(const SyclGradient&) = delete;
    SyclGradient& operator=(const SyclGradient&) = delete;
    SyclGradient(SyclGradient&&) = delete;
    SyclGradient& operator=(SyclGradient&&) = delete;

    void destroy() noexcept override
    {
        delete this;
    }

    void get_name(const ice::sonic::String& out_name) noexcept override
    {
        m_helper.copy_into(out_name, m_name);
    }

    void compute(
        const ice::sonic::GradientContext& context,
        const ice::sonic::Status& out_status
    ) noexcept override
    {
        try {
            SyclGradientEmitter emitter{context, out_status};
            if (emitter.ok()) {
                emit(emitter);
            }
        } catch (const std::exception& error) {
            m_helper.fail(out_status, ice::sonic::Code::k_internal, error.what());
        }
    }

protected:
    virtual void emit(SyclGradientEmitter& emitter) = 0;

private:
    SyclStatus m_helper;
    std::string_view m_name;
};

} // namespace aten_xpu
