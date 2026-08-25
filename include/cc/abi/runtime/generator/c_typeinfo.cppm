module;

#include "c/extern/generator/typeinfo.h"

export module cc_abi_runtime_generator:c_typeinfo;

import cc_abi_builder_intern;
import cc_abi_builder_generator;

export namespace ice {

// C ABI adapter: implements GeneratorTypeInfoViewBase by calling TF_Generator_TypeInfo_* functions.
class CGeneratorTypeInfoView : public GeneratorTypeInfoViewBase {
public:
    explicit CGeneratorTypeInfoView(const TF_Generator_TypeInfo* handle) : m_handle(handle) {}
    ~CGeneratorTypeInfoView() override = default;

    CGeneratorTypeInfoView(const CGeneratorTypeInfoView&) = default;
    CGeneratorTypeInfoView& operator=(const CGeneratorTypeInfoView&) = default;
    CGeneratorTypeInfoView(CGeneratorTypeInfoView&&) = default;
    CGeneratorTypeInfoView& operator=(CGeneratorTypeInfoView&&) = default;

    int get_data_type() const override {

        return TF_Generator_TypeInfo_GetDataType(m_handle);

    }
    StringBuilder get_type_attr_name() const override {

        return StringBuilder(TF_Generator_TypeInfo_GetTypeAttrName(m_handle));

    }
    bool is_read_only() const override {

        return TF_Generator_TypeInfo_IsReadOnly(m_handle);

    }
    bool is_list() const override {

        return TF_Generator_TypeInfo_IsList(m_handle);

    }

    const TF_Generator_TypeInfo* get_handle() const { return m_handle; }

private:
    const TF_Generator_TypeInfo* m_handle;
};

} // namespace ice
