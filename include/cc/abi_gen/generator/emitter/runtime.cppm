module;

#include <cstddef>

export module cc_abi_gen_generator:runtime_emitter;

import std;
import cc_templating;
import :runtime_spec;

export namespace cc_abi_gen::generator::emitter {

class RuntimeEmitter
{
public:
    static constexpr std::string_view k_string_struct = "TF_StringOps";
    static constexpr std::string_view k_runtime_partition = "runtime";
    static constexpr std::string_view k_create_slot = "create";

    static std::span<const RuntimeSpec> get_specs() noexcept
    {
        static constexpr std::array<RuntimeSpec, 1> k_specs{
            RuntimeSpec{k_string_struct, k_runtime_partition, "runtime_base"},
        };

        return k_specs;
    }

    RuntimeEmitter& add_value(std::string&& key, std::string&& value)
    {
        m_values.emplace_back(std::move(key), std::move(value));
        return *this;
    }

    void clear_values() noexcept
    {
        m_values.clear();
    }

    std::expected<std::string, std::string>
    render(const RuntimeSpec& spec, std::string_view module_name)
    {
        m_scratch_variables = m_values;
        m_scratch_variables.emplace_back("module_name", std::string{module_name});
        m_scratch_variables.emplace_back("partition", std::string{spec.get_partition()});

        return cc::templating::TemplateRenderer::render_template(
            spec.get_template_name(),
            m_scratch_variables
        );
    }

private:
    std::vector<std::pair<std::string, std::string>> m_values;
    std::vector<std::pair<std::string, std::string>> m_scratch_variables;
};

} // namespace cc_abi_gen::generator::emitter
