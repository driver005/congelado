module;

#include "c/extern/generator/attribute.h"

export module cc_abi_runtime_generator:c_attribute;

import cc_abi_builder_intern;
import cc_abi_builder_generator;

export namespace ice {

// C ABI adapter: implements GeneratorAttributeViewBase by calling TF_Generator_Attribute_* functions.
class CGeneratorAttributeView : public GeneratorAttributeViewBase {
public:
    explicit CGeneratorAttributeView(const TF_Generator_Attribute* handle) : m_handle(handle) {}
    ~CGeneratorAttributeView() override = default;

    CGeneratorAttributeView(const CGeneratorAttributeView&) = default;
    CGeneratorAttributeView& operator=(const CGeneratorAttributeView&) = default;
    CGeneratorAttributeView(CGeneratorAttributeView&&) = default;
    CGeneratorAttributeView& operator=(CGeneratorAttributeView&&) = default;

    StringBuilder get_name() const override {
        return StringBuilder(TF_Generator_Attribute_GetName(m_handle));
    }
    StringBuilder get_description() const override {
        return StringBuilder(TF_Generator_Attribute_GetDescription(m_handle));
    }
    StringBuilder get_full_type() const override {
        return StringBuilder(TF_Generator_Attribute_GetFullType(m_handle));
    }
    StringBuilder get_base_type() const override {
        return StringBuilder(TF_Generator_Attribute_GetBaseType(m_handle));
    }
    bool is_list() const override {
        return TF_Generator_Attribute_IsList(m_handle);
    }

    const TF_Generator_Attribute* get_handle() const { return m_handle; }

private:
    const TF_Generator_Attribute* m_handle;
};

} // namespace ice
