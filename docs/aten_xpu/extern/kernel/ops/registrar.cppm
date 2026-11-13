module;

#include "include/c/extern/ops/op_definition_builder.h"
#include "include/c/intern/status.h"

export module aten_xpu_extern_kernel:ops_registrar;

import std;
import cc_ice_intern_sonic;
import cc_ice_extern_ops_sonic;
import aten_xpu_intern;
import :ops_shape_inference;

export namespace aten_xpu {

// Registers op definitions with the host; the specs follow TensorFlow's op definition grammar.
class SyclOpRegistrar
{
public:
    explicit SyclOpRegistrar(const SyclOpsTable& ops) :
        m_status{ops.getStatusOps()},
        m_scratch{ops.getStringOps()}
    {
        m_status.create();
        m_scratch.create();
    }

    ~SyclOpRegistrar()
    {
        m_scratch.destroy();
        m_status.destroy();
    }

    SyclOpRegistrar(const SyclOpRegistrar&) = delete;
    SyclOpRegistrar& operator=(const SyclOpRegistrar&) = delete;
    SyclOpRegistrar(SyclOpRegistrar&&) = delete;
    SyclOpRegistrar& operator=(SyclOpRegistrar&&) = delete;

    template<SyclShapeRule Rule>
    bool add(
        std::string_view op_name,
        std::initializer_list<std::string_view> attributes,
        std::initializer_list<std::string_view> inputs,
        std::initializer_list<std::string_view> outputs
    )
    {
        ::TF_OpDefinitionBuilderOps* builder_ops = nullptr;
        ::TF_OpDefinitionBuilder handle{};
        m_scratch.copy(op_name);
        create_op_definition_builder(
            &builder_ops,
            &handle.plugin_data,
            m_scratch.get_handle(),
            m_status.get_handle()
        );
        if (!ok()) {
            return false;
        }

        ice::sonic::OpDefinitionBuilder builder{builder_ops, &handle};
        for (const auto attribute: attributes) {
            m_scratch.copy(attribute);
            builder.add_attr(m_scratch);
        }
        for (const auto input: inputs) {
            m_scratch.copy(input);
            builder.add_input(m_scratch);
        }
        for (const auto output: outputs) {
            m_scratch.copy(output);
            builder.add_output(m_scratch);
        }
        builder.set_shape_inference_function(&SyclShapeInference::infer<Rule>);
        builder.register_op_definition(m_status);
        destroy_op_definition_builder(handle.plugin_data);
        ++m_registered;
        return ok();
    }

    bool ok() const noexcept
    {
        ice::sonic::Code code{ice::sonic::Code::k_ok};
        m_status.get_code(code);
        return code.get_value() == TF_OK;
    }

    std::size_t getRegisteredCount() const noexcept
    {
        return m_registered;
    }

    const ice::sonic::Status& getStatus() const noexcept
    {
        return m_status;
    }

private:
    ice::sonic::Status m_status;
    ice::sonic::String m_scratch;
    std::size_t m_registered{0};
};

} // namespace aten_xpu
