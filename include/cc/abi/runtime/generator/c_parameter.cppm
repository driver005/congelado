module;

#include "c/extern/generator/parameter.h"
#include "c/extern/generator/typeinfo.h"

export module cc_abi_runtime_generator:c_parameter;

import std;
import cc_abi_builder_intern;
import cc_abi_builder_generator;
import :c_typeinfo;

export namespace ice {

// C ABI adapter: implements GeneratorParameterViewBase by calling TF_Generator_Parameter_* functions.
class CGeneratorParameterView : public GeneratorParameterViewBase {
public:
    explicit CGeneratorParameterView(const TF_Generator_Parameter* handle) : m_handle(handle) {}
    ~CGeneratorParameterView() override = default;

    CGeneratorParameterView(const CGeneratorParameterView&) = default;
    CGeneratorParameterView& operator=(const CGeneratorParameterView&) = default;
    CGeneratorParameterView(CGeneratorParameterView&&) = default;
    CGeneratorParameterView& operator=(CGeneratorParameterView&&) = default;

    StringBuilder get_name() const override {

        return StringBuilder(TF_Generator_Parameter_GetName(m_handle));

    }
    StringBuilder get_description() const override {

        return StringBuilder(TF_Generator_Parameter_GetDescription(m_handle));

    }
    int get_position() const override {

        return TF_Generator_Parameter_GetPosition(m_handle);

    }
    std::unique_ptr<GeneratorTypeInfoViewBase> get_type() const override {

        const TF_Generator_TypeInfo* type = TF_Generator_Parameter_GetType(m_handle);
        if (!type) return nullptr;
        return std::make_unique<CGeneratorTypeInfoView>(type);

    }

    const TF_Generator_Parameter* get_handle() const { return m_handle; }

private:
    const TF_Generator_Parameter* m_handle;
};

} // namespace ice
