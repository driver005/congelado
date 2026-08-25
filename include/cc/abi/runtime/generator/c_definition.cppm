module;

#include "c/extern/generator/definition.h"
#include "c/extern/generator/parameter.h"
#include "c/extern/generator/attribute.h"
#include "c/extern/generator/typeinfo.h"

export module cc_abi_runtime_generator:c_definition;

import std;
import cc_abi_builder_intern;
import cc_abi_builder_generator;
import :c_parameter;
import :c_attribute;

export namespace ice {

// C ABI adapter: implements GeneratorDefinitionViewBase by calling TF_Generator_Definition_* functions.
class CGeneratorDefinitionView : public GeneratorDefinitionViewBase {
public:
    explicit CGeneratorDefinitionView(const TF_Generator_Definition* handle) : m_handle(handle) {}
    ~CGeneratorDefinitionView() override = default;

    CGeneratorDefinitionView(const CGeneratorDefinitionView&) = default;
    CGeneratorDefinitionView& operator=(const CGeneratorDefinitionView&) = default;
    CGeneratorDefinitionView(CGeneratorDefinitionView&&) = default;
    CGeneratorDefinitionView& operator=(CGeneratorDefinitionView&&) = default;

    StringBuilder get_name() const override {

        return StringBuilder(TF_Generator_Definition_GetName(m_handle));

    }
    StringBuilder get_summary() const override {

        return StringBuilder(TF_Generator_Definition_GetSummary(m_handle));

    }
    StringBuilder get_description() const override {

        return StringBuilder(TF_Generator_Definition_GetDescription(m_handle));

    }

    size_t get_input_count() const override {

        return TF_Generator_Definition_GetInputCount(m_handle);

    }
    std::unique_ptr<GeneratorParameterViewBase> get_input(size_t index) const override {

        const TF_Generator_Parameter* param = TF_Generator_Definition_GetInput(m_handle, index);
        if (!param) return nullptr;
        return std::make_unique<CGeneratorParameterView>(param);

    }

    size_t get_output_count() const override {

        return TF_Generator_Definition_GetOutputCount(m_handle);

    }
    std::unique_ptr<GeneratorParameterViewBase> get_output(size_t index) const override {

        const TF_Generator_Parameter* param = TF_Generator_Definition_GetOutput(m_handle, index);
        if (!param) return nullptr;
        return std::make_unique<CGeneratorParameterView>(param);

    }

    size_t get_attr_count() const override {

        return TF_Generator_Definition_GetAttrCount(m_handle);

    }
    std::unique_ptr<GeneratorAttributeViewBase> get_attr(size_t index) const override {

        const TF_Generator_Attribute* attr = TF_Generator_Definition_GetAttr(m_handle, index);
        if (!attr) return nullptr;
        return std::make_unique<CGeneratorAttributeView>(attr);

    }

    const TF_Generator_Definition* get_handle() const { return m_handle; }

private:
    const TF_Generator_Definition* m_handle;
};

} // namespace ice
